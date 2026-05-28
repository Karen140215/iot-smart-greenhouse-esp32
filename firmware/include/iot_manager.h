/**
 * @file iot_manager.h
 * @brief Interfaz del módulo de gestión IoT.
 *        Responsable de la serialización de datos a JSON, publicación de
 *        telemetría MQTT y procesamiento de comandos entrantes desde Node-RED.
 *
 * @project  Sistema IoT - Cultivo de Hortalizas
 * @platform ESP32-WROOM-32
 * @version  1.0.0
 */

#pragma once

#include <Arduino.h>

/**
 * @brief Serializa todos los datos del sistema (sensores + actuadores + modo)
 *        en formato JSON y los publica en los tópicos MQTT configurados.
 *
 *        Formato JSON publicado en TOPIC_SENSORES:
 *        {
 *          "temperatura": 28.5,
 *          "humedad_aire": 71.0,
 *          "humedad_sustrato": 43.0,
 *          "bomba_riego": "ON",
 *          "control_termico": "OFF",
 *          "modo": "AUTO",
 *          "timestamp": 12345678
 *        }
 */
void publishIoTData();

/**
 * @brief Procesa un comando MQTT recibido desde Node-RED.
 *        Despacha la acción correspondiente según el tópico y el payload JSON.
 *
 * @param topic   Tópico MQTT del mensaje recibido.
 * @param payload Payload JSON del mensaje (cadena terminada en null).
 */
void handleMQTTCommand(const String& topic, const String& payload);

/**
 * @brief Publica el estado actual del sistema completo (heartbeat con estado).
 *        Utilizado para que Node-RED detecte si el ESP32 está en línea.
 */
void publishSystemStatus();
