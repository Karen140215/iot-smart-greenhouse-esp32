# Diagrama de Conexiones del Hardware

**Proyecto:** Sistema IoT para Monitoreo y Control de Cultivo de Hortalizas  
**Plataforma:** ESP32-WROOM-32 DevKit v1  

---

## Tabla de Conexiones Completa

| Componente | Pin del Componente | Pin ESP32 | Color sugerido | Notas |
|-----------|-------------------|-----------|----------------|-------|
| **DHT11** | VCC | 3.3V | Rojo | Alimentación del sensor |
| **DHT11** | GND | GND | Negro | Tierra común |
| **DHT11** | DATA | GPIO 4 | Amarillo | Señal digital con pull-up |
| **Sensor Suelo** | VCC | 3.3V | Rojo | Alimentación del sensor |
| **Sensor Suelo** | GND | GND | Negro | Tierra común |
| **Sensor Suelo** | AOUT | GPIO 34 | Verde | Señal analógica (ADC) |
| **Módulo Relé** | VCC | 5V (USB o fuente) | Rojo | Alimentación de la bobina |
| **Módulo Relé** | GND | GND | Negro | Tierra común |
| **Módulo Relé** | IN | GPIO 26 | Naranja | Señal de control (LOW = ON) |
| **Módulo Relé** | COM | + Fuente externa | — | Terminal común del relé |
| **Módulo Relé** | NO | + Bomba | — | Normalmente abierto (NO) |
| **Bomba 5V** | + (positivo) | Relé NO | Rojo | Controlada por relé |
| **Bomba 5V** | - (negativo) | GND fuente externa | Negro | Tierra de fuente externa |
| **Servo MG995** | Señal (naranja/amarillo) | GPIO 18 | Naranja | PWM 50 Hz |
| **Servo MG995** | VCC (rojo) | 5V fuente externa | Rojo | NO conectar al ESP32 |
| **Servo MG995** | GND (marrón/negro) | GND fuente externa + ESP32 | Negro | GND compartido obligatorio |

---

## Diagrama ASCII de Conexiones

```
                          ESP32-WROOM-32
                    ┌──────────────────────┐
                    │                      │
    3.3V ───────────┤ 3V3              GND ├─── GND (común)
                    │                      │
    [DHT11 DATA] ───┤ GPIO4         GPIO18 ├─── [Servo MG995 SEÑAL]
                    │                      │
    [Relé IN] ──────┤ GPIO26        GPIO34 ├─── [Sensor Suelo AOUT]
                    │                      │
                    │  ...otras pines...   │
                    └──────────────────────┘

    DHT11                    Sensor Suelo
    ┌──────┐                 ┌──────────┐
    │ VCC  ├── 3.3V          │   VCC    ├── 3.3V
    │ GND  ├── GND           │   GND    ├── GND
    │ DATA ├── GPIO4         │   AOUT   ├── GPIO34
    └──────┘                 └──────────┘

    Módulo Relé              Fuente Externa 5V/2A
    ┌──────────┐             ┌────────────────┐
    │  VCC  ───┼── 5V        │ +5V ──┬── Relé COM
    │  GND  ───┼── GND       │       └── Servo VCC
    │  IN   ───┼── GPIO26    │ GND ──┬── Relé GND (bomba)
    │  COM  ───┼── +5V Ext   │       ├── Servo GND
    │  NO   ───┼── +Bomba    │       └── GND ESP32 (común)
    └──────────┘             └────────────────┘

    Bomba 5V
    ┌─────────┐
    │ (+) ────┼── Relé NO (salida)
    │ (-) ────┼── GND fuente externa
    └─────────┘
    [Diodo 1N4007 en paralelo: cátodo al (+)]
```

---

## Notas de Instalación

### DHT11
- Si el módulo no tiene resistencia pull-up integrada, colocar resistencia de **10kΩ**
  entre el pin DATA y VCC (3.3V).
- El DHT11 requiere al menos **1 segundo** entre lecturas. El firmware respeta este límite
  al configurar `INTERVAL_SENSOR_MS = 2000`.

### Sensor Capacitivo de Humedad de Suelo
- **GPIO34** es un pin de solo entrada (input-only) del ESP32. No puede usarse como salida.
- La salida analógica del sensor varía de 0 a ~3.3V.
- No sumergir la sección electrónica del sensor, solo la sonda.

### Módulo de Relé
- Verificar si tu módulo es **activo en LOW** (LOW = relé activo) o **activo en HIGH**.
  La mayoría de módulos de relé con optoacoplador son activos en LOW.
- Si el relé no activa con señal de 3.3V, puede requerir un transistor NPN como
  interfaz entre el GPIO del ESP32 y la entrada del relé.

### Servo MG995
- **Alimentación externa obligatoria.** El regulador del ESP32 no puede proveer la
  corriente necesaria (hasta 900 mA bajo carga).
- El pin de señal (GPIO18) sí va al ESP32 directamente.
- El **GND del servo DEBE conectarse al GND del ESP32** para referencia común de señal.
- Rango de pulso MG995: 500µs–2400µs (configurado en `actuator_manager.cpp`).

### Bomba de Agua
- Siempre conectar con el relé en posición **NO (Normalmente Abierto)**.
  En caso de fallo del sistema, el relé queda abierto y la bomba permanece apagada.
- Colocar **diodo flyback 1N4007** en paralelo con la bomba para proteger el relé.

---

## Advertencias Eléctricas

> ⚠️ **NUNCA conectar la bomba o el servo directamente a los pines del ESP32.**  
> Los GPIOs del ESP32 pueden suministrar máximo 12 mA por pin y 40 mA totales.
>
> ⚠️ **Siempre verificar la polaridad** antes de energizar el sistema.
>
> ⚠️ **GPIO34–GPIO39 son de solo lectura.** No usar como salidas digitales.
>
> ⚠️ **El ADC del ESP32 tiene no-linealidades** cerca de 0V y 3.3V.
>    El mapeo de calibración del sensor de suelo compensa esto parcialmente.
