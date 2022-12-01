#include "config.h"
#include "macro_types.h"
#include "stm32f1xx.h"
#include "stm32f1_timer.h"
#include "stm32f1_stepper_motor.h"


/*
Comment utiliser ce module logiciel ? (qui est encore en phase de développement... il faut donc bien le comprendre pour s'en servir !)
1- définir le nombre de moteurs dans le fichier header, en adaptant l'enumeration motor_id_e.
2- adapter ci-dessous les tableaux enable_pins, dir_pins, pulse_pins qui indiquent les broches de pilotage des moteurs pas à pas.

*/

#if USE_STEPPER_MOTOR

#ifndef GPIO_STEPPER_MOTOR
	#define GPIO_STEPPER_MOTOR			GPIOB
#endif

#ifndef STEPPER_MOTOR_TIMER
	#define STEPPER_MOTOR_TIMER					TIMER2_ID
	#define STEPPER_MOTOR_timer_irq_handler		TIMER2_user_handler_it
#endif

#define DEFAULT_IT_PERIOD 100			//période de l'interruption qui génère les pulses.

											//	 X   Y
static const uint8_t enable_pins[MOTOR_NB] = 	{11, 10};	//définir ici les numéros de broches des enables des moteurs  (sur le GPIO unique GPIO_STEPPER_MOTOR)
static const uint8_t dir_pins[MOTOR_NB] = 		{14, 12};	//ici les broches des directions 
static const uint8_t pulse_pins[MOTOR_NB] = 	{15, 13};	//ici les broches des pulse

volatile static int32_t positions[MOTOR_NB] = {0};
volatile static int32_t goals[MOTOR_NB] = {0};
volatile static int32_t pulse_period[MOTOR_NB] = {10, 10};	//"vitesse" par défaut (période par défaut entre deux pulses)

static void STEPPER_MOTOR_pin_set(uint32_t pin, bool_e b);

void STEPPER_MOTORS_init(void)
{
	for(motor_id_e m=0; m<MOTOR_NB; m++)
	{
		//Configurer en sortie OD les sorties
		BSP_GPIO_PinCfg(GPIO_STEPPER_MOTOR, 1<<enable_pins[m], GPIO_MODE_OUTPUT_OD,GPIO_NOPULL,GPIO_SPEED_FREQ_HIGH);
		BSP_GPIO_PinCfg(GPIO_STEPPER_MOTOR, 1<<dir_pins[m], GPIO_MODE_OUTPUT_OD,GPIO_NOPULL,GPIO_SPEED_FREQ_HIGH);
		BSP_GPIO_PinCfg(GPIO_STEPPER_MOTOR, 1<<pulse_pins[m], GPIO_MODE_OUTPUT_OD,GPIO_NOPULL,GPIO_SPEED_FREQ_HIGH);

		//Appliquer 1 sur toutes les sorties
		STEPPER_MOTOR_pin_set(enable_pins[m], 1);
		STEPPER_MOTOR_pin_set(dir_pins[m], 1);
		STEPPER_MOTOR_pin_set(pulse_pins[m], 1);
	
		STEPPER_MOTOR_enable(m, TRUE);
	}
	
	TIMER_run_us(STEPPER_MOTOR_TIMER, DEFAULT_IT_PERIOD, TRUE);
}



void STEPPER_MOTOR_enable(motor_id_e id, bool_e enable)
{
	STEPPER_MOTOR_pin_set(enable_pins[id], enable);
}


void STEPPER_MOTORS_do_pulse(motor_id_e id)
{
	STEPPER_MOTOR_pin_set(pulse_pins[id], 0);
	Delay_us(30);
	STEPPER_MOTOR_pin_set(pulse_pins[id], 1);
}

void STEPPER_MOTORS_set_dir(motor_id_e id, bool_e direction)
{
	STEPPER_MOTOR_pin_set(dir_pins[id], direction);
}


static void STEPPER_MOTOR_pin_set(uint32_t pin, bool_e b)
{
	if(pin<8)
	{
		if(b)
			GPIO_STEPPER_MOTOR->CRL &= (uint32_t)(~(3<<(4*(pin))));
		else
			GPIO_STEPPER_MOTOR->CRL |= (3<<(4*(pin)));
	}
	else
	{
		if(b)
			GPIO_STEPPER_MOTOR->CRH &= (uint32_t)(~(3<<(4*(pin - 8))));
		else
			GPIO_STEPPER_MOTOR->CRH |= (3<<(4*(pin - 8)));
	}
}

void STEPPER_MOTOR_set_goal(motor_id_e id, int32_t newgoal)
{
	if(id<MOTOR_NB)
		goals[id] = newgoal;

}

int32_t STEPPER_MOTOR_get_positionx (motor_id_e id){
	return positions[id];
}

bool_e STEPPER_MOTOR_is_arrived (motor_id_e id){
	return (goals[id]==positions[id])?TRUE:FALSE;
}

int32_t STEPPER_MOTOR_get_goal (motor_id_e id){
	return goals[id];
}

void STEPPER_MOTOR_timer_irq_handler (){
	static uint32_t periods[MOTOR_NB] = {0};

	for(motor_id_e m=0; m<MOTOR_NB; m++)
	{
		periods[m] = (periods[m]+1)%pulse_period[m];
		if (periods[m] == 0)
		{
			if (positions[m] < goals[m])
			{
					positions[m]++;
					STEPPER_MOTORS_set_dir(m, FALSE);
					STEPPER_MOTORS_do_pulse(m);
			}
			else if (positions[m] > goals[m])
			{
					positions[m]--;
					STEPPER_MOTORS_set_dir(m, TRUE);
					STEPPER_MOTORS_do_pulse(m);
			}
		}
	}
}


#endif
