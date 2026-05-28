/**
 * @file iot_manager.cpp
 * @brief Implementación del módulo de gestión IoT.
 *        Serializa datos del sistema a JSON y los publica via MQTT.
 *        Deserializa comandos JSON recibidos desde Node-RED y los despacha.
 *
 * @project  Sistema IoT - Cultivo de Hortalizas
 * @platform ESP32-WROOM-32
 * @version  1.0.0
 *
 * @details  Formato JSON publicado (TOPIC_SENSORES):
 *           {
 *             "temperatura":      28.5,
 *             "humedad_aire":     71.0,
 *             "humedad_sustrato": 43.0,
 *             "bomba_riego":      "ON",
 *             "control_termico":  "OFF",
 *             "modo":             "AUTO",
 *             "uptime_s":         12345
 *           }
 *
 *           Formato de comandos esperados (payload JSON):
 *           TOPIC_CMD_RIEGO:  {"accion": "ON"}  |  {"accion": "OFF"}
 *           TOPIC_CMD_SERVO:  {"angulo": 90}
 *           TOPIC_CMD_MODO:   {"modo": "AUTO"}  |  {"modo": "MANUAL"}
 */

#include "iot_manager.h"
#include "mqtt_manager.h"
#include "sensor_manager.h"
#include "actuator_manager.h"
#include "control_manager.h"
#include "config.h"
#include <ArduinoJson.h>
#include <Arduino.h>
#include <WiFi.h>

// ============================================================
//  publishIoTData
// ============================================================
void publishIoTData() {
    SensorData data = getSensorData();

    // --- Documento JSON principal (payload completo) ---
    StaticJsonDocument<256> doc;

    // Campos de sensores (NAN si la lectura es inválida)
    doc["temperatura"]      = data.temperaturaValida
                                ? data.temperatura
                                : (float)NAN;
    doc["humedad_aire"]     = data.humedadAireValida
                                ? data.humedadAire
                                : (float)NAN;
    doc["humedad_sustrato"] = data.humedadSustratoValida
                                ? data.humedadSustrato
                                : (float)NAN;

    // Estado de actuadores
    doc["bomba_riego"]      = getPumpState()     ? "ON" : "OFF";
    doc["control_termico"]  = (getServoAngle() >= SERVO_POS_ABIERTO) ? "ON" : "OFF";

    // Modo de operación y tiempo de actividad
    doc["modo"]             = getModeString();
    doc["uptime_s"]         = millis() / 1000UL;

    // Serializar a String y publicar
    String payload;
    payload.reserve(256);
    serializeJson(doc, payload);

    publishMQTT(TOPIC_SENSORES, payload);

    // --- Publicar también tópicos individuales (para gauges de Node-RED) ---
    if (data.temperaturaValida) {
        publishMQTT(TOPIC_TEMPERATURA,
                    String(data.temperatura, 1));
    }
    if (data.humedadAireValida) {
        publishMQTT(TOPIC_HUMEDAD_AIRE,
                    String(data.humedadAire, 1));
    }
    if (data.humedadSustratoValida) {
        publishMQTT(TOPIC_HUMEDAD_SUSTRATO,
                    String(data.humedadSustrato, 1));
    }

    publishMQTT(TOPIC_BOMBA_ESTADO,  getPumpState() ? "ON" : "OFF");
    publishMQTT(TOPIC_SERVO_ESTADO,  getServoStateString());
    publishMQTT(TOPIC_MODO,          getModeString());

    Serial.printf("[IoT] Datos publicados: T=%.1f°C, HA=%.1f%%, HS=%.1f%%, Modo=%s\n",
                  data.temperatura, data.humedadAire,
                  data.humedadSustrato, getModeString().c_str());
}

