/*
 * hc-sr04.c
 *
 *  Created on: 8 janv. 2024
 *      Author: Alexandre GASCOIN
 *      Ce module est dédié au capteur à ultrasons, le HC-SR04.
 *      Le trigger déclenche un signal et le echo reçoit le signal. Il faut donc configurer une sortie et une entrée.
 */

#include "hc-sr04.h"
#include "HC-SR04/HCSR04.h"
#include "stm32f1xx_hal.h"
#include "stm32f1_timer.h"
#include "stm32f1_gpio.h"
#include "stm32f1_extit.h"
#include <stdio.h>


//Si La période de mesure est inférieure, elle ne sera respectée que si le capteur à présenté un écho moins long.
#define PERIOD_MEASURE			100

char reponse[50];

/**
 * @brief initialise le capteur à ultrasons HC-SR04
 * @pre avoir correctement branché le capteur
 * @post associe les voies echo et trigger aux pins correspondant.
 */
void init_HCSR04(void)
{
	static uint8_t id_sensor;

	if(HCSR04_add(&id_sensor, GPIOB, GPIO_PIN_7, GPIOB, GPIO_PIN_8) != HAL_OK)
	{
		printf("HCSR04 non ajouté - erreur gênante\n");
	}else{
		printf("HCSR04 ajouté\n");
	}
}

/**
 * @brief lit la donnée perçue par l'ECHO
 * @pre avoir initialisé le capteur
 * @return renvoie une chaîne de caractère en fonction de 3 états différents :
 * 1er cas : tout se passe bien, on reçoie la distance entre le capteur et le premier truc qui est devant
 * 2ème cas : une erreur s'est produite
 * 3ème cas : le programme boucle et donc on nous renvoie le timeout
 */
char* HCSR04_read(void)
{
	typedef enum
	{
		LAUNCH_MEASURE,
		RUN,
		WAIT_DURING_MEASURE,
		WAIT_BEFORE_NEXT_MEASURE
	}state_e;

	static state_e state = LAUNCH_MEASURE;
	static uint32_t tlocal;
	static uint8_t id_sensor;
	static uint8_t control;
	uint16_t distance;
	control = 0;
	while(!control)
	{
		//ne pas oublier d'appeler en tâche de fond cette fonction.
		HCSR04_process_main();
		switch(state)
		{
			case LAUNCH_MEASURE:
				HCSR04_run_measure(id_sensor);
				tlocal = HAL_GetTick();
				state = WAIT_DURING_MEASURE;
				break;
			case WAIT_DURING_MEASURE:
				switch(HCSR04_get_value(id_sensor, &distance))
				{
					case HAL_BUSY:
						//rien à faire... on attend...
						break;
					case HAL_OK:
						printf("sensor %d - distance : %d\n", id_sensor, distance);
						sprintf(reponse, "sensor %d - distance : %d\n", id_sensor, distance);
						state = WAIT_BEFORE_NEXT_MEASURE;
						control = 1;
						return reponse;
						break;
					case HAL_ERROR:
						printf("sensor %d - erreur ou mesure non lancee\n", id_sensor);
						sprintf(reponse, "sensor %d - erreur ou mesure non lancee\n", id_sensor);
						state = WAIT_BEFORE_NEXT_MEASURE;
						control = 1;
						return reponse;
						break;
					case HAL_TIMEOUT:
						printf("sensor %d - timeout\n", id_sensor);
						sprintf(reponse, "sensor %d - timeout\n", id_sensor);
						state = WAIT_BEFORE_NEXT_MEASURE;
						control = 1;
						return reponse;
						break;
				}
				break;
			case WAIT_BEFORE_NEXT_MEASURE:
				if(HAL_GetTick() > tlocal + PERIOD_MEASURE)
					state = LAUNCH_MEASURE;
				break;
			default:
				break;
		}
	}
	return NULL;
}
