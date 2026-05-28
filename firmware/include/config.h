/**
 * @file config.h
 * @brief Configuración central del sistema embebido IoT para cultivo de hortalizas.
 *        Centraliza todos los parámetros ajustables: credenciales, pines, umbrales,
 *        tópicos MQTT e intervalos de temporización.
 *
 * @project  Sistema IoT - Monitoreo y Control de Cultivo de Hortalizas
 * @platform ESP32-WROOM-32
 * @version  1.0.0
 * @date     2026
 * @author   Ingeniería Mecatrónica - Sistemas Embebidos
 */

#pragma once

// ============================================================
//  RED WIFI
// ============================================================
#define WIFI_SSID            "Karen_Wifi"
#define WIFI_PASSWORD        "1402151022"
#define WIFI_TIMEOUT_MS      10000UL     // Tiempo máximo de espera de conexión
#define WIFI_RETRY_INTERVAL  30000UL     // Intervalo entre reintentos de conexión

// ============================================================
//  BROKER MQTT (Mosquitto local en PC Windows)
// ============================================================
#define MQTT_BROKER_IP       "10.201.149.74"   // IP local del PC con Mosquitto
#define MQTT_BROKER_PORT     1883
#define MQTT_CLIENT_ID       "esp32-cultivo-01"
#define MQTT_USER            ""                // Dejar vacío si no hay autenticación
#define MQTT_PASSWORD_STR    ""
#define MQTT_KEEPALIVE       60                // Segundos de keepalive
#define MQTT_RETRY_INTERVAL  5000UL            // Intervalo entre reintentos MQTT

// ============================================================
//  TÓPICOS MQTT — TELEMETRÍA (ESP32 → Broker)
// ============================================================
#define TOPIC_SENSORES           "cultivo/sensores"
#define TOPIC_TEMPERATURA        "cultivo/temperatura"
#define TOPIC_HUMEDAD_AIRE       "cultivo/humedad_aire"
#define TOPIC_HUMEDAD_SUSTRATO   "cultivo/humedad_sustrato"
#define TOPIC_BOMBA_ESTADO       "cultivo/bomba_riego/estado"
#define TOPIC_SERVO_ESTADO       "cultivo/control_termico/estado"
#define TOPIC_MODO               "cultivo/modo"
#define TOPIC_SISTEMA_ESTADO     "cultivo/sistema/estado"
#define TOPIC_HEARTBEAT          "cultivo/heartbeat"

// ============================================================
//  TÓPICOS MQTT — COMANDOS (Broker → ESP32)
// ============================================================
#define TOPIC_CMD_RIEGO          "cultivo/comando/riego"
#define TOPIC_CMD_SERVO          "cultivo/comando/servo"
#define TOPIC_CMD_MODO           "cultivo/comando/modo"

// ============================================================
//  PINES GPIO — ESP32-WROOM-32
// ============================================================
#define PIN_DHT11            4    // Sensor temperatura/humedad aire
#define PIN_HUMEDAD_SUELO    34   // Sensor capacitivo humedad sustrato (ADC)
#define PIN_RELE_BOMBA       26   // Relé 5V → bomba de riego
#define PIN_SERVO            18   // Señal PWM → servo MG995

// ============================================================
//  SENSOR DHT11
// ============================================================
#define DHT_TYPE             DHT11

// ============================================================
//  CALIBRACIÓN SENSOR RESISTIVO DE HUMEDAD DE SUELO (HW-080)
//
//  *** IMPORTANTE — DIFERENCIA ARDUINO UNO vs ESP32 ***
//  Arduino Uno : ADC 10 bits → rango 0–1023
//  ESP32       : ADC 12 bits → rango 0–4095  (4× más grande)
//
//  Los valores medidos en Arduino Uno (850/250) NO sirven en ESP32.
//  Los valores ESP32 son aproximadamente 4× los del Arduino Uno.
//  Ejemplo: Uno=850 → ESP32≈3400 | Uno=250 → ESP32≈1000
//
//  PROCEDIMIENTO DE CALIBRACIÓN CON ESP32:
//  1. Activa MODO_CALIBRACION 1 (abajo) y sube el firmware.
//  2. Abre el Monitor Serial a 115200 bps.
//  3. Saca el sensor del suelo, déjalo en el aire (seco).
//     Anota el valor ADC que aparece en pantalla → eso es SOIL_ADC_SECO.
//  4. Sumerge la punta del sensor en un vaso de agua.
//     Anota el valor ADC → eso es SOIL_ADC_MOJADO.
//  5. Pon MODO_CALIBRACION 0, escribe los valores reales abajo y sube.
//
//  Valores de referencia ESP32 (pueden variar según tu placa/sensor):
//    Seco  (aire) → ADC ≈ 3400
//    Mojado(agua) → ADC ≈  900
//
//  Si ADC = 4095 siempre → el sensor está DESCONECTADO o el pin flota.
//  Verifica: VCC→3.3V, GND→GND, AO→GPIO34. (Solo usar AO, no DO)
// ============================================================
#define MODO_CALIBRACION     0       // 1 = solo imprime ADC crudo sin convertir
                                     // 0 = operación normal con porcentaje

