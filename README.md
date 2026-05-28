# Sistema IoT para Monitoreo y Control Automático de un Cultivo de Hortalizas

**Materia:** Sistemas Embebidos  
**Carrera:** Ingeniería Mecatrónica  
**Plataforma:** ESP32-WROOM-32 | PlatformIO | Visual Studio Code  
**Protocolo IoT:** MQTT (Mosquitto) + Node-RED  
**Versión de Firmware:** 1.0.0  

---

## Descripción del Proyecto

Este proyecto implementa un **sistema embebido IoT** para el monitoreo ambiental y el control automático de un cultivo de hortalizas en ambiente controlado. El sistema adquiere datos de temperatura, humedad relativa del aire y humedad del suelo, aplica lógica de control automático con histéresis y comunica el estado completo del sistema mediante el protocolo MQTT hacia un dashboard Node-RED.

El firmware está desarrollado con **arquitectura modular** en C++ sobre el framework Arduino para ESP32, siguiendo buenas prácticas de sistemas embebidos: temporización no bloqueante con `millis()`, máquinas de estados, validación de sensores, reconexión automática WiFi/MQTT y serialización JSON.

---

## Características Principales

| Característica | Descripción |
|---|---|
| Plataforma | ESP32-WROOM-32 |
| Sensores | DHT11 + Sensor capacitivo de humedad de suelo |
| Actuadores | Relé 5V (bomba de riego) + Servo MG995 (ventilación) |
| Protocolo IoT | MQTT sobre WiFi |
| Broker | Mosquitto MQTT (local en PC Windows) |
| Dashboard | Node-RED con UI responsive para móvil |
| Programación | C++ modular con PlatformIO |
| Control | Automático con histéresis + Manual remoto (MQTT) |

---

## Estructura del Repositorio

```
proyecto-final-sistemas-embebidos/
│
├── README.md                        ← Este archivo
│
├── firmware/                        ← Código fuente del ESP32
│   ├── platformio.ini               ← Configuración PlatformIO
│   ├── README.md                    ← Guía de instalación y uso del firmware
│   ├── src/
│   │   ├── main.cpp                 ← Punto de entrada (setup/loop)
│   │   ├── wifi_manager.cpp         ← Gestión de conexión WiFi
│   │   ├── mqtt_manager.cpp         ← Gestión de conexión MQTT
│   │   ├── sensor_manager.cpp       ← Lectura y validación de sensores
│   │   ├── actuator_manager.cpp     ← Control de relé y servomotor
│   │   ├── control_manager.cpp      ← Lógica automática con histéresis
│   │   └── iot_manager.cpp          ← Serialización JSON y comandos MQTT
│   └── include/
│       ├── config.h                 ← Configuración central (pines, umbrales)
│       ├── wifi_manager.h
│       ├── mqtt_manager.h
│       ├── sensor_manager.h
│       ├── actuator_manager.h
│       ├── control_manager.h
│       └── iot_manager.h
│
├── docs/                            ← Documentación técnica
│   ├── requisitos.md                ← Requisitos funcionales y no funcionales
│   ├── arquitectura.md              ← Arquitectura hardware/software
│   └── bitacora.md                  ← Bitácora técnica de desarrollo
│
├── hardware/                        ← Documentación de hardware
│   ├── diagrama_conexiones.md       ← Tabla y descripción de conexiones
│   └── lista_componentes.md         ← Lista de materiales (BOM)
│
├── iot/                             ← Documentación IoT
│   ├── mqtt_topics.md               ← Especificación de tópicos MQTT
│   ├── nodered_flow.json            ← Flow Node-RED exportable e importable
│   └── dashboard_evidencias.md      ← Descripción del dashboard
│
└── presentacion/
    └── guia_exposicion.md           ← Guía para exposición universitaria
```

---

## Arquitectura del Sistema

```
┌─────────────────────────────────────────────────────────────────┐
│                         CAPA FÍSICA                             │
│  DHT11  ──→  ESP32  ←──  Sensor Suelo                         │
│               │                                                  │
│            Relé ──→ Bomba       Servo MG995 ──→ Ventilación    │
└────────────────────────────────┬────────────────────────────────┘
                                 │ WiFi / TCP
┌────────────────────────────────▼────────────────────────────────┐
│                      CAPA DE TRANSPORTE                         │
│              Mosquitto MQTT Broker (PC Local)                   │
│         cultivo/sensores | cultivo/comando/*                    │
└────────────────────────────────┬────────────────────────────────┘
                                 │
┌────────────────────────────────▼────────────────────────────────┐
│                       CAPA DE APLICACIÓN                        │
│              Node-RED Dashboard (PC Local)                      │
│     Gauges | Gráficas | Botones de control | Alertas            │
└─────────────────────────────────────────────────────────────────┘
```

