/**
 * @file sensor_manager.cpp
 * @brief Gestión de sensores: DHT11 (temperatura/humedad aire) y HW-080
 *        (humedad de suelo resistivo). Incluye promediado con descarte de
 *        outliers, conversión a porcentaje calibrado y salida serial clara.
 *
 * @project  Sistema IoT - Cultivo de Hortalizas
 * @platform ESP32-WROOM-32
 * @version  1.1.0  — calibración HW-080 real (seco≈850, mojado≈250)
 *
 * NOTAS ANTI-CORROSIÓN HW-080:
 *   El sensor HW-080 usa electrodos resistivos que sufren electrólisis cuando
 *   están alimentados de forma continua con corriente DC. Para prolongar la
 *   vida útil:
 *     1. Alimentar el sensor SOLO durante la lectura (ver SENSOR_POWER_PIN).
 *        Conectar VCC del sensor a un GPIO capaz de 3.3V (no directamente a 3V3).
 *        Activar el GPIO, esperar 50 ms de estabilización, leer, desactivar.
 *     2. Si no puedes usar un GPIO de alimentación, limitar el tiempo de
 *        encendido reduciendo INTERVAL_SENSOR_MS a ≥ 10000 ms.
 *     3. Para proyectos de largo plazo, reemplazar con sensor capacitivo
 *        (p.ej. STEMMA Soil Sensor de Adafruit) que no corroe.
 *
 * NOTA REDUCCIÓN DE RUIDO:
 *   El ADC del ESP32 tiene ruido inherente (~±10 cuentas). Se toman
 *   NUM_MUESTRAS_ADC lecturas, se descartan el mínimo y el máximo, y se
 *   promedia el resto (método "recortado"). Además se agrega una pequeña
 *   pausa entre muestras para evitar que el ADC capture la misma muestra.
 */

#include "sensor_manager.h"
#include "config.h"
#include <DHT.h>
#include <algorithm>   // std::sort (disponible en ESP32 Arduino core)

// ---- Parámetros de muestreo ADC -----------------------------------------
// Tomar 12 muestras, descartar 2 min y 2 max → promediar 8 centrales.
// Más muestras → más estabilidad, más tiempo de lectura (~120 ms total).
static constexpr int NUM_MUESTRAS_ADC  = 12;
static constexpr int DESCARTAR_EXTREMOS = 2;   // se descartan N min + N max

// ---- Objeto DHT y estado interno ----------------------------------------
static DHT        dht(PIN_DHT11, DHT_TYPE);
static SensorData sensorData = {};

// ============================================================
//  initSensors
// ============================================================
void initSensors() {
    dht.begin();

    // GPIO34 es solo-entrada (no necesita pinMode).
    // Resolución 12 bits → rango 0–4095.
    // ADC_11db → mide hasta ~3.1 V (suficiente para el HW-080 a 3.3 V).
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);

    Serial.println(F("[SENSOR] ─────────────────────────────────────"));
    Serial.printf("[SENSOR] DHT11        → GPIO %d\n", PIN_DHT11);
    Serial.printf("[SENSOR] HW-080 suelo → GPIO %d  (ADC 12 bits)\n", PIN_HUMEDAD_SUELO);
    Serial.printf("[SENSOR] Calibración  → SECO=%d (0%%)  MOJADO=%d (100%%)\n",
                  SOIL_ADC_SECO, SOIL_ADC_MOJADO);
    Serial.printf("[SENSOR] Muestras ADC → %d (descarta %d min + %d max)\n",
                  NUM_MUESTRAS_ADC, DESCARTAR_EXTREMOS, DESCARTAR_EXTREMOS);
    Serial.println(F("[SENSOR] ─────────────────────────────────────"));
}

