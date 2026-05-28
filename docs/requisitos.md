# Requisitos del Sistema

**Proyecto:** Sistema IoT para Monitoreo y Control de Cultivo de Hortalizas  
**Metodología:** Scrum / Kanban  
**Versión:** 1.0.0  

---

## 1. Historias de Usuario

### HU-01 — Monitoreo ambiental en tiempo real

> **Como** agricultor o estudiante,  
> **quiero** ver la temperatura, humedad del aire y humedad del suelo en tiempo real,  
> **para** tomar decisiones informadas sobre el cultivo sin necesidad de medición manual.

**Criterios de aceptación:**
- El sistema lee sensores cada 2 segundos.
- Los datos se muestran en el dashboard de Node-RED con actualización automática.
- El dashboard es accesible desde un navegador en PC y teléfono móvil.
- Los valores se muestran en gauges con indicadores de color (verde/amarillo/rojo).

---

### HU-02 — Control automático de riego

> **Como** agricultor,  
> **quiero** que el sistema active automáticamente el riego cuando el suelo está seco,  
> **para** mantener la humedad óptima del cultivo sin intervención manual constante.

**Criterios de aceptación:**
- La bomba se activa cuando la humedad del suelo baja del 55% (`UMBRAL_HUMEDAD_MIN`).
- La bomba se apaga cuando la humedad del suelo supera el 70% (`UMBRAL_HUMEDAD_MAX`).
- El sistema implementa histéresis de 15% para evitar encendidos y apagados frecuentes.
- El estado de la bomba se publica por MQTT y se visualiza en el dashboard.

---

### HU-03 — Control térmico automático

> **Como** agricultor,  
> **quiero** que el sistema abra la ventilación cuando la temperatura es demasiado alta,  
> **para** proteger el cultivo de golpes de calor.

**Criterios de aceptación:**
- El servo abre la ventilación cuando la temperatura supera los 30°C.
- El servo cierra la ventilación cuando la temperatura baja de 27°C.
- La histéresis de 3°C evita vibraciones del mecanismo.
- El estado del servo se muestra en el dashboard.

---

### HU-04 — Control manual desde dashboard

> **Como** operador,  
> **quiero** poder encender/apagar la bomba y mover el servo manualmente desde el dashboard,  
> **para** intervenir cuando necesito realizar pruebas o ajustes manuales.

**Criterios de aceptación:**
- El dashboard tiene botones para encender y apagar la bomba manualmente.
- El dashboard tiene botones para abrir y cerrar la ventilación.
- Los comandos llegan al ESP32 en menos de 2 segundos.
- El control manual se activa cambiando al modo MANUAL.

---

### HU-05 — Cambio de modo de operación

> **Como** operador,  
> **quiero** alternar entre modo automático y modo manual,  
> **para** tener control total sobre el sistema cuando sea necesario.

**Criterios de aceptación:**
- El dashboard tiene botones para cambiar entre modo AUTO y MANUAL.
- En modo MANUAL, el control automático queda suspendido.
- En modo AUTO, el control manual se ignora y se retoma el control automático.
- El modo actual se muestra claramente en el dashboard.

---

### HU-06 — Alertas de condiciones críticas

> **Como** agricultor,  
> **quiero** recibir alertas cuando las condiciones son críticas,  
> **para** actuar rápidamente antes de que el cultivo se dañe.

**Criterios de aceptación:**
- Se genera una alerta visible cuando la humedad del suelo baja del 20%.
- Se genera una alerta cuando la temperatura supera los 35°C.
- Las alertas aparecen como notificaciones en el dashboard.
- Las alertas se registran en el panel de alertas de Node-RED.

---

### HU-07 — Reconexión automática

> **Como** operador,  
> **quiero** que el sistema se reconecte automáticamente si pierde WiFi o conexión MQTT,  
> **para** garantizar operación continua sin intervención manual.

**Criterios de aceptación:**
- El sistema verifica la conexión WiFi cada 30 segundos.
- El sistema verifica la conexión MQTT cada 5 segundos.
- La reconexión es transparente y no interrumpe el control automático.
- El ESP32 publica un heartbeat periódico que indica que está en línea.

---

## 2. Product Backlog

### Sprint 1 — Infraestructura base

| ID | Historia | Prioridad | Estado |
|----|----------|-----------|--------|
| T-01 | Configurar PlatformIO y estructura de proyecto | Alta | ✅ Completado |
| T-02 | Implementar módulo WiFi con reconexión | Alta | ✅ Completado |
| T-03 | Implementar módulo MQTT con PubSubClient | Alta | ✅ Completado |
| T-04 | Implementar lectura del DHT11 con validación | Alta | ✅ Completado |
| T-05 | Implementar lectura del sensor de suelo con calibración | Alta | ✅ Completado |

