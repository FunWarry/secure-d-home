/*
 * stm32f1_stepper_motor.h
 *
 *  Created on: 29 nov. 2022
 *      Author: julia
 */

#ifndef BSP_STM32F1_STEPPER_MOTOR_H_
#define BSP_STM32F1_STEPPER_MOTOR_H_

// définir ici le nombre de moteurs, en adaptant l'enumeration motor_id_e.... et en laissant MOTOR_NB à la fin de la liste des moteurs.
typedef enum
{
	MOTOR_X,
	MOTOR_Y,	//commenter cette ligne si vous utilisez un seul moteur.
	MOTOR_NB
}motor_id_e;

void STEPPER_MOTORS_init(void);

void STEPPER_MOTOR_enable(motor_id_e id, bool_e enable);
void STEPPER_MOTOR_set_goal(motor_id_e id, int32_t newgoal);
int32_t STEPPER_MOTOR_get_position(motor_id_e id);
bool_e STEPPER_MOTOR_is_arrived (motor_id_e id);
int32_t STEPPER_MOTOR_get_goal (motor_id_e id);

#endif /* BSP_STM32F1_STEPPER_MOTOR_H_ */
