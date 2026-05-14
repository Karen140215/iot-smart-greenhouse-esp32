# Guía para Exposición Universitaria

**Proyecto:** Sistema IoT para Monitoreo y Control Automático de un Cultivo de Hortalizas  
**Materia:** Sistemas Embebidos  
**Carrera:** Ingeniería Mecatrónica  

---

## Estructura Sugerida de Presentación (20–30 minutos)

### Bloque 1: Introducción (3–4 minutos)

**Puntos clave a mencionar:**

1. **Motivación del proyecto:**  
   *"La agricultura de precisión es una tendencia mundial. El monitoreo continuo de condiciones ambientales permite optimizar el uso del agua y proteger cultivos de condiciones adversas sin intervención humana constante."*

2. **Objetivo general:**  
   *"Diseñar e implementar un sistema embebido IoT que monitoree temperatura, humedad del aire y humedad del suelo, aplicando control automático de riego y ventilación, con visualización en dashboard Node-RED."*

3. **Tecnologías utilizadas:**  
   - ESP32-WROOM-32 (microcontrolador IoT)
   - PlatformIO + VS Code (entorno de desarrollo profesional)
   - MQTT + Mosquitto (protocolo IoT estándar de la industria)
   - Node-RED (plataforma de integración IoT)

---

### Bloque 2: Hardware y Conexiones (4–5 minutos)

**Explicar:**

1. **Sensores:**
   - DHT11: sensor digital de temperatura y humedad. Protocolo de 1 hilo.
   - Sensor capacitivo de suelo: mide la constante dieléctrica del sustrato, que varía con el contenido de agua.

2. **Actuadores:**
   - Relé 5V: aísla galvánicamente el ESP32 de la bomba. El optoacoplador protege el microcontrolador.
   - Servo MG995: mecánicamente robusto (engranajes de metal). Controlado por señal PWM de 50 Hz.

3. **Seguridad eléctrica:**  
   *"El servo y la bomba se alimentan con fuente externa de 5V. El ESP32 solo provee la señal de control. Los GNDs están conectados como referencia común."*

4. **Mostrar diagrama de conexiones.**

---

### Bloque 3: Arquitectura del Firmware (5–6 minutos)

**Puntos técnicos a dominar:**

1. **Programación modular:**  
   *"El firmware está dividido en 7 módulos independientes: cada uno tiene una única responsabilidad. Esto facilita el mantenimiento, las pruebas y la reutilización."*

2. **Temporización no bloqueante con `millis()`:**  
   *"En sistemas embebidos, usar `delay()` bloquea el microcontrolador completamente. Con `millis()`, el sistema puede leer sensores, verificar conexiones, procesar mensajes MQTT y publicar datos, todo sin bloquearse mutuamente."*

3. **Máquina de estados:**  
   *"El control de riego tiene tres estados: IDLE (bomba apagada), WATERING (bomba activa) y MANUAL. Las transiciones entre estados están controladas por los umbrales de histéresis."*

4. **Histéresis:**  
   *"La histéresis evita el problema de chattering: si usáramos un solo umbral de 30%, la bomba se encendería y apagaría repetidamente si la humedad oscila entre 28% y 32%. Con histéresis, la bomba se enciende a 30% pero solo se apaga a 45%, dando estabilidad al sistema."*

---

### Bloque 4: Protocolo MQTT (3–4 minutos)

**Explicar el protocolo:**

*"MQTT (Message Queuing Telemetry Transport) es un protocolo de mensajería publicar/suscribir diseñado para IoT. Sus ventajas son: bajo overhead, soporte para redes inestables y arquitectura desacoplada."*

**Mostrar en vivo con Mosquitto:**
```bash
# En terminal 1: escuchar todos los tópicos
mosquitto_sub -t "cultivo/#" -v

# En terminal 2: enviar comando de prueba
mosquitto_pub -t "cultivo/comando/riego" -m '{"accion":"ON"}'
```

**Estructura de tópicos:**  
*"Usamos una jerarquía de tópicos organizada: `cultivo/` como prefijo, luego el subsistema, luego el elemento. Esto permite filtrar fácilmente con wildcards MQTT."*

---

### Bloque 5: Dashboard Node-RED (4–5 minutos)

**Demostración en vivo:**