// ============================================================
//  handleMQTTCommand — Despachador de comandos recibidos
//
//  REGLA DE PRIORIDAD:
//    1. El comando de MODO siempre se acepta (nunca se bloquea).
//    2. Los comandos de BOMBA y SERVO solo se ejecutan en modo MANUAL.
//       En modo AUTO se ignoran con un aviso en Serial. Esto evita que
//       el control automático contradiga una orden manual al segundo
//       siguiente (la causa raíz de la inestabilidad).
// ============================================================
void handleMQTTCommand(const String& topic, const String& payload) {
    StaticJsonDocument<128> cmd;
    DeserializationError    error = deserializeJson(cmd, payload);

    if (error) {
        Serial.printf("[IoT] ✗ Error JSON: %s | payload: %s\n",
                      error.c_str(), payload.c_str());
        return;
    }

    // --- Cambio de modo (siempre aceptado, sin importar el modo actual) ---
    if (topic == TOPIC_CMD_MODO) {
        const char* modo = cmd["modo"];
        if (modo == nullptr) {
            Serial.println(F("[IoT] ✗ Cmd modo sin campo 'modo'."));
            return;
        }
        if (strcmp(modo, "AUTO") == 0) {
            setAutoMode(true);
        } else if (strcmp(modo, "MANUAL") == 0) {
            setAutoMode(false);
        } else {
            Serial.printf("[IoT] ✗ Modo desconocido: %s\n", modo);
            return;
        }
        // Publicar el nuevo modo INMEDIATAMENTE para que Node-RED actualice
        // su contexto de flujo sin esperar el ciclo de 5 segundos.
        // Sin esto, el gate de Node-RED sigue bloqueando por hasta 5 s.
        publishMQTT(TOPIC_MODO, getModeString());
        return;
    }

    // --- Guardia de modo: RIEGO y SERVO solo se ejecutan en MANUAL ---
    if (isAutoMode()) {
        Serial.printf("[IoT] Cmd ignorado en modo AUTO (tópico: %s). "
                      "Envía MANUAL primero.\n", topic.c_str());
        return;
    }

    // --- Comando de bomba (solo en modo MANUAL) ---
    if (topic == TOPIC_CMD_RIEGO) {
        const char* accion = cmd["accion"];
        if (accion == nullptr) {
            Serial.println(F("[IoT] ✗ Cmd riego sin campo 'accion'."));
            return;
        }
        if (strcmp(accion, "ON") == 0) {
            setPump(true);
            Serial.println(F("[IoT] MANUAL: Bomba ENCENDIDA"));
        } else if (strcmp(accion, "OFF") == 0) {
            setPump(false);
            Serial.println(F("[IoT] MANUAL: Bomba APAGADA"));
        } else {
            Serial.printf("[IoT] ✗ Acción de riego desconocida: %s\n", accion);
        }
    }

    // --- Comando de servo (solo en modo MANUAL) ---
    else if (topic == TOPIC_CMD_SERVO) {
        if (cmd.containsKey("angulo")) {
            int angulo = cmd["angulo"].as<int>();
            setServoPosition(angulo);
            Serial.printf("[IoT] MANUAL: Servo → %d°\n", angulo);
        } else if (cmd.containsKey("accion")) {
            const char* accion = cmd["accion"];
            if (accion == nullptr) return;
            if (strcmp(accion, "ABRIR") == 0) {
                setServoPosition(SERVO_POS_ABIERTO);
                Serial.println(F("[IoT] MANUAL: Ventilación ABIERTA"));
            } else if (strcmp(accion, "CERRAR") == 0) {
                setServoPosition(SERVO_POS_CERRADO);
                Serial.println(F("[IoT] MANUAL: Ventilación CERRADA"));
            }
        }
    }

    else {
        Serial.printf("[IoT] Tópico no manejado: %s\n", topic.c_str());
    }
}

// ============================================================
//  publishSystemStatus
// ============================================================
void publishSystemStatus() {
    StaticJsonDocument<128> doc;
    doc["estado"]    = "ONLINE";
    doc["ip"]        = WiFi.localIP().toString();
    doc["uptime_s"]  = millis() / 1000UL;
    doc["firmware"]  = FIRMWARE_VERSION;
    doc["modo"]      = getModeString();

    String payload;
    serializeJson(doc, payload);
    publishMQTT(TOPIC_HEARTBEAT, payload);
}