### Sprint 2 — Control y actuadores

| ID | Historia | Prioridad | Estado |
|----|----------|-----------|--------|
| T-06 | Implementar módulo de control de relé (bomba) | Alta | ✅ Completado |
| T-07 | Implementar control de servo MG995 con ESP32Servo | Alta | ✅ Completado |
| T-08 | Implementar control automático con histéresis (riego) | Alta | ✅ Completado |
| T-09 | Implementar control automático con histéresis (temperatura) | Alta | ✅ Completado |
| T-10 | Implementar modo AUTO/MANUAL con máquina de estados | Alta | ✅ Completado |

### Sprint 3 — IoT y Dashboard

| ID | Historia | Prioridad | Estado |
|----|----------|-----------|--------|
| T-11 | Serialización JSON de telemetría con ArduinoJson | Alta | ✅ Completado |
| T-12 | Deserialización JSON de comandos MQTT | Alta | ✅ Completado |
| T-13 | Crear flow de Node-RED con gauges y gráficas | Alta | ✅ Completado |
| T-14 | Implementar botones de control en Node-RED | Alta | ✅ Completado |
| T-15 | Implementar sistema de alertas en Node-RED | Media | ✅ Completado |

### Sprint 4 — Documentación y pruebas

| ID | Historia | Prioridad | Estado |
|----|----------|-----------|--------|
| T-16 | Documentar arquitectura hardware/software | Media | ✅ Completado |
| T-17 | Documentar tópicos MQTT | Media | ✅ Completado |
| T-18 | Elaborar bitácora técnica | Media | ✅ Completado |
| T-19 | Preparar guía de exposición | Baja | ✅ Completado |
| T-20 | Validar sistema completo end-to-end | Alta | ✅ Completado |

---

## 3. Requisitos Funcionales

| ID | Requisito | Módulo |
|----|-----------|--------|
| RF-01 | El sistema debe leer temperatura y humedad del aire cada 2 segundos | sensor_manager |
| RF-02 | El sistema debe leer humedad del suelo cada 2 segundos con promedio de 5 muestras | sensor_manager |
| RF-03 | El sistema debe validar el rango de todas las lecturas de sensores | sensor_manager |
| RF-04 | El sistema debe activar la bomba cuando humedad suelo < 55% (`UMBRAL_HUMEDAD_MIN`) | control_manager |
| RF-05 | El sistema debe desactivar la bomba cuando humedad suelo > 70% (`UMBRAL_HUMEDAD_MAX`) | control_manager |
| RF-06 | El sistema debe abrir ventilación cuando temperatura > 30°C | control_manager |
| RF-07 | El sistema debe cerrar ventilación cuando temperatura < 27°C | control_manager |
| RF-08 | El sistema debe publicar datos JSON cada 5 segundos vía MQTT | iot_manager |
| RF-09 | El sistema debe suscribirse a tópicos de comandos MQTT | mqtt_manager |
| RF-10 | El sistema debe procesar comandos de riego, servo y modo | iot_manager |
| RF-11 | El sistema debe reconectarse automáticamente a WiFi y MQTT | wifi_manager, mqtt_manager |
| RF-12 | El sistema debe publicar heartbeat cada 10 segundos | iot_manager |
| RF-13 | El dashboard debe mostrar datos en tiempo real | Node-RED |
| RF-14 | El dashboard debe permitir control manual de actuadores | Node-RED |
| RF-15 | El dashboard debe emitir alertas en condiciones críticas | Node-RED |

---

## 4. Requisitos No Funcionales

| ID | Requisito | Métrica |
|----|-----------|---------|
| RNF-01 | El firmware no debe usar `delay()` en el bucle principal | 0 instancias de delay() en loop() |
| RNF-02 | Tiempo de respuesta a comandos MQTT < 2 segundos | Medición con Mosquitto CLI |
| RNF-03 | El código debe estar organizado en módulos independientes | 7 módulos .cpp con sus .h |
| RNF-04 | El sistema debe reconectarse en < 15 segundos tras pérdida de WiFi | Prueba de desconexión |
| RNF-05 | Todos los archivos de configuración en un único config.h | 1 archivo de configuración |
| RNF-06 | El dashboard debe ser accesible desde dispositivos móviles | Prueba en navegador móvil |
| RNF-07 | El firmware debe arrancar sin errores con sensores desconectados | Validación de lecturas |
| RNF-08 | El sistema debe funcionar con resolución ADC de 12 bits | analogReadResolution(12) |
| RNF-09 | Las lecturas de suelo deben ser promedio de 5 muestras | Implementado en sensor_manager |
| RNF-10 | La histéresis debe estar parametrizada en config.h | HISTERESIS_HUMEDAD, HISTERESIS_TEMP |