---

## Instalación Rápida

### 1. Firmware ESP32

```bash
# Abrir el proyecto en VS Code con PlatformIO
# Editar firmware/include/config.h con:
#   - WIFI_SSID y WIFI_PASSWORD
#   - MQTT_BROKER_IP (IP del PC con Mosquitto)

# Calibrar sensor de suelo (opcional, solo si cambia el hardware):
#   1. Poner MODO_CALIBRACION 1 en config.h
#   2. Compilar y cargar, abrir Monitor Serial a 115200 bps
#   3. Registrar valor ADC en seco y en agua
#   4. Actualizar SOIL_ADC_SECO y SOIL_ADC_MOJADO en config.h
#   5. Poner MODO_CALIBRACION 0 y volver a compilar

# Compilar y cargar:
pio run --target upload
# Monitorear serial:
pio device monitor
```

### 2. Broker Mosquitto (Windows)

```bash
# Instalar Mosquitto desde: https://mosquitto.org/download/
# Iniciar el servicio:
net start mosquitto
# Verificar con:
mosquitto_sub -t "cultivo/#" -v
```

### 3. Node-RED

```bash
# Instalar Node.js desde: https://nodejs.org
# Instalar Node-RED:
npm install -g --unsafe-perm node-red
# Instalar dashboard:
npm install -g node-red-dashboard
# Iniciar:
node-red
# Importar el flow desde: iot/nodered_flow.json
```

---

## Tópicos MQTT

| Tópico | Dirección | Descripción |
|--------|-----------|-------------|
| `cultivo/sensores` | ESP32 → Broker | JSON completo con todos los datos |
| `cultivo/temperatura` | ESP32 → Broker | Temperatura en °C |
| `cultivo/humedad_aire` | ESP32 → Broker | Humedad del aire en % |
| `cultivo/humedad_sustrato` | ESP32 → Broker | Humedad del suelo en % |
| `cultivo/bomba_riego/estado` | ESP32 → Broker | "ON" / "OFF" |
| `cultivo/control_termico/estado` | ESP32 → Broker | "ABIERTO" / "CERRADO" |
| `cultivo/modo` | ESP32 → Broker | "AUTO" / "MANUAL" |
| `cultivo/comando/riego` | Broker → ESP32 | `{"accion":"ON"}` o `{"accion":"OFF"}` |
| `cultivo/comando/servo` | Broker → ESP32 | `{"angulo":90}` o `{"accion":"ABRIR"}` |
| `cultivo/comando/modo` | Broker → ESP32 | `{"modo":"AUTO"}` o `{"modo":"MANUAL"}` |

---

## Pines de Conexión

| Componente | Pin ESP32 | Tipo |
|------------|-----------|------|
| DHT11 (datos) | GPIO 4 | Digital entrada |
| Sensor humedad suelo | GPIO 34 | ADC entrada (solo lectura) |
| Relé bomba de riego | GPIO 26 | Digital salida |
| Servo MG995 (señal) | GPIO 18 | PWM salida |

> **IMPORTANTE:** El servo MG995 y la bomba se alimentan con **fuente externa 5V**.  
> El GND de la fuente externa **debe compartirse** con el GND del ESP32.

---

## Umbrales de Control

| Parámetro | Valor | Configurable en |
|-----------|-------|-----------------|
| Humedad suelo — encender bomba (UMBRAL_HUMEDAD_MIN) | < 55% | `config.h` |
| Humedad suelo — apagar bomba (UMBRAL_HUMEDAD_MAX) | > 70% | `config.h` |
| Histéresis de humedad (HISTERESIS_HUMEDAD) | 15% | `config.h` |
| Temperatura — abrir ventilación (UMBRAL_TEMP_MAX) | > 30°C | `config.h` |
| Temperatura — cerrar ventilación (UMBRAL_TEMP_MIN) | < 27°C | `config.h` |
| Histéresis de temperatura (HISTERESIS_TEMP) | 3°C | `config.h` |

> **Nota:** `UMBRAL_HUMEDAD_MAX = UMBRAL_HUMEDAD_MIN + HISTERESIS_HUMEDAD` y `UMBRAL_TEMP_MIN = UMBRAL_TEMP_MAX - HISTERESIS_TEMP` se calculan automáticamente en `config.h`.

---

## Equipo de Desarrollo

- Proyecto Final — Sistemas Embebidos — Ingeniería Mecatrónica
- Framework: Arduino + PlatformIO
- Lenguaje: C++
- Protocolo: MQTT v3.1.1

---

## Licencia

Proyecto académico — Uso educativo exclusivo.
