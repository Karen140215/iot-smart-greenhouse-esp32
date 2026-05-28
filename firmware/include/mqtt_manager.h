/**
 * @file mqtt_manager.h
 * @brief Interfaz del módulo de gestión MQTT.
 *        Administra la conexión al broker Mosquitto, suscripción a tópicos
 *        de comandos, publicación de telemetría y procesamiento del loop MQTT.
 *
 * @project  Sistema IoT - Cultivo de Hortalizas
 * @platform ESP32-WROOM-32
 * @version  1.0.0
 */

#pragma once

#include <Arduino.h>

/**
 * @brief Inicializa el cliente MQTT: configura broker, puerto y función de callback.
 *        Debe llamarse después de que WiFi esté conectado.
 */
void initMQTT();

/**
 * @brief Intenta conectar al broker MQTT y suscribe a los tópicos de comandos.
 *        Reporta estado por Serial.
 */
void connectMQTT();

/**
 * @brief Verifica el estado MQTT y reconecta si es necesario (no bloqueante).
 *        Debe llamarse periódicamente desde el bucle principal.
 */
void checkMQTTConnection();

/**
 * @brief Procesa el loop interno de PubSubClient (recepción de mensajes).
 *        Debe llamarse en cada iteración del loop principal.
 */
void processMQTT();

/**
 * @brief Publica un mensaje en un tópico MQTT con QoS 0.
 * @param topic  Tópico MQTT destino.
 * @param payload Cadena JSON o texto a publicar.
 * @return true si el mensaje fue publicado exitosamente.
 */
bool publishMQTT(const char* topic, const String& payload);

/**
 * @brief Retorna true si el cliente MQTT está actualmente conectado al broker.
 */
bool isMQTTConnected();

/**
 * @brief Callback interno invocado por PubSubClient al recibir un mensaje.
 *        Despacha el mensaje al módulo IoT para su procesamiento.
 */
void onMQTTMessage(char* topic, byte* payload, unsigned int length);
