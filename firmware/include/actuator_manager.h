/**
 * @file actuator_manager.h
 * @brief Interfaz del módulo de control de actuadores.
 *        Gestiona el relé (bomba de riego) y el servomotor MG995 (control térmico).
 *        Expone funciones de alto nivel desacopladas de la lógica de control.
 *
 * @project  Sistema IoT - Cultivo de Hortalizas
 * @platform ESP32-WROOM-32
 * @version  1.0.0
 */

#pragma once

class String;

/**
 * @brief Inicializa el relé (OUTPUT, apagado por defecto) y el servo MG995.
 *        El servo se posiciona en SERVO_POS_CERRADO al iniciar.
 */
void initActuators();

/**
 * @brief Activa o desactiva la bomba de riego a través del relé.
 * @param encender true para encender la bomba, false para apagarla.
 */
void setPump(bool encender);

/**
 * @brief Mueve el servomotor MG995 a la posición angular indicada.
 * @param angulo Ángulo en grados (0–180°). Se limita al rango válido.
 */
void setServoPosition(int angulo);

/**
 * @brief Retorna el estado actual de la bomba de riego.
 * @return true si la bomba está encendida.
 */
bool getPumpState();

/**
 * @brief Retorna la posición angular actual del servomotor.
 * @return int Ángulo en grados (0–180).
 */
int getServoAngle();

/**
 * @brief Retorna el estado del servo como cadena de texto ("ABIERTO" / "CERRADO").
 * @return String Descripción del estado del servo.
 */
String getServoStateString();