#define SOIL_ADC_SECO        4100    // ADC medido con sensor en aire (seco  → 0%)
#define SOIL_ADC_MOJADO      1200    // ADC medido con sensor en agua (mojado→ 100%)

// ============================================================
//  UMBRALES DE CONTROL — RIEGO (con histéresis)
//  Lógica: ENCENDER bomba cuando humedad < UMBRAL_MIN
//          APAGAR bomba cuando humedad > UMBRAL_MIN + HISTERESIS
// ============================================================
#define UMBRAL_HUMEDAD_MIN   55.0f   // % — umbral inferior de humedad de suelo
#define HISTERESIS_HUMEDAD   15.0f   // % — banda de histéresis para evitar oscilación
#define UMBRAL_HUMEDAD_MAX   (UMBRAL_HUMEDAD_MIN + HISTERESIS_HUMEDAD)

// ============================================================
//  UMBRALES DE CONTROL — TEMPERATURA (con histéresis)
//  Lógica: ABRIR ventilación cuando temp > UMBRAL_TEMP_MAX
//          CERRAR ventilación cuando temp < UMBRAL_TEMP_MAX - HISTERESIS_TEMP
// ============================================================
#define UMBRAL_TEMP_MAX      30.0f   // °C — umbral superior de temperatura
#define HISTERESIS_TEMP       3.0f   // °C — banda de histéresis térmica
#define UMBRAL_TEMP_MIN      (UMBRAL_TEMP_MAX - HISTERESIS_TEMP)

// ============================================================
//  POSICIONES DEL SERVOMOTOR MG995
// ============================================================
#define SERVO_POS_CERRADO    0       // grados — techo/ventilación cerrada
#define SERVO_POS_ABIERTO    90      // grados — techo/ventilación abierta

// ============================================================
//  VALIDACIÓN DE LECTURAS DE SENSORES
// ============================================================
#define TEMP_MIN_VALIDA      -10.0f
#define TEMP_MAX_VALIDA       60.0f
#define HUM_MIN_VALIDA         0.0f
#define HUM_MAX_VALIDA       100.0f
#define SUELO_MIN_VALIDO       0.0f
#define SUELO_MAX_VALIDO     100.0f

// ============================================================
//  INTERVALOS DE TEMPORIZACIÓN NO BLOQUEANTE (millis)
// ============================================================
#define INTERVAL_SENSOR_MS       2000UL    // Lectura de sensores
#define INTERVAL_PUBLISH_MS      5000UL    // Publicación MQTT
#define INTERVAL_CONTROL_MS      1000UL    // Evaluación de control automático
#define INTERVAL_WIFI_CHECK_MS  30000UL    // Verificación WiFi
#define INTERVAL_MQTT_CHECK_MS   5000UL    // Verificación MQTT
#define INTERVAL_HEARTBEAT_MS   10000UL    // Publicación de heartbeat

// ============================================================
//  CONFIGURACIÓN SERIAL
// ============================================================
#define SERIAL_BAUD_RATE     115200

// ============================================================
//  IDENTIFICACIÓN DEL PROYECTO
// ============================================================
#define FIRMWARE_VERSION     "1.0.0"
#define PROYECTO_NOMBRE      "Sistema IoT - Cultivo de Hortalizas"
#define PLATAFORMA           "ESP32-WROOM-32"
