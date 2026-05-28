# Lista de Componentes (Bill of Materials — BOM)

**Proyecto:** Sistema IoT para Monitoreo y Control de Cultivo de Hortalizas  
**Plataforma:** ESP32-WROOM-32  

---

## Componentes Principales

| N° | Componente | Descripción | Cantidad | Referencia |
|----|-----------|-------------|----------|-----------|
| 1 | **ESP32-WROOM-32** | Microcontrolador con WiFi y Bluetooth integrados, dual-core 240 MHz | 1 | Placa de desarrollo (DevKit v1) |
| 2 | **DHT11** | Sensor digital de temperatura (0–50°C ±2°C) y humedad relativa (20–90% ±5%) | 1 | Con resistencia pull-up 10kΩ integrada |
| 3 | **Sensor de humedad de suelo** | Sensor analógico de humedad de sustrato, salida analógica (0–3.3V). Compatible con módulo resistivo HW-080 o sensor capacitivo v1.2/v2.0 | 1 | Solo usar pin AO (analógico); el pin DO no se usa |
| 4 | **Módulo Relé 5V 1 canal** | Relé optoacoplado 5V, carga máx 10A/250VAC, activo en LOW | 1 | Compatible con señales 3.3V del ESP32 |
| 5 | **Servomotor MG995** | Servo de metal, torque 9.4–13 kg·cm, ángulo 0–180°, alimentación 4.8–7.2V | 1 | Con cables de señal, positivo y negativo |
| 6 | **Bomba sumergible 5V** | Bomba de agua pequeña DC 5V, caudal ~80–120 L/h | 1 | Usar con manguera de silicona |

---

## Componentes de Soporte

| N° | Componente | Descripción | Cantidad |
|----|-----------|-------------|----------|
| 7 | Fuente de alimentación 5V / 2A | Para servo MG995 y bomba de agua | 1 |
| 8 | Regulador de voltaje 3.3V (o usar el del ESP32) | Para sensores que lo requieran | 1 |
| 9 | Protoboard 830 puntos | Para montaje y pruebas | 1 |
| 10 | Cables Dupont M-M | Para conexiones entre módulos | 20+ |
| 11 | Cables Dupont M-H | Para conexiones al ESP32 DevKit | 10+ |
| 12 | Cable USB Micro-B | Para programación del ESP32 | 1 |
| 13 | Resistencia 10kΩ | Pull-up para DHT11 si no está integrada | 1 |
| 14 | Condensador 100nF | Desacoplamiento de alimentación del ESP32 | 2 |
| 15 | Diodo 1N4007 | Protección contra corriente inversa de la bomba | 1 |
| 16 | Transistor NPN BC547 | Opcional: para manejar la bomba directamente | 1 |

---

## Herramientas Necesarias

| Herramienta | Uso |
|------------|-----|
| Multímetro | Verificar voltajes, continuidad y corrientes |
| Cautín y estaño | Soldadura de componentes (opcional) |
| Pinzas | Manipulación de componentes pequeños |
| Cable de manguera transparente | Para el sistema de riego |
| Recipiente con agua | Para pruebas de la bomba y sensor de suelo |

---

## Especificaciones Eléctricas Resumidas

| Componente | Voltaje de Operación | Corriente Típica | Interfaz ESP32 |
|-----------|---------------------|------------------|---------------|
| ESP32-WROOM-32 | 3.3V (USB 5V) | 80–240 mA (WiFi activo) | — |
| DHT11 | 3.3–5V | 2.5 mA | GPIO 4 (digital) |
| Sensor suelo capacitivo | 3.3–5V | < 5 mA | GPIO 34 (ADC) |
| Módulo relé 5V | 5V (bobina) / 3.3V (señal) | 70–90 mA (activado) | GPIO 26 (digital) |
| Servo MG995 | 4.8–7.2V (externo) | 100–900 mA | GPIO 18 (PWM) |
| Bomba 5V | 5V (externo) | 200–400 mA | Vía relé |

---

## Estimación de Consumo Total

| Componente | Corriente estimada |
|-----------|-------------------|
| ESP32 (WiFi activo) | ~200 mA |
| DHT11 | ~5 mA |
| Sensor suelo | ~5 mA |
| Relé (activado) | ~90 mA |
| Total ESP32 + sensores + relé | **~300 mA @ 3.3V/5V** |
| Servo MG995 (movimiento) | ~500 mA @ 5V |
| Bomba (operación) | ~300 mA @ 5V |
| **Total sistema completo** | **~1.1 A @ 5V** |

> **Recomendación:** Usar fuente de 5V/2A mínimo para el sistema completo.  
> El ESP32 puede alimentarse por USB del PC durante desarrollo.

---

## Consideraciones de Protección Eléctrica

1. **Diodo de protección flyback** en la bomba: las bobinas de los motores generan
   picos de voltaje al apagarse. Colocar diodo 1N4007 en paralelo con la bomba
   (cátodo al positivo).

2. **Optoacoplamiento del relé**: el módulo de relé tiene optoacoplador integrado,
   lo que aísla galvánicamente el ESP32 de la carga (bomba).

3. **GPIO34 solo como entrada**: en el ESP32, GPIO34–GPIO39 son pines de solo entrada
   (input-only). No usar como salida. Correcto para el sensor analógico de suelo.

4. **GND común**: aunque se usen fuentes de alimentación separadas, es obligatorio
   conectar los GND entre el ESP32, el servo y la bomba para que las señales de
   referencia sean coherentes.

5. **No alimentar servo desde ESP32**: el MG995 puede consumir hasta 900 mA en carga.
   El regulador 3.3V del ESP32 solo provee ~600 mA total. Usar fuente externa 5V.

---

## Valores de Calibración del Sensor de Suelo (ADC ESP32)

> El ESP32 usa ADC de **12 bits** → rango 0–4095 (≠ Arduino Uno: 10 bits, 0–1023).

| Condición | Valor ADC medido | Definido en config.h |
|-----------|-----------------|----------------------|
| Sensor en el aire (seco → 0%) | ~4100 | `SOIL_ADC_SECO = 4100` |
| Sensor sumergido en agua (húmedo → 100%) | ~1200 | `SOIL_ADC_MOJADO = 1200` |

**Para recalibrar con tu hardware específico:**
1. Activar `MODO_CALIBRACION 1` en `config.h` y cargar el firmware.
2. Abrir Monitor Serial a 115200 bps.
3. Registrar el valor ADC con el sensor en seco → actualizar `SOIL_ADC_SECO`.
4. Registrar el valor ADC con el sensor en agua → actualizar `SOIL_ADC_MOJADO`.
5. Desactivar `MODO_CALIBRACION 0` y cargar el firmware de nuevo.

**Si el ADC siempre devuelve 4095:** el sensor está desconectado o el pin flota. Verificar VCC→3.3V, GND→GND y AOUT→GPIO34.