// ============================================================
//  leerAdcPromediado — promedio recortado de NUM_MUESTRAS_ADC lecturas
// ============================================================
// Descarta DESCARTAR_EXTREMOS valores mínimos y máximos antes de promediar.
// Reduce el impacto de picos de ruido del ADC interno del ESP32.
static int leerAdcPromediado(uint8_t pin) {
    int muestras[NUM_MUESTRAS_ADC];

    for (int i = 0; i < NUM_MUESTRAS_ADC; i++) {
        muestras[i] = analogRead(pin);
        delay(10);   // 10 ms entre muestras evita leer el mismo ciclo ADC
    }

    // Ordenar para poder descartar extremos
    std::sort(muestras, muestras + NUM_MUESTRAS_ADC);

    long suma = 0;
    int  validas = 0;
    for (int i = DESCARTAR_EXTREMOS; i < NUM_MUESTRAS_ADC - DESCARTAR_EXTREMOS; i++) {
        suma += muestras[i];
        validas++;
    }

    return (validas > 0) ? (int)(suma / validas) : muestras[NUM_MUESTRAS_ADC / 2];
}

// ============================================================
//  mapSoilMoisture — Conversión ADC → porcentaje de humedad
// ============================================================
// HW-080 (resistivo): ADC alto = seco (0%), ADC bajo = mojado (100%).
// Calibración: SOIL_ADC_SECO=850, SOIL_ADC_MOJADO=250 (valores reales medidos).
float mapSoilMoisture(int adcRaw) {
    float porcentaje = (float)(SOIL_ADC_SECO - adcRaw) /
                       (float)(SOIL_ADC_SECO - SOIL_ADC_MOJADO) * 100.0f;

    // Clamp estricto: nunca retornar valores negativos ni > 100%
    if (porcentaje < 0.0f)   porcentaje = 0.0f;
    if (porcentaje > 100.0f) porcentaje = 100.0f;

    return porcentaje;
}

// ============================================================
//  readSensors
// ============================================================
void readSensors() {
    // --- Lectura del DHT11 ---
    float tempRaw = dht.readTemperature();
    float humRaw  = dht.readHumidity();

    // Validar temperatura
    if (!isnan(tempRaw) && tempRaw >= TEMP_MIN_VALIDA && tempRaw <= TEMP_MAX_VALIDA) {
        sensorData.temperatura        = tempRaw;
        sensorData.temperaturaValida  = true;
    } else {
        sensorData.temperaturaValida  = false;
        Serial.println(F("[SENSOR] ✗ Lectura inválida de temperatura (DHT11)."));
    }

    // Validar humedad del aire
    if (!isnan(humRaw) && humRaw >= HUM_MIN_VALIDA && humRaw <= HUM_MAX_VALIDA) {
        sensorData.humedadAire        = humRaw;
        sensorData.humedadAireValida  = true;
    } else {
        sensorData.humedadAireValida  = false;
        Serial.println(F("[SENSOR] ✗ Lectura inválida de humedad de aire (DHT11)."));
    }

    // --- Lectura del HW-080 (humedad de suelo) ---
    // Usa promedio recortado (descarta outliers) para máxima estabilidad.
    int adcPromedio = leerAdcPromediado(PIN_HUMEDAD_SUELO);
    sensorData.humedadSueloAdcRaw = adcPromedio;

#if MODO_CALIBRACION
    // En modo calibración solo se imprime el ADC crudo; no se convierte.
    // Usa estos valores para ajustar SOIL_ADC_SECO y SOIL_ADC_MOJADO en config.h.
    sensorData.humedadSustrato       = 0.0f;
    sensorData.humedadSustratoValida = false;
    (void)0;   // el printSensorData() mostrará el ADC crudo de todas formas
#else
    float sueloHumedad = mapSoilMoisture(adcPromedio);
    if (sueloHumedad >= SUELO_MIN_VALIDO && sueloHumedad <= SUELO_MAX_VALIDO) {
        sensorData.humedadSustrato       = sueloHumedad;
        sensorData.humedadSustratoValida = true;
    } else {
        sensorData.humedadSustratoValida = false;
        Serial.printf("[SENSOR] ✗ Lectura fuera de rango: ADC=%d  %%=%.1f\n",
                      adcPromedio, sueloHumedad);
    }
#endif
}

// ============================================================
//  getSensorData
// ============================================================
SensorData getSensorData() {
    return sensorData;
}

