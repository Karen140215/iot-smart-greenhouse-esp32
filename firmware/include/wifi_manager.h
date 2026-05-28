/**
 * @file wifi_manager.h
 * @brief Interfaz del módulo de gestión de conexión WiFi.
 *        Provee conexión inicial, monitoreo de estado y reconexión automática
 *        sin bloquear el bucle principal.
 *
 * @project  Sistema IoT - Cultivo de Hortalizas
 * @platform ESP32-WROOM-32
 * @version  1.0.0
 */

#pragma once

#include <Arduino.h>

/**
 * @brief Intenta conectar al punto de acceso WiFi configurado en config.h.
 *        Espera hasta WIFI_TIMEOUT_MS milisegundos. Reporta estado por Serial.
 */
void connectWiFi();

/**
 * @brief Verifica el estado de la conexión WiFi y reconecta si es necesario.
 *        Debe llamarse periódicamente desde el bucle principal (no bloqueante).
 */
void checkWiFiConnection();

/**
 * @brief Retorna true si el ESP32 está actualmente conectado a WiFi.
 * @return bool Estado de conexión WiFi.
 */
bool isWiFiConnected();

/**
 * @brief Obtiene la dirección IP local asignada por DHCP como String.
 * @return String IP local (ej: "192.168.1.105") o "No conectado".
 */
String getLocalIP();
