# Firmware — Sistema IoT para Cultivo de Hortalizas

## Descripción

Firmware modular en C++ para **ESP32-WROOM-32** que implementa el monitoreo ambiental
y el control automático de un cultivo de hortalizas mediante MQTT.

---

## Requisitos de Software

| Herramienta | Versión mínima | Descarga |
|-------------|---------------|----------|
| Visual Studio Code | Cualquier versión reciente | code.visualstudio.com |
| Extensión PlatformIO IDE | Última versión | marketplace.visualstudio.com |
| Extensión C/C++ (Microsoft) | Última versión | marketplace.visualstudio.com |

---

## Dependencias de Librerías (gestionadas por PlatformIO)

| Librería | Versión | Propósito |
|----------|---------|-----------|
| `adafruit/DHT sensor library` | ^1.4.6 | Lectura del sensor DHT11 |
| `adafruit/Adafruit Unified Sensor` | ^1.1.14 | Dependencia del DHT |
| `knolleary/PubSubClient` | ^2.8.0 | Cliente MQTT |
| `bblanchon/ArduinoJson` | ^6.21.5 | Serialización/deserialización JSON |
| `madhephaestus/ESP32Servo` | ^3.0.0 | Control PWM del servo MG995 |

PlatformIO descarga estas librerías automáticamente al compilar.

---

## Configuración Inicial (OBLIGATORIO antes de cargar)

Editar `include/config.h` y ajustar:

```cpp
// Credenciales WiFi
#define WIFI_SSID       "NOMBRE_DE_TU_RED"
#define WIFI_PASSWORD   "TU_CONTRASEÑA"

// IP del PC con Mosquitto (ver con ipconfig en Windows)
#define MQTT_BROKER_IP  "192.168.X.X"
```

---

## Cómo Compilar y Cargar

### Opción 1: VS Code con PlatformIO (recomendado)

1. Abrir VS Code
2. Abrir la carpeta `firmware/` con PlatformIO
3. Clic en el botón **Build** (✓) para compilar
4. Conectar el ESP32 por USB
5. Clic en **Upload** (→) para cargar
6. Clic en **Serial Monitor** (🔌) para ver la salida

### Opción 2: Línea de comandos PlatformIO

```bash
cd firmware/

# Compilar
pio run

# Compilar y cargar
pio run --target upload

# Monitor serial
pio device monitor --baud 115200

# Limpiar caché de compilación
pio run --target clean
```

---

## Estructura de Módulos

```
src/
├── main.cpp          → Bucle principal no bloqueante (millis)
├── wifi_manager.cpp  → Conexión y reconexión WiFi automática
├── mqtt_manager.cpp  → PubSubClient: conexión, suscripción, publicación
├── sensor_manager.cpp → DHT11 + ADC sensor suelo, validación y calibración
├── actuator_manager.cpp → Relé (bomba) + Servo MG995
├── control_manager.cpp  → Máquina de estados + histéresis
└── iot_manager.cpp   → Serialización JSON + despacho de comandos MQTT

include/
├── config.h          → ÚNICO archivo de configuración (pines, umbrales, MQTT)
└── *.h               → Interfaces de cada módulo
```

---

## Temporización No Bloqueante

El sistema usa `millis()` para temporizaciones. **Nunca se usa `delay()`** en el
bucle principal. Cada tarea tiene su propio intervalo configurable:

| Tarea | Intervalo | Constante en config.h |
|-------|-----------|----------------------|
| Lectura de sensores | 2 segundos | `INTERVAL_SENSOR_MS` |
| Publicación MQTT | 5 segundos | `INTERVAL_PUBLISH_MS` |
| Control automático | 1 segundo | `INTERVAL_CONTROL_MS` |
| Verificación WiFi | 30 segundos | `INTERVAL_WIFI_CHECK_MS` |
| Verificación MQTT | 5 segundos | `INTERVAL_MQTT_CHECK_MS` |
| Heartbeat | 10 segundos | `INTERVAL_HEARTBEAT_MS` |

---

## Calibración del Sensor de Suelo

El sensor capacitivo de humedad de suelo entrega un valor ADC inverso:
- **ADC alto (~3200)** = suelo completamente seco (0%)
- **ADC bajo (~1200)** = suelo en agua (100%)

### Procedimiento de calibración:

1. Con el sensor en **seco** (sin tierra), leer el ADC: actualizar `SOIL_ADC_SECO`
2. Con el sensor **sumergido en agua**, leer el ADC: actualizar `SOIL_ADC_MOJADO`
3. Recompilar y cargar el firmware

```cpp
// En config.h:
#define SOIL_ADC_SECO    3200    // ← Tu valor en seco
#define SOIL_ADC_MOJADO  1200    // ← Tu valor en agua
```

Para ver los valores ADC crudos, descomentar la siguiente línea en `sensor_manager.cpp`:
```cpp
// Serial.printf("[SENSOR] ADC crudo suelo: %d\n", adcPromedio);
```

---

## Salida del Monitor Serial

```
╔══════════════════════════════════════════╗
║   Sistema IoT - Cultivo de Hortalizas   ║
║   ESP32-WROOM-32  |  PlatformIO         ║
╚══════════════════════════════════════════╝
[WiFi] Conectando a SSID: MiRed..........
[WiFi] ✓ Conectado. IP local: 192.168.1.105
[MQTT] ✓ Conectado al broker Mosquitto.
[SISTEMA] ✓ Inicialización completada.

┌─────────────────────────────────────┐
│          LECTURA DE SENSORES        │
├─────────────────────────────────────┤
│ Temperatura:       28.0 °C          │
│ Humedad aire:      65.0 %           │
│ Humedad suelo:     25.0 %           │
└─────────────────────────────────────┘
[CTRL] Riego ACTIVADO — Humedad suelo: 25.0% < 30.0%
[IoT]  Datos publicados: T=28.0°C, HA=65.0%, HS=25.0%, Modo=AUTO
```
