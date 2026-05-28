/**
 * @file wifi_manager.cpp
 * @brief Implementación del módulo de gestión de conexión WiFi.
 *        Maneja la conexión inicial, el monitoreo continuo y la reconexión
 *        automática al punto de acceso configurado en config.h.
 *
 * @project  Sistema IoT - Cultivo de Hortalizas
 * @platform ESP32-WROOM-32
 * @version  1.0.0
 */

#include "wifi_manager.h"
#include "config.h"
#include <WiFi.h>

// ============================================================
//  connectWiFi
// ============================================================
void connectWiFi() {
    Serial.printf("[WiFi] Conectando a SSID: %s\n", WIFI_SSID);

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    unsigned long startTime = millis();

    // Espera activa con timeout — única vez que se usa un bucle bloqueante
    // justificado porque WiFi es prerequisito de todo el sistema.
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - startTime >= WIFI_TIMEOUT_MS) {
            Serial.println(F("[WiFi] ✗ Timeout de conexión. Se reintentará."));
            return;
        }
        delay(500);
        Serial.print(F("."));
    }

    Serial.println();
    Serial.printf("[WiFi] ✓ Conectado. IP local: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("[WiFi] RSSI: %d dBm\n", WiFi.RSSI());
}

// ============================================================
//  checkWiFiConnection
// ============================================================
void checkWiFiConnection() {
    if (WiFi.status() == WL_CONNECTED) {
        return;  // Sin acción si ya está conectado
    }

    Serial.println(F("[WiFi] Conexión perdida. Reconectando..."));
    WiFi.disconnect();
    connectWiFi();
}

// ============================================================
//  isWiFiConnected
// ============================================================
bool isWiFiConnected() {
    return (WiFi.status() == WL_CONNECTED);
}

// ============================================================
//  getLocalIP
// ============================================================
String getLocalIP() {
    if (isWiFiConnected()) {
        return WiFi.localIP().toString();
    }
    return String("No conectado");
}
