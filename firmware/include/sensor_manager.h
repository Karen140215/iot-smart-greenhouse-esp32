/**
 * @file sensor_manager.h
 * @brief Interfaz del módulo de gestión de sensores.
 *        Maneja la adquisición de datos del DHT11 (temperatura y humedad de aire)
 *        y del sensor capacitivo de humedad de suelo. Incluye validación de lecturas.
 *
 * @project  Sistema IoT - Cultivo de Hortalizas
 * @platform ESP32-WROOM-32
 * @version  1.0.0
 */

#pragma once

#include <Arduino.h>

/**
 * @brief Estructura que agrupa todas las lecturas de sensores del sistema.
 */
struct SensorData {
    float   temperatura;           // °C — temperatura ambiente (DHT11)
    float   humedadAire;           // %  — humedad relativa del aire (DHT11)
    float   humedadSustrato;       // %  — humedad del suelo (HW-080 resistivo)
    int     humedadSueloAdcRaw;    // ADC crudo promediado (0–4095), útil para depuración
    bool    temperaturaValida;
    bool    humedadAireValida;
    bool    humedadSustratoValida;
};

/**
 * @brief Inicializa el sensor DHT11 y configura el pin ADC del sensor de suelo.
 */
void initSensors();

/**
 * @brief Lee todos los sensores y actualiza el estado interno.
 *        Valida rangos antes de almacenar la lectura.
 */
void readSensors();

/**
 * @brief Retorna la última lectura validada de todos los sensores.
 * @return SensorData Estructura con todos los valores.
 */
SensorData getSensorData();

/**
 * @brief Imprime los datos de sensores por el puerto Serial (formato tabla).
 *        Útil para depuración y monitoreo local.
 */
void printSensorData();

/**
 * @brief Convierte el valor ADC crudo del sensor capacitivo a porcentaje de humedad.
 * @param adcRaw Valor ADC sin procesar (0–4095 para 12 bits).
 * @return float Porcentaje de humedad del suelo (0.0–100.0 %).
 */
float mapSoilMoisture(int adcRaw);
