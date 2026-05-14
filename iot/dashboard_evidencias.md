# Dashboard Node-RED — Descripción y Guía de Uso

**Proyecto:** Sistema IoT para Cultivo de Hortalizas  
**Acceso local:** http://localhost:1880/ui  
**Acceso desde móvil:** http://[IP-del-PC]:1880/ui  

---

## Instalación de Node-RED

### Paso 1: Instalar Node.js

Descargar e instalar Node.js LTS desde: https://nodejs.org  
Verificar instalación:
```bash
node --version    # ej: v20.x.x
npm --version     # ej: 10.x.x
```

### Paso 2: Instalar Node-RED

```bash
npm install -g --unsafe-perm node-red
```

### Paso 3: Instalar el Dashboard

```bash
# Navegar a la carpeta de Node-RED (en Windows)
cd %USERPROFILE%\.node-red

# Instalar el módulo de dashboard
npm install node-red-dashboard
```

### Paso 4: Iniciar Node-RED

```bash
node-red
```

Abrir en navegador: **http://localhost:1880**

### Paso 5: Importar el Flow del Proyecto

1. En Node-RED, hacer clic en el menú ☰ (esquina superior derecha)
2. Seleccionar **Import**
3. Pegar el contenido de `iot/nodered_flow.json`
4. Hacer clic en **Import**
5. Hacer clic en **Deploy** (botón rojo, esquina superior derecha)
6. Abrir el dashboard: **http://localhost:1880/ui**

---

## Descripción del Dashboard

### Pestaña 1: Monitoreo

Muestra el estado en tiempo real de todos los sensores y el sistema.

#### Gauges (indicadores circulares)

| Widget | Rango | Colores |
|--------|-------|---------|
| Temperatura | 0–50°C | Verde < 25°C / Amarillo 25–35°C / Rojo > 35°C |
| Humedad del Aire | 0–100% | Rojo < 30% / Amarillo 30–60% / Verde > 60% |
| Humedad del Suelo | 0–100% | Rojo < 30% / Amarillo 30–60% / Verde > 60% |

#### Gráficas en tiempo real

- **Temperatura:** Histórico de la última hora.
- **Humedad del Aire:** Histórico de la última hora.
- **Humedad del Suelo:** Histórico de la última hora.
- Las gráficas muestran el eje X con hora:minuto:segundo.

#### Panel de estado

| Indicador | Valores posibles |
|-----------|-----------------|
| Bomba de Riego | ENCENDIDA / APAGADA |
| Control Térmico | VENTILACIÓN ACTIVA / VENTILACIÓN CERRADA |
| Modo de Operación | AUTO / MANUAL |
| Estado ESP32 | EN LÍNEA con IP, uptime y versión de firmware |

---

### Pestaña 2: Control

Permite controlar manualmente los actuadores.

#### Control de Riego

| Botón | Color | Acción | Comando MQTT |
|-------|-------|--------|-------------|
| ENCENDER BOMBA | Azul | Activa la bomba | `{"accion":"ON"}` → `cultivo/comando/riego` |
| APAGAR BOMBA | Gris | Apaga la bomba | `{"accion":"OFF"}` → `cultivo/comando/riego` |

#### Control Térmico

| Botón | Color | Acción | Comando MQTT |
|-------|-------|--------|-------------|
| ABRIR VENTILACIÓN | Naranja | Servo a 90° | `{"accion":"ABRIR"}` → `cultivo/comando/servo` |
| CERRAR VENTILACIÓN | Gris oscuro | Servo a 0° | `{"accion":"CERRAR"}` → `cultivo/comando/servo` |

#### Modo de Operación

| Botón | Color | Acción | Comando MQTT |
|-------|-------|--------|-------------|
| MODO AUTOMÁTICO | Verde | Activa control automático | `{"modo":"AUTO"}` → `cultivo/comando/modo` |
| MODO MANUAL | Naranja | Activa control manual | `{"modo":"MANUAL"}` → `cultivo/comando/modo` |

> **Nota:** Para que los botones de control funcionen, primero cambiar al **MODO MANUAL**.  
> En modo automático, el ESP32 puede revertir los cambios manuales.

---

### Pestaña 3: Alertas

Muestra el historial de alertas generadas por el sistema.

| Alerta | Condición | Mensaje |
|--------|-----------|---------|
| Suelo muy seco | humedad_sustrato < 20% | "⚠️ ALERTA: Suelo muy seco (XX%). Revisar sistema de riego." |
| Temperatura crítica | temperatura > 35°C | "🌡️ ALERTA: Temperatura crítica (XX°C). Verificar ventilación." |

Las alertas también aparecen como **notificaciones emergentes** en la esquina superior derecha del dashboard durante 5 segundos.

---

## Acceso desde Dispositivo Móvil

1. Asegurarse de que el PC y el teléfono estén en la **misma red WiFi**.
2. Obtener la IP del PC en Windows:
   ```bash
   ipconfig
   # Buscar: Dirección IPv4 (ej: 192.168.1.100)
   ```
3. En el navegador del teléfono, ingresar: **http://192.168.1.100:1880/ui**
4. El dashboard es responsive y se adapta automáticamente a pantallas pequeñas.

---

## Estructura del Flow Node-RED

```
MQTT in (cultivo/temperatura)    → Gauge Temperatura + Gráfica Temperatura
MQTT in (cultivo/humedad_aire)   → Gauge Humedad Aire + Gráfica Humedad Aire
MQTT in (cultivo/humedad_sustrato) → Gauge Humedad Suelo + Gráfica Humedad Suelo
MQTT in (cultivo/sensores JSON)  → Función distribuir → todos los widgets
                                  → Función alerta suelo
                                  → Función alerta temperatura
MQTT in (cultivo/bomba_riego/estado) → Texto estado bomba
MQTT in (cultivo/control_termico/estado) → Texto estado servo
MQTT in (cultivo/modo)           → Texto modo operación
MQTT in (cultivo/heartbeat)      → Texto estado ESP32

Botón Encender Bomba   → MQTT out (cultivo/comando/riego)
Botón Apagar Bomba     → MQTT out (cultivo/comando/riego)
Botón Abrir Ventilación  → MQTT out (cultivo/comando/servo)
Botón Cerrar Ventilación → MQTT out (cultivo/comando/servo)
Botón Modo AUTO        → MQTT out (cultivo/comando/modo)
Botón Modo MANUAL      → MQTT out (cultivo/comando/modo)
```
