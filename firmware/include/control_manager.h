/**
 * @file control_manager.h
 * @brief Interfaz del módulo de lógica de control automático.
 *        Implementa el control de riego y control térmico con histéresis.
 *        Soporta modo automático y modo manual (comandado por MQTT).
 *
 * @project  Sistema IoT - Cultivo de Hortalizas
 * @platform ESP32-WROOM-32
 * @version  1.0.0
 */

#pragma once

#include <Arduino.h>

/**
 * @brief Estados posibles del sistema de riego (máquina de estados).
 */
enum class IrrigationState {
    IDLE,       // Suelo con humedad suficiente, bomba apagada
    WATERING,   // Bomba activa, regando
    MANUAL      // Control manual desde Node-RED (ignora automático)
};

/**
 * @brief Estados posibles del sistema de control térmico (máquina de estados).
 */
enum class ThermalState {
    NORMAL,      // Temperatura dentro del rango aceptable, servo cerrado
    VENTILATING, // Temperatura alta, servo abierto (ventilando)
    MANUAL       // Control manual desde Node-RED
};

/**
 * @brief Inicializa las variables de estado del control y el modo de operación.
 *        Por defecto, el sistema inicia en modo AUTOMÁTICO.
 */
void initControl();

/**
 * @brief Ejecuta el ciclo de control automático con histéresis.
 *        Evalúa sensores y actúa sobre bomba y servo según umbrales.
 *        No tiene efecto si el sistema está en modo manual.
 */
void runAutomaticControl();

/**
 * @brief Establece el modo de operación del sistema.
 * @param modoAuto true para modo automático, false para modo manual.
 */
void setAutoMode(bool modoAuto);

/**
 * @brief Retorna true si el sistema está en modo automático.
 */
bool isAutoMode();

/**
 * @brief Retorna el estado actual del sistema de riego.
 */
IrrigationState getIrrigationState();

/**
 * @brief Retorna el estado actual del sistema de control térmico.
 */
ThermalState getThermalState();

/**
 * @brief Retorna el modo de operación como cadena ("AUTO" / "MANUAL").
 */
String getModeString();