// ============================================================
//  buildMoistureBar — genera barra visual de 10 bloques
// ============================================================
// Ejemplo: 60% → "[██████    ] 60.0%"
static void buildMoistureBar(char* buf, size_t bufLen, float pct) {
    const int TOTAL = 10;
    int llenos = (int)(pct / 10.0f + 0.5f);   // redondeo al bloque más cercano
    if (llenos > TOTAL) llenos = TOTAL;
    if (llenos < 0)     llenos = 0;

    int pos = 0;
    buf[pos++] = '[';
    for (int i = 0; i < TOTAL && pos < (int)bufLen - 3; i++) {
        buf[pos++] = (i < llenos) ? '#' : ' ';
    }
    buf[pos++] = ']';
    buf[pos]   = '\0';
}

// ============================================================
//  printSensorData
// ============================================================
void printSensorData() {
    Serial.println(F("┌────────────────────────────────────────────┐"));
    Serial.println(F("│           LECTURA DE SENSORES              │"));
    Serial.println(F("├────────────────────────────────────────────┤"));

    // Temperatura
    if (sensorData.temperaturaValida) {
        Serial.printf("│ Temperatura  : %5.1f C                      │\n",
                      sensorData.temperatura);
    } else {
        Serial.println(F("│ Temperatura  : ERROR (DHT11)               │"));
    }

    // Humedad del aire
    if (sensorData.humedadAireValida) {
        Serial.printf("│ Humedad aire : %5.1f %%                      │\n",
                      sensorData.humedadAire);
    } else {
        Serial.println(F("│ Humedad aire : ERROR (DHT11)               │"));
    }

    Serial.println(F("├────────────────────────────────────────────┤"));

#if MODO_CALIBRACION
    // --- Modo calibración: muestra solo el ADC crudo con instrucciones ---
    Serial.println(F("│  *** MODO CALIBRACION ACTIVO ***           │"));
    Serial.printf( "│  ADC crudo = %4d                           │\n",
                   sensorData.humedadSueloAdcRaw);
    if (sensorData.humedadSueloAdcRaw >= 4090) {
        Serial.println(F("│  ADVERTENCIA: ADC=4095 → sensor desconect │"));
        Serial.println(F("│  Verifica: AO→GPIO34, VCC→3.3V, GND→GND  │"));
    } else {
        Serial.println(F("│  1) Sensor en AIRE  → anota este ADC      │"));
        Serial.println(F("│     → ese valor es SOIL_ADC_SECO           │"));
        Serial.println(F("│  2) Sensor en AGUA  → anota este ADC      │"));
        Serial.println(F("│     → ese valor es SOIL_ADC_MOJADO         │"));
        Serial.println(F("│  3) Actualiza config.h y pon MODO_CAL=0   │"));
    }
#else
    // --- Operación normal: porcentaje + barra visual ---
    if (sensorData.humedadSustratoValida) {
        char barra[16];
        buildMoistureBar(barra, sizeof(barra), sensorData.humedadSustrato);

        Serial.printf("│ Humedad suelo: %5.1f %%  ADC=%4d           │\n",
                      sensorData.humedadSustrato,
                      sensorData.humedadSueloAdcRaw);
        Serial.printf("│  %s %5.1f%%                    │\n",
                      barra, sensorData.humedadSustrato);

        const char* estado;
        if      (sensorData.humedadSustrato < UMBRAL_HUMEDAD_MIN) estado = "NECESITA RIEGO";
        else if (sensorData.humedadSustrato > UMBRAL_HUMEDAD_MAX) estado = "BIEN HIDRATADO ";
        else                                                        estado = "ZONA OK        ";
        Serial.printf("│  Estado: %s                    │\n", estado);
    } else {
        Serial.printf("│ Humedad suelo: ERROR  ADC=%4d             │\n",
                      sensorData.humedadSueloAdcRaw);
        if (sensorData.humedadSueloAdcRaw >= 4090) {
            Serial.println(F("│  ADC=4095: sensor desconectado o flotando  │"));
        }
    }
#endif

    Serial.println(F("└────────────────────────────────────────────┘"));
}
