/**
 * @file main.cpp
 * @brief Punto de entrada principal del firmware.
 *        Implementa el bucle principal no bloqueante usando millis().
 *        Coordina todos los módulos del sistema: sensores, actuadores,
 *        control automático, WiFi, MQTT y publicación IoT.
 *
 * @project  Sistema IoT - Monitoreo y Control de Cultivo de Hortalizas
 * @platform ESP32-WROOM-32
 * @framework Arduino + PlatformIO
 * @version  1.0.0
 * @date     2026
 * @author   Ingeniería Mecatrónica - Sistemas Embebidos
 *
 * @details  Arquitectura de temporización no bloqueante:
 *           Cada tarea tiene su propia marca de tiempo (lastXxx) y su
 *           intervalo (INTERVAL_XXX_MS). El bucle loop() evalúa en cada
 *           iteración si ha transcurrido el tiempo suficiente para ejecutar
 *           cada tarea, sin usar delay().
 */

#include <Arduino.h>
#include "config.h"
#include "wifi_manager.h"
#include "mqtt_manager.h"
#include "sensor_manager.h"
#include "actuator_manager.h"
#include "control_manager.h"
#include "iot_manager.h"

// ============================================================
//  Marcas de tiempo para temporización no bloqueante
// ============================================================
static unsigned long lastSensorRead   = 0;
static unsigned long lastPublish      = 0;
static unsigned long lastControlLoop  = 0;
static unsigned long lastWiFiCheck    = 0;
static unsigned long lastMQTTCheck    = 0;
static unsigned long lastHeartbeat    = 0;

// ============================================================
//  setup() — Inicialización del sistema
// ============================================================
void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    delay(200);  // Pequeño retardo para estabilizar Serial

    // --- Encabezado de arranque ---
    Serial.println();
    Serial.println(F("╔══════════════════════════════════════════╗"));
    Serial.println(F("║   Sistema IoT - Cultivo de Hortalizas   ║"));
    Serial.println(F("║   ESP32-WROOM-32  |  PlatformIO         ║"));
    Serial.println(F("║   Ingeniería Mecatrónica                ║"));
    Serial.println(F("╚══════════════════════════════════════════╝"));
    Serial.printf ("[CONFIG] Firmware v%s\n", FIRMWARE_VERSION);
    Serial.printf ("[CONFIG] Proyecto: %s\n\n", PROYECTO_NOMBRE);

    // --- Inicialización de módulos (orden de dependencias) ---
    // 1. Actuadores primero → estado seguro (bomba OFF, servo cerrado)
    initActuators();

    // 2. Sensores
    initSensors();

    // 3. Lógica de control (modo AUTO por defecto)
    initControl();

    // 4. Conectar a WiFi
    connectWiFi();

    // 5. Inicializar cliente MQTT y conectar al broker
    initMQTT();
    connectMQTT();

    Serial.println(F("\n[SISTEMA] ✓ Inicialización completada. Sistema operativo.\n"));
}

// ============================================================
//  loop() — Bucle principal no bloqueante
// ============================================================
void loop() {
    unsigned long now = millis();

    // --- 1. Verificación periódica de conexión WiFi ---
    if (now - lastWiFiCheck >= INTERVAL_WIFI_CHECK_MS) {
        lastWiFiCheck = now;
        checkWiFiConnection();
    }

    // --- 2. Verificación periódica de conexión MQTT ---
    if (now - lastMQTTCheck >= INTERVAL_MQTT_CHECK_MS) {
        lastMQTTCheck = now;
        checkMQTTConnection();
    }

    // --- 3. Procesamiento de mensajes MQTT entrantes (PubSubClient loop) ---
    processMQTT();

    // --- 4. Lectura periódica de sensores ---
    if (now - lastSensorRead >= INTERVAL_SENSOR_MS) {
        lastSensorRead = now;
        readSensors();
        printSensorData();  // Reporte en monitor serial
    }

    // --- 5. Bucle de control automático (histéresis) ---
    if (now - lastControlLoop >= INTERVAL_CONTROL_MS) {
        lastControlLoop = now;
        runAutomaticControl();
    }

    // --- 6. Publicación periódica de datos IoT por MQTT ---
    if (now - lastPublish >= INTERVAL_PUBLISH_MS) {
        lastPublish = now;
        publishIoTData();
    }

    // --- 7. Heartbeat de sistema (presencia en línea) ---
    if (now - lastHeartbeat >= INTERVAL_HEARTBEAT_MS) {
        lastHeartbeat = now;
        publishSystemStatus();
    }
}
