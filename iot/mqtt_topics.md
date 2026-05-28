# Especificación de Tópicos MQTT

**Proyecto:** Sistema IoT para Cultivo de Hortalizas  
**Broker:** Mosquitto MQTT (local, puerto 1883)  
**Cliente ESP32:** `esp32-cultivo-01`  
**Protocolo:** MQTT v3.1.1  

---

## Convención de Nombres

```
cultivo/[subsistema]/[elemento]
```

- Todos los tópicos en minúsculas con guiones bajos.
- Jerarquía de máximo 3 niveles para mantener claridad.
- Prefijo `cultivo/` para filtrar fácilmente todo el sistema con wildcard `cultivo/#`.

---

## Tópicos de Telemetría (ESP32 → Broker → Node-RED)

| Tópico | QoS | Retain | Tipo | Descripción |
|--------|-----|--------|------|-------------|
| `cultivo/sensores` | 0 | No | JSON | Payload completo con todos los datos |
| `cultivo/temperatura` | 0 | No | Float | Temperatura en °C como texto |
| `cultivo/humedad_aire` | 0 | No | Float | Humedad relativa del aire en % |
| `cultivo/humedad_sustrato` | 0 | No | Float | Humedad del suelo en % |
| `cultivo/bomba_riego/estado` | 0 | Yes | String | "ON" o "OFF" |
| `cultivo/control_termico/estado` | 0 | Yes | String | "ABIERTO" o "CERRADO" |
| `cultivo/modo` | 0 | Yes | String | "AUTO" o "MANUAL" |
| `cultivo/sistema/estado` | 0 | Yes | JSON | Estado de conexión ("ONLINE") |
| `cultivo/heartbeat` | 0 | No | JSON | Latido periódico con IP y uptime |

---

## Tópicos de Comandos (Node-RED → Broker → ESP32)

| Tópico | QoS | Descripción | Payload esperado |
|--------|-----|-------------|-----------------|
| `cultivo/comando/riego` | 0 | Control manual de la bomba | Ver abajo |
| `cultivo/comando/servo` | 0 | Control manual del servo | Ver abajo |
| `cultivo/comando/modo` | 0 | Cambio de modo AUTO/MANUAL | Ver abajo |

---

## Formatos de Payload

### Tópico: `cultivo/sensores` (publicación principal)

```json
{
  "temperatura": 28.5,
  "humedad_aire": 71.0,
  "humedad_sustrato": 43.0,
  "bomba_riego": "ON",
  "control_termico": "OFF",
  "modo": "AUTO",
  "uptime_s": 12345
}
```

### Tópico: `cultivo/heartbeat`

```json
{
  "estado": "ONLINE",
  "ip": "192.168.1.105",
  "uptime_s": 12345,
  "firmware": "1.0.0",
  "modo": "AUTO"
}
```

### Tópico: `cultivo/sistema/estado`

```json
{ "estado": "ONLINE" }
```

---

## Formatos de Comandos

### `cultivo/comando/riego` — Control de bomba

```json
{ "accion": "ON" }    // Encender bomba (modo manual)
{ "accion": "OFF" }   // Apagar bomba
```

### `cultivo/comando/servo` — Control de servo

```json
{ "angulo": 90 }        // Mover servo a 90° (ventilación abierta)
{ "angulo": 0 }         // Mover servo a 0° (ventilación cerrada)
{ "accion": "ABRIR" }   // Alias para ángulo SERVO_POS_ABIERTO
{ "accion": "CERRAR" }  // Alias para ángulo SERVO_POS_CERRADO
```

### `cultivo/comando/modo` — Cambio de modo

```json
{ "modo": "AUTO" }    // Activar control automático
{ "modo": "MANUAL" }  // Activar control manual
```

---

## Comandos de Prueba con Mosquitto CLI

```bash
# Suscribirse a todos los tópicos del sistema
mosquitto_sub -h localhost -t "cultivo/#" -v

# Suscribirse solo a sensores
mosquitto_sub -h localhost -t "cultivo/sensores" -v

# Encender bomba manualmente
mosquitto_pub -h localhost -t "cultivo/comando/riego" -m '{"accion":"ON"}'

# Apagar bomba
mosquitto_pub -h localhost -t "cultivo/comando/riego" -m '{"accion":"OFF"}'

# Abrir ventilación (servo a 90°)
mosquitto_pub -h localhost -t "cultivo/comando/servo" -m '{"angulo":90}'

# Cambiar a modo manual
mosquitto_pub -h localhost -t "cultivo/comando/modo" -m '{"modo":"MANUAL"}'

# Cambiar a modo automático
mosquitto_pub -h localhost -t "cultivo/comando/modo" -m '{"modo":"AUTO"}'
```

---

## Configuración de Mosquitto (mosquitto.conf)

Para usar Mosquitto localmente sin autenticación (entorno de desarrollo):

```conf
# mosquitto.conf — configuración básica para desarrollo
listener 1883
allow_anonymous true
```

Para habilitar logging de mensajes:

```conf
log_type all
log_dest file C:\mosquitto\mosquitto.log
```

Iniciar Mosquitto con configuración personalizada:

```bash
mosquitto -c "C:\Program Files\mosquitto\mosquitto.conf" -v
```
