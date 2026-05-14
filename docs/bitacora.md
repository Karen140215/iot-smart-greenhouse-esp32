# Bitácora Técnica de Desarrollo

**Proyecto:** Sistema IoT para Monitoreo y Control de Cultivo de Hortalizas  
**Materia:** Sistemas Embebidos — Ingeniería Mecatrónica  
**Plataforma:** ESP32-WROOM-32 + Node-RED + MQTT  

---

## Formato de Registro

Cada entrada incluye: fecha, descripción de la actividad, resultado, problemas encontrados y soluciones aplicadas.

---

## Semana 1 — Configuración del entorno y arquitectura

### Entrada 1 — Instalación del entorno de desarrollo

**Fecha:** [completar con fecha real]  
**Actividad:** Instalación de Visual Studio Code + extensión PlatformIO IDE.  
**Resultado:** Entorno de desarrollo funcional.  
**Observaciones:**
- Se instaló VS Code desde el sitio oficial.
- Se instaló la extensión PlatformIO desde el Marketplace de VS Code.
- Se creó el proyecto con `platform = espressif32`, `board = esp32dev`, `framework = arduino`.
- PlatformIO descargó automáticamente el toolchain de ESP32 (~800 MB).

**Lección aprendida:** PlatformIO gestiona las dependencias de librerías en `platformio.ini`, eliminando la necesidad de instalar librerías manualmente como en Arduino IDE.

---

### Entrada 2 — Diseño de la arquitectura modular

**Fecha:** [completar con fecha real]  
**Actividad:** Diseño de la estructura de módulos del firmware.  
**Resultado:** Definición de 7 módulos con responsabilidades claras.  
**Decisión de diseño:** Se separó la lógica IoT (serialización JSON, comandos MQTT) en un módulo propio (`iot_manager`) para mantener `mqtt_manager` enfocado solo en la capa de transporte.

**Razón:** El principio de responsabilidad única (SRP) facilita el mantenimiento y las pruebas individuales de cada módulo.

---

## Semana 2 — Implementación del firmware base

### Entrada 3 — Implementación de WiFi con reconexión

**Fecha:** [completar con fecha real]  
**Actividad:** Implementación de `wifi_manager.cpp` con reconexión automática.  
**Resultado:** Conexión WiFi funcional con reconexión periódica.  
**Problema encontrado:** La función `WiFi.begin()` puede demorar hasta 10 segundos en conectarse. Bloquear el sistema en este punto es aceptable solo durante el `setup()`.  
**Solución:** Se usa un bucle bloqueante SOLO durante la inicialización. En el loop principal, `checkWiFiConnection()` es no bloqueante.

---

### Entrada 4 — Integración del sensor DHT11

**Fecha:** [completar con fecha real]  
**Actividad:** Implementación de la lectura del DHT11 con validación.  
**Resultado:** Lectura de temperatura y humedad con detección de errores.  
**Problema encontrado:** El DHT11 ocasionalmente retorna `NaN` (Not a Number) cuando no tiene tiempo suficiente entre lecturas.  
**Solución:** Se configuró el intervalo de lectura en 2000 ms (INTERVAL_SENSOR_MS), que es mayor al mínimo de 1000 ms que requiere el DHT11. Además, se verifica `isnan()` antes de aceptar la lectura.

**Código clave:**
```cpp
float tempRaw = dht.readTemperature();
if (!isnan(tempRaw) && tempRaw >= TEMP_MIN_VALIDA && tempRaw <= TEMP_MAX_VALIDA) {
    sensorData.temperatura = tempRaw;
    sensorData.temperaturaValida = true;
}
```

---

### Entrada 5 — Calibración del sensor capacitivo de suelo

**Fecha:** [completar con fecha real]  
**Actividad:** Calibración del sensor capacitivo de humedad de suelo.  
**Procedimiento:**
1. Con el sensor completamente seco, leer el ADC: valor = **[registrar valor]**
2. Con el sensor sumergido en agua, leer el ADC: valor = **[registrar valor]**
3. Actualizar `SOIL_ADC_SECO` y `SOIL_ADC_MOJADO` en `config.h`.

**Resultado:** Mapeo lineal calibrado entre ADC y porcentaje de humedad.  
**Observación importante:** El ADC del ESP32 tiene ligeras no-linealidades cerca de 0V y 3.3V. Se promedian 5 muestras para reducir el ruido. Para mayor precisión, se puede usar una corrección por tabla de búsqueda (look-up table).

---

### Entrada 6 — Implementación del cliente MQTT

**Fecha:** [completar con fecha real]  
**Actividad:** Configuración de PubSubClient y conexión al broker Mosquitto.  
**Resultado:** Cliente MQTT funcional con suscripción a 3 tópicos de comandos.  
**Problema encontrado:** El tamaño máximo por defecto del paquete MQTT en PubSubClient es 256 bytes. El payload JSON del sistema puede superar este límite.  
**Solución:** Se agregó en `platformio.ini`:
```ini
build_flags = -D MQTT_MAX_PACKET_SIZE=512
```
Esto amplía el buffer interno de PubSubClient a 512 bytes.

