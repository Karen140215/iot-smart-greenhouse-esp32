/**
 * @file control_manager.cpp
 * @brief Implementación del módulo de lógica de control automático.
 *        Implementa máquinas de estados con histéresis para riego y control
 *        térmico. El modo manual desactiva el control automático sin perder estado.
 *
 * @project  Sistema IoT - Cultivo de Hortalizas
 * @platform ESP32-WROOM-32
 * @version  1.0.0
 *
 * @details  PRINCIPIO DE HISTÉRESIS:
 *
 *           El control ON/OFF simple tiene un problema: si el valor medido oscila
 *           alrededor del umbral, el actuador se encendería y apagaría continuamente
 *           (chattering). La histéresis resuelve esto usando dos umbrales:
 *
 *           RIEGO:
 *             - Encender bomba si: humedad_suelo < UMBRAL_MIN  (ej: 30%)
 *             - Apagar bomba si:   humedad_suelo > UMBRAL_MAX  (ej: 45%)
 *             - Entre 30% y 45%: mantener estado actual.
 *
 *           TEMPERATURA:
 *             - Abrir ventilación si: temperatura > UMBRAL_TEMP_MAX  (ej: 30°C)
 *             - Cerrar ventilación si: temperatura < UMBRAL_TEMP_MIN (ej: 27°C)
 *             - Entre 27°C y 30°C: mantener estado actual.
 */

#include "control_manager.h"
#include "sensor_manager.h"
#include "actuator_manager.h"
#include "config.h"
#include <Arduino.h>

// ============================================================
//  Estado interno del módulo de control
// ============================================================
static bool            autoMode        = true;
static IrrigationState irrigationState = IrrigationState::IDLE;
static ThermalState    thermalState    = ThermalState::NORMAL;

// ============================================================
//  initControl
// ============================================================
void initControl() {
    autoMode        = true;
    irrigationState = IrrigationState::IDLE;
    thermalState    = ThermalState::NORMAL;

    Serial.println(F("[CTRL] Control automático inicializado. Modo: AUTO"));
    Serial.printf ("[CTRL] Riego — Umbral mín: %.1f%%, Umbral máx: %.1f%%\n",
                   UMBRAL_HUMEDAD_MIN, UMBRAL_HUMEDAD_MAX);
    Serial.printf ("[CTRL] Térmica — Umbral máx: %.1f°C, Umbral mín: %.1f°C\n",
                   UMBRAL_TEMP_MAX, UMBRAL_TEMP_MIN);
}

// ============================================================
//  controlIrrigation — Máquina de estados de riego
// ============================================================
static void controlIrrigation(const SensorData& data) {
    if (!data.humedadSustratoValida) {
        return;  // No actuar con lecturas inválidas
    }

    switch (irrigationState) {
        case IrrigationState::IDLE:
            // Transición IDLE → WATERING cuando el suelo está demasiado seco
            if (data.humedadSustrato < UMBRAL_HUMEDAD_MIN) {
                irrigationState = IrrigationState::WATERING;
                setPump(true);
                Serial.printf("[CTRL] Riego ACTIVADO — Humedad suelo: %.1f%% < %.1f%%\n",
                              data.humedadSustrato, UMBRAL_HUMEDAD_MIN);
            }
            break;

        case IrrigationState::WATERING:
            // Transición WATERING → IDLE cuando el suelo alcanza la humedad objetivo
            if (data.humedadSustrato > UMBRAL_HUMEDAD_MAX) {
                irrigationState = IrrigationState::IDLE;
                setPump(false);
                Serial.printf("[CTRL] Riego DESACTIVADO — Humedad suelo: %.1f%% > %.1f%%\n",
                              data.humedadSustrato, UMBRAL_HUMEDAD_MAX);
            }
            break;

        case IrrigationState::MANUAL:
            // En modo manual, el control automático no interviene
            break;
    }
}

// ============================================================
//  controlTemperature — Máquina de estados de control térmico
// ============================================================
static void controlTemperature(const SensorData& data) {
    if (!data.temperaturaValida) {
        return;  // No actuar con lecturas inválidas
    }

    switch (thermalState) {
        case ThermalState::NORMAL:
            // Transición NORMAL → VENTILATING cuando la temperatura es muy alta
            if (data.temperatura > UMBRAL_TEMP_MAX) {
                thermalState = ThermalState::VENTILATING;
                setServoPosition(SERVO_POS_ABIERTO);
                Serial.printf("[CTRL] Ventilación ABIERTA — Temp: %.1f°C > %.1f°C\n",
                              data.temperatura, UMBRAL_TEMP_MAX);
            }
            break;

        case ThermalState::VENTILATING:
            // Transición VENTILATING → NORMAL cuando la temperatura baja suficiente
            if (data.temperatura < UMBRAL_TEMP_MIN) {
                thermalState = ThermalState::NORMAL;
                setServoPosition(SERVO_POS_CERRADO);
                Serial.printf("[CTRL] Ventilación CERRADA — Temp: %.1f°C < %.1f°C\n",
                              data.temperatura, UMBRAL_TEMP_MIN);
            }
            break;

        case ThermalState::MANUAL:
            break;
    }
}

// ============================================================
//  runAutomaticControl
// ============================================================
void runAutomaticControl() {
    if (!autoMode) {
        return;  // Modo manual activo: el control automático no interviene
    }

    SensorData data = getSensorData();
    controlIrrigation(data);
    controlTemperature(data);
}

// ============================================================
//  setAutoMode
// ============================================================
void setAutoMode(bool modoAuto) {
    if (autoMode == modoAuto) {
        return;  // Sin cambio real → no actuar
    }

    autoMode = modoAuto;

    if (modoAuto) {
        // Al entrar en AUTO: apagar actuadores primero para garantizar estado
        // conocido. El control automático los reactivará si los sensores lo
        // indican en la siguiente iteración de runAutomaticControl().
        setPump(false);
        setServoPosition(SERVO_POS_CERRADO);

        irrigationState = IrrigationState::IDLE;
        thermalState    = ThermalState::NORMAL;

        Serial.println(F("[CTRL] Modo → AUTOMATICO | Actuadores en estado seguro."));
        Serial.printf ("[CTRL] Control activo con umbrales: riego<%.0f%% / temp>%.0f°C\n",
                       UMBRAL_HUMEDAD_MIN, UMBRAL_TEMP_MAX);
    } else {
        // Al entrar en MANUAL: el control automático se suspende por completo.
        // Los actuadores mantienen su estado actual hasta que Node-RED ordene.
        Serial.println(F("[CTRL] Modo → MANUAL | Control automático suspendido."));
        Serial.println(F("[CTRL] Bomba y servo se controlan solo desde Node-RED."));
    }
}

// ============================================================
//  Getters
// ============================================================
bool isAutoMode() {
    return autoMode;
}

IrrigationState getIrrigationState() {
    return irrigationState;
}

ThermalState getThermalState() {
    return thermalState;
}

String getModeString() {
    return autoMode ? String("AUTO") : String("MANUAL");
}
