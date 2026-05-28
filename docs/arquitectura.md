# Arquitectura del Sistema

**Proyecto:** Sistema IoT para Monitoreo y Control de Cultivo de Hortalizas  
**Versión:** 1.0.0  

---

## 1. Visión General

El sistema implementa una arquitectura IoT de tres capas:

```
┌───────────────────────────────────────────────────────────────────┐
│  CAPA DE PERCEPCIÓN (Edge / Dispositivo Embebido)                 │
│                                                                   │
│   Sensores                  Actuadores                            │
│   ┌──────────┐              ┌──────────────┐                      │
│   │  DHT11   │              │ Relé + Bomba │                      │
│   │ T° y HA  │              │    (Riego)   │                      │
│   └────┬─────┘              └──────┬───────┘                      │
│        │                           │                              │
│   ┌────┴─────────────────────┐     │                              │
│   │     ESP32-WROOM-32       ├─────┘                              │
│   │  Firmware modular C++    │                                    │
│   │  PlatformIO + Arduino    │                                    │
│   └────────────┬─────────────┘                                    │
│   ┌────────────┘                                                  │
│   │  Sensor Suelo     Servo MG995                                 │
│   └───────────────────────────────                                │
└─────────────────────┬─────────────────────────────────────────────┘
                      │ WiFi 802.11 b/g/n
                      │ TCP/IP
                      ▼
┌───────────────────────────────────────────────────────────────────┐
│  CAPA DE TRANSPORTE (Red Local)                                   │
│                                                                   │
│   ┌─────────────────────────────────┐                             │
│   │   Mosquitto MQTT Broker         │                             │
│   │   PC Windows — puerto 1883      │                             │
│   │   Protocolo: MQTT v3.1.1        │                             │
│   └─────────────────────────────────┘                             │
└─────────────────────┬─────────────────────────────────────────────┘
                      │
                      ▼
┌───────────────────────────────────────────────────────────────────┐
│  CAPA DE APLICACIÓN (Procesamiento y Visualización)               │
│                                                                   │
│   ┌─────────────────────────────────┐                             │
│   │   Node-RED Dashboard            │                             │
│   │   PC Windows — puerto 1880      │                             │
│   │   Dashboard UI responsive       │                             │
│   └─────────────────────────────────┘                             │
│                                                                   │
│   Acceso web: http://localhost:1880/ui                            │
│   Acceso móvil: http://[IP-PC]:1880/ui                           │
└───────────────────────────────────────────────────────────────────┘
```

---

## 2. Arquitectura del Firmware (Software)

### 2.1 Diagrama de Módulos

```
┌─────────────────────────────────────────────────────────────────┐
│                         main.cpp                                │
│              (Orquestador — bucle no bloqueante)                │
│                                                                 │
│   millis()  →  Temporización de 5 tareas independientes        │
└──────────┬──────────┬──────────┬──────────┬────────────────────┘
           │          │          │          │
    ┌──────▼──┐  ┌────▼────┐  ┌─▼──────┐  ┌▼───────────┐
    │  WiFi   │  │  MQTT   │  │Sensores│  │ Actuadores │
    │ Manager │  │ Manager │  │Manager │  │  Manager   │
    └──────┬──┘  └────┬────┘  └─┬──────┘  └┬───────────┘
           │          │          │           │
           └──────────┼──────────┼───────────┘
                      │          │
               ┌──────▼──────────▼──────┐
               │    Control Manager     │
               │  (Máquina de estados   │
               │   + Histéresis)        │
               └──────────┬─────────────┘
                          │
               ┌──────────▼─────────────┐
               │      IoT Manager       │
               │  (Serialización JSON   │
               │   + Comandos MQTT)     │
               └────────────────────────┘
```

### 2.2 Responsabilidades de Cada Módulo

| Módulo | Responsabilidad | Dependencias |
|--------|----------------|--------------|
| `main.cpp` | Inicialización y bucle principal con millis() | Todos |
| `wifi_manager` | Conexión/reconexión WiFi automática | Arduino WiFi.h |
| `mqtt_manager` | Cliente MQTT: connect, subscribe, publish, loop | PubSubClient, wifi_manager |
| `sensor_manager` | Lectura y validación de DHT11 y sensor de suelo | DHT.h |
| `actuator_manager` | Control del relé y servo con estado | ESP32Servo.h |
| `control_manager` | Máquina de estados + histéresis + modo AUTO/MANUAL | sensor_manager, actuator_manager |
| `iot_manager` | Serialización JSON, publicación y deserialización de comandos | mqtt_manager, ArduinoJson |
| `config.h` | Configuración centralizada (pines, umbrales, credenciales) | — |

---

## 3. Máquina de Estados del Sistema

### 3.1 Sistema de Riego