---

## Semana 3 — Control automático e histéresis

### Entrada 7 — Implementación de la histéresis

**Fecha:** [completar con fecha real]  
**Actividad:** Implementación del control con histéresis usando máquina de estados.  
**Resultado:** Control estable sin chattering en bomba ni servo.  
**Prueba:** Se simuló la variación de humedad alrededor del umbral (30%) con valores de prueba. Sin histéresis, la bomba se encendería y apagaría continuamente si la humedad oscila entre 28% y 32%. Con histéresis (umbral apagado en 45%), la bomba permanece encendida hasta alcanzar 45%.

---

### Entrada 8 — Prueba de control térmico

**Fecha:** [completar con fecha real]  
**Actividad:** Prueba del servo MG995 con señal de control del ESP32.  
**Resultado:** Servo funcional en rango 0°–90°.  
**Observación:** El servo MG995 requiere una fuente de alimentación externa. Al intentar alimentarlo desde el pin de 5V del ESP32, el regulador de voltaje se sobrecalentó. Se confirmó la necesidad de fuente externa.

---

## Semana 4 — IoT y Dashboard Node-RED

### Entrada 9 — Configuración de Mosquitto Broker

**Fecha:** [completar con fecha real]  
**Actividad:** Instalación y configuración de Mosquitto en Windows.  
**Comando de instalación:** Descargado desde mosquitto.org.  
**Configuración usada:**
```conf
listener 1883
allow_anonymous true
```
**Verificación:**
```bash
mosquitto_sub -t "cultivo/#" -v
```

---

### Entrada 10 — Construcción del flow Node-RED

**Fecha:** [completar con fecha real]  
**Actividad:** Construcción del dashboard Node-RED con gauges, gráficas, botones y alertas.  
**Resultado:** Dashboard funcional accesible en `http://localhost:1880/ui`.  
**Librerías instaladas:**
- `node-red-dashboard` — para todos los widgets de UI.
**Procedimiento de instalación:**
```bash
# En la carpeta de Node-RED
npm install node-red-dashboard
```

---

### Entrada 11 — Prueba de integración end-to-end

**Fecha:** [completar con fecha real]  
**Actividad:** Prueba completa del sistema: ESP32 → Mosquitto → Node-RED.  
**Resultado:** Sistema funcionando de extremo a extremo.  
**Latencia medida:** ~200 ms desde lectura del sensor hasta actualización del dashboard.  
**Pruebas realizadas:**
- [x] Lectura correcta de temperatura y humedad (DHT11)
- [x] Lectura correcta de humedad de suelo (sensor capacitivo)
- [x] Activación automática de bomba cuando humedad < 30%
- [x] Desactivación de bomba cuando humedad > 45%
- [x] Apertura automática de ventilación cuando temperatura > 30°C
- [x] Cierre de ventilación cuando temperatura < 27°C
- [x] Recepción de comandos desde Node-RED
- [x] Cambio de modo AUTO/MANUAL
- [x] Reconexión tras pérdida de WiFi
- [x] Reconexión tras pérdida de MQTT
- [x] Alertas en dashboard
- [x] Visualización en móvil

---

## Observaciones Finales

### Decisiones de diseño

1. **Temporización con `millis()`:** Se eligió sobre FreeRTOS por simplicidad. Para proyectos más complejos, FreeRTOS con tareas independientes sería la solución adecuada.

2. **ArduinoJson v6:** Se eligió sobre v7 por mayor compatibilidad y documentación disponible.

3. **QoS 0 para MQTT:** Suficiente para monitoreo de sensores donde una pérdida ocasional es aceptable. Para comandos críticos, QoS 1 garantizaría entrega.

4. **Módulo relé activo en LOW:** La mayoría de módulos de relé con optoacoplador usan lógica inversa. Se documenta y parametriza en `actuator_manager.cpp`.

### Posibles Mejoras Futuras

1. Almacenamiento de datos históricos en base de datos (InfluxDB + Grafana).
2. Autenticación MQTT con usuario/contraseña.
3. OTA (Over-The-Air) updates para actualizar el firmware sin cable USB.
4. Integración con Telegram o WhatsApp para alertas push.
5. Control PID en lugar de control ON/OFF para mayor precisión.
6. Sensor de luminosidad (BH1750) para control de iluminación artificial.
7. Sensor de CO₂ (MH-Z19B) para monitoreo de calidad del aire.
8. Panel solar + batería para operación autónoma.
9. Múltiples zonas de riego con diferentes sensores.
10. Integración con API de clima para anticipar condiciones.
