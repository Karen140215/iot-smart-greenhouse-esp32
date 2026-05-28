/**
 * @file actuator_manager.cpp
 * @brief Implementación del módulo de control de actuadores.
 *        Controla el relé de la bomba de riego y el servomotor MG995
 *        de forma segura e independiente de la lógica de control.
 *
 * @project  Sistema IoT - Cultivo de Hortalizas
 * @platform ESP32-WROOM-32
 * @version  1.0.0
 *
 * @note SEGURIDAD ELÉCTRICA:
 *       - El relé se usa con lógica activa en LOW (LOW=ENCENDIDO para la mayoría
 *         de módulos de relé optoacoplados). Verificar con tu módulo.
 *       - El servo MG995 debe alimentarse con fuente externa 5V (no del ESP32).
 *       - GND de fuente externa debe compartirse con GND del ESP32.
 */

#include <Arduino.h>
#include "config.h"
#include <ESP32Servo.h>

// ============================================================
//  Objeto servo y variables de estado (alcance de archivo)
// ============================================================
static Servo servoMotor;
static bool  pumpState    = false;
static int   servoAngle   = SERVO_POS_CERRADO;

// Ajuste de lógica del relé: muchos módulos de relé de 5V son activos en LOW
// (LOW = relé activado). Cambiar a false si tu módulo es activo en HIGH.
static const bool RELE_ACTIVO_LOW = true;

// ============================================================
//  Función auxiliar interna
// ============================================================
static void writeRelay(bool encender) {
    if (RELE_ACTIVO_LOW) {
        digitalWrite(PIN_RELE_BOMBA, encender ? LOW : HIGH);
    } else {
        digitalWrite(PIN_RELE_BOMBA, encender ? HIGH : LOW);
    }
}

// ============================================================
//  initActuators
// ============================================================
void initActuators() {
    // Relé: configurar como salida y apagar inmediatamente (estado seguro)
    pinMode(PIN_RELE_BOMBA, OUTPUT);
    writeRelay(false);  // Bomba apagada al arrancar

    // Servo MG995: asignar canales PWM del ESP32Servo
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);

    servoMotor.setPeriodHertz(50);             // 50 Hz estándar para servos
    servoMotor.attach(PIN_SERVO, 500, 2400);   // Pulso MG995: 500µs–2400µs
    servoMotor.write(SERVO_POS_CERRADO);       // Posición inicial: cerrado
    servoAngle = SERVO_POS_CERRADO;

    Serial.printf("[ACT] Relé (bomba) en GPIO%d — Estado: APAGADO\n", PIN_RELE_BOMBA);
    Serial.printf("[ACT] Servo MG995 en GPIO%d — Posición: %d°\n",
                  PIN_SERVO, SERVO_POS_CERRADO);
}

// ============================================================
//  setPump
// ============================================================
void setPump(bool encender) {
    if (pumpState == encender) {
        return;  // Sin cambio de estado → no actuar (evita escrituras innecesarias)
    }

    pumpState = encender;
    writeRelay(encender);

    Serial.printf("[ACT] Bomba de riego: %s\n", encender ? "ENCENDIDA" : "APAGADA");
}

// ============================================================
//  setServoPosition
// ============================================================
void setServoPosition(int angulo) {
    // Limitar ángulo al rango válido del servo MG995
    angulo = constrain(angulo, 0, 180);

    if (servoAngle == angulo) {
        return;  // Sin cambio de posición → no mover
    }

    servoAngle = angulo;
    servoMotor.write(angulo);

    Serial.printf("[ACT] Servo movido a: %d° (%s)\n",
                  angulo,
                  angulo >= SERVO_POS_ABIERTO ? "ABIERTO" : "CERRADO");
}

// ============================================================
//  Getters de estado
// ============================================================
bool getPumpState() {
    return pumpState;
}

int getServoAngle() {
    return servoAngle;
}

String getServoStateString() {
    return (servoAngle >= SERVO_POS_ABIERTO) ? String("ABIERTO") : String("CERRADO");
}
