/**
 * @file mqtt_manager.cpp
 * @brief Implementación del módulo de gestión MQTT.
 *        Utiliza PubSubClient sobre WiFiClient para conectarse al broker
 *        Mosquitto local. Gestiona suscripciones, publicaciones y reconexión.
 *
 * @project  Sistema IoT - Cultivo de Hortalizas
 * @platform ESP32-WROOM-32
 * @version  1.0.0
 */

#include "mqtt_manager.h"
#include "iot_manager.h"
#include "wifi_manager.h"
#include "config.h"
#include <WiFi.h>
#include <PubSubClient.h>

// ============================================================
//  Objetos de cliente WiFi y MQTT (alcance de archivo)
// ============================================================
static WiFiClient   wifiClient;
static PubSubClient mqttClient(wifiClient);

// ============================================================
//  onMQTTMessage — Callback de recepción de mensajes MQTT
// ============================================================
void onMQTTMessage(char* topic, byte* payload, unsigned int length) {
    // Convertir payload a String (con null terminator explícito)
    String payloadStr;
    payloadStr.reserve(length);
    for (unsigned int i = 0; i < length; i++) {
        payloadStr += (char)payload[i];
    }

    Serial.printf("[MQTT] Mensaje recibido | Tópico: %s | Payload: %s\n",
                  topic, payloadStr.c_str());

    // Despachar al módulo IoT para procesamiento del comando
    handleMQTTCommand(String(topic), payloadStr);
}

// ============================================================
//  initMQTT
// ============================================================
void initMQTT() {
    mqttClient.setServer(MQTT_BROKER_IP, MQTT_BROKER_PORT);
    mqttClient.setCallback(onMQTTMessage);
    mqttClient.setKeepAlive(MQTT_KEEPALIVE);
    mqttClient.setBufferSize(512);  

    Serial.printf("[MQTT] Broker configurado: %s:%d\n",
                  MQTT_BROKER_IP, MQTT_BROKER_PORT);
}

// ============================================================
//  subscribeToTopics — Suscripción interna a tópicos de comando
// ============================================================
static void subscribeToTopics() {
    mqttClient.subscribe(TOPIC_CMD_RIEGO);
    mqttClient.subscribe(TOPIC_CMD_SERVO);
    mqttClient.subscribe(TOPIC_CMD_MODO);

    Serial.printf("[MQTT] Suscrito a: %s, %s, %s\n",
                  TOPIC_CMD_RIEGO, TOPIC_CMD_SERVO, TOPIC_CMD_MODO);
}

// ============================================================
//  connectMQTT
// ============================================================
void connectMQTT() {
    if (!isWiFiConnected()) {
        Serial.println(F("[MQTT] Sin WiFi. No se puede conectar al broker."));
        return;
    }

    Serial.printf("[MQTT] Conectando a broker %s:%d con ID '%s'...\n",
                  MQTT_BROKER_IP, MQTT_BROKER_PORT, MQTT_CLIENT_ID);

    bool connected;
    if (strlen(MQTT_USER) > 0) {
        connected = mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD_STR);
    } else {
        connected = mqttClient.connect(MQTT_CLIENT_ID);
    }

    if (connected) {
        Serial.println(F("[MQTT] ✓ Conectado al broker Mosquitto."));
        subscribeToTopics();

        // Publicar estado de conexión (mensaje LWT inverso)
        mqttClient.publish(TOPIC_SISTEMA_ESTADO, "{\"estado\":\"ONLINE\"}", true);
    } else {
        Serial.printf("[MQTT] ✗ Fallo de conexión. Código de error: %d\n",
                      mqttClient.state());
    }
}

// ============================================================
//  checkMQTTConnection
// ============================================================
void checkMQTTConnection() {
    if (mqttClient.connected()) {
        return;
    }

    if (!isWiFiConnected()) {
        return;  // Esperar a que WiFi se recupere primero
    }

    Serial.println(F("[MQTT] Conexión perdida. Reconectando..."));
    connectMQTT();
}

// ============================================================
//  processMQTT
// ============================================================
void processMQTT() {
    mqttClient.loop();
}

// ============================================================
//  publishMQTT
// ============================================================
bool publishMQTT(const char* topic, const String& payload) {
    if (!mqttClient.connected()) {
        Serial.printf("[MQTT] No conectado. No se pudo publicar en %s\n", topic);
        return false;
    }

    bool ok = mqttClient.publish(topic, payload.c_str());
    if (!ok) {
        Serial.printf("[MQTT] ✗ Error al publicar en %s\n", topic);
    }
    return ok;
}

// ============================================================
//  isMQTTConnected
// ============================================================
bool isMQTTConnected() {
    return mqttClient.connected();
}