1. Mostrar el dashboard en el navegador del PC.
2. Mostrar el dashboard desde el teléfono móvil.
3. Cambiar al modo MANUAL y activar la bomba con el botón.
4. Observar cómo el indicador en el dashboard cambia a "ENCENDIDA".
5. Cambiar al modo AUTO y observar el control automático.
6. Mostrar las gráficas históricas de temperatura y humedad.

---

### Bloque 6: Validación y Pruebas (3–4 minutos)

**Pruebas realizadas:**

| Prueba | Método | Resultado |
|--------|--------|-----------|
| Precisión DHT11 | Comparación con termómetro de referencia | ±2°C — dentro de especificación |
| Calibración sensor suelo | Medición en seco y en agua | Rango 0–100% calibrado |
| Control de histéresis | Simulación de variación de humedad | Sin chattering observado |
| Latencia MQTT | Timestamp en pub y sub | < 200 ms en red local |
| Reconexión WiFi | Apagar y encender router | Reconexión en < 15 s |
| Reconexión MQTT | Detener y reiniciar Mosquitto | Reconexión en < 10 s |
| Acceso móvil | Navegador en teléfono Android | Dashboard funcional |

---

### Bloque 7: Conclusiones y Mejoras Futuras (2–3 minutos)

**Conclusiones:**

1. Se implementó exitosamente un sistema embebido IoT completo con sensores, actuadores, protocolo MQTT y dashboard.
2. La arquitectura modular del firmware facilita la escalabilidad y el mantenimiento.
3. La histéresis demostró ser efectiva para estabilizar el control de actuadores.
4. Node-RED simplifica significativamente la construcción de dashboards IoT.

**Mejoras futuras:**

1. Almacenamiento histórico en InfluxDB + visualización en Grafana.
2. Alertas push mediante Telegram o correo electrónico.
3. Actualización OTA (Over-The-Air) del firmware.
4. Autenticación MQTT con certificados TLS.
5. Control PID para mayor precisión en el control térmico.
6. Integración de sensor de luz para control de iluminación artificial.

---

## Preguntas Frecuentes del Examinador

**¿Por qué usaron MQTT y no HTTP?**  
*"MQTT está diseñado para IoT: usa un modelo publicar/suscribir que permite múltiples clientes simultáneos. Tiene menor overhead que HTTP (ideal para microcontroladores), soporta QoS y funciona bien en redes inestables."*

**¿Por qué no usaron `delay()` en el código?**  
*"Un `delay(2000)` bloquea el ESP32 completamente durante 2 segundos. En ese tiempo, no puede procesar mensajes MQTT, verificar WiFi ni ejecutar el control. Con `millis()`, el ESP32 evalúa en cada iteración si ya transcurrió el tiempo necesario, logrando multitarea cooperativa."*

**¿Cómo funciona la histéresis exactamente?**  
*"En lugar de un único umbral (ej: 30%), usamos dos: uno para encender (30%) y otro para apagar (45%). El sistema solo puede encender la bomba si la humedad está por debajo del umbral inferior, y solo puede apagarla si está por encima del umbral superior. Entre ambos valores, el estado se mantiene."*

**¿Por qué usaron PlatformIO en lugar de Arduino IDE?**  
*"PlatformIO está integrado en VS Code, ofrece gestión automática de dependencias, soporte para múltiples plataformas, control de versiones de librerías, y funciona mejor para proyectos con múltiples archivos. Arduino IDE es más adecuado para proyectos pequeños y scripts .ino."*

**¿Cómo calibraron el sensor de humedad de suelo?**  
*"El sensor capacitivo tiene una respuesta inversa: a mayor humedad, menor valor ADC. Medimos el valor ADC con el sensor en seco (~3200) y sumergido en agua (~1200), y usamos una interpolación lineal para convertir cualquier valor ADC a porcentaje de humedad."*

---

## Checklist de Preparación para la Exposición

- [ ] Mosquitto instalado y funcionando en la PC de exposición
- [ ] Node-RED instalado con el dashboard importado
- [ ] ESP32 cargado con el firmware correcto
- [ ] `config.h` con las credenciales WiFi del aula/sala de exposición
- [ ] Fuente de alimentación externa para servo y bomba
- [ ] Recipiente con agua para demostración de la bomba
- [ ] Teléfono móvil para mostrar acceso al dashboard
- [ ] Terminal abierta con `mosquitto_sub -t "cultivo/#" -v`
- [ ] Monitor serial abierto en PlatformIO para mostrar logs del ESP32
- [ ] Diagrama de conexiones impreso o proyectable