```
          humedad_suelo < UMBRAL_MIN (55%)
    ┌─────────────────────────────────────────┐
    │                                         ▼
┌───┴───┐    humedad_suelo > UMBRAL_MAX  ┌──────────┐
│ IDLE  │ ◄─────────────────────────────  │WATERING  │
│ Bomba │         (70%)                  │Bomba ON  │
│  OFF  │                                └──────────┘
└───────┘
    ▲                                          ▲
    │         Modo AUTO activado               │
    └──────────── MANUAL ─────────────────────┘
                   ↕ (comandos MQTT)
```

### 3.2 Control Térmico

```
        temperatura > UMBRAL_MAX (30°C)
    ┌───────────────────────────────────────┐
    │                                       ▼
┌───┴────┐   temperatura < UMBRAL_MIN  ┌──────────────┐
│NORMAL  │ ◄────────────────────────── │ VENTILATING  │
│Servo 0°│         (27°C)              │ Servo 90°    │
└────────┘                             └──────────────┘
    ▲                                        ▲
    │         Modo AUTO activado             │
    └────────────── MANUAL ─────────────────┘
```

---

## 4. Principio de Histéresis

La histéresis es una técnica de control que utiliza dos umbrales distintos para activar y desactivar un actuador. Esto evita el problema de **chattering** (encendido y apagado rápido repetitivo) que ocurre cuando el valor medido oscila alrededor de un único umbral.

### Ejemplo — Control de Riego:

```
Humedad (%)
    │
 70 ┤────────────────────── UMBRAL_MAX ─── Apagar bomba
    │              ╔═══════╗
 55 ┤────── UMBRAL_MIN ─────╚═══════════── Encender bomba
    │      ║               
    │══════╝               
    ├──────────────────────────────► Tiempo
    │
    Zona de histéresis: [55%, 70%]   (banda = 15%)
    En esta zona: mantener estado actual
```

> Los valores `UMBRAL_HUMEDAD_MIN = 55%` y `HISTERESIS_HUMEDAD = 15%` están parametrizados en `config.h`.  
> `UMBRAL_HUMEDAD_MAX` se calcula como `UMBRAL_HUMEDAD_MIN + HISTERESIS_HUMEDAD = 70%`.

### Implementación en código:

```cpp
// control_manager.cpp — controlIrrigation()
case IrrigationState::IDLE:
    if (data.humedadSustrato < UMBRAL_HUMEDAD_MIN) {  // < 55%
        irrigationState = IrrigationState::WATERING;
        setPump(true);
    }
    break;

case IrrigationState::WATERING:
    if (data.humedadSustrato > UMBRAL_HUMEDAD_MAX) {  // > 70%
        irrigationState = IrrigationState::IDLE;
        setPump(false);
    }
    break;
```

---

## 5. Flujo de Datos IoT

```
ESP32                    MQTT Broker              Node-RED
  │                          │                       │
  │   publish(sensores)      │                       │
  │ ─────────────────────────►                       │
  │                          │   forward(sensores)   │
  │                          │ ──────────────────────►
  │                          │                       │ → Actualizar gauges
  │                          │                       │ → Actualizar gráficas
  │                          │                       │ → Evaluar alertas
  │                          │                       │
  │                          │  publish(cmd/riego)   │
  │                          │ ◄──────────────────────
  │   receive(cmd/riego)     │                       │
  │ ◄─────────────────────────                       │
  │                          │                       │
  │ → handleMQTTCommand()    │                       │
  │ → setPump(true/false)    │                       │
  │ → publish(estado)        │                       │
  │ ─────────────────────────►                       │
```

---

## 6. Temporización No Bloqueante

El firmware implementa un sistema de multitarea cooperativa basado en `millis()`:

```cpp
// Patrón estándar de temporización no bloqueante
if (millis() - lastSensorRead >= INTERVAL_SENSOR_MS) {
    lastSensorRead = millis();
    readSensors();  // tarea de 2 segundos
}

if (millis() - lastPublish >= INTERVAL_PUBLISH_MS) {
    lastPublish = millis();
    publishIoTData();  // tarea de 5 segundos
}
```

**Ventajas sobre `delay()`:**
- El MCU no se bloquea esperando
- Múltiples tareas con distintos intervalos corren en "paralelo"
- Las tareas de mayor prioridad (como `processMQTT()`) se ejecutan en cada iteración
- El sistema responde inmediatamente a comandos MQTT externos

---

## 7. Arquitectura Hardware

```
                    Fuente USB (5V)
                         │
                    ESP32-WROOM-32
                    │           │
          GPIO4─────┤           ├─────GPIO34
          │   GPIO26─┤           ├─────GPIO18
          │         │           │
          │         └─────┬─────┘
          │               │ GND común
          │               │
    ┌─────▼──┐    ┌────────▼──────────────────┐
    │ DHT11  │    │    Fuente Externa 5V/2A   │
    │ T°, HA │    │  ┌────────┐  ┌──────────┐ │
    └────────┘    │  │  Relé  │  │Servo MG995│ │
                  │  │ + Bomba│  │  Techo   │ │
    ┌────────┐    │  └────────┘  └──────────┘ │
    │Sensor  │    └──────────────────────────-─┘
    │ Suelo  │
    └────────┘
```
