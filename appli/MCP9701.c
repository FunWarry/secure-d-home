/*
 * MCP9701.c
 *
 *  Created on: 21 déc. 2023
 *      Author: Alexandre GASCOIN
 *		Ce module est dédié au capteur de température.
 *		Le capteur renvoie la température ambiante.
 */

#include "MCP9701.h"
#include "stm32f1_adc.h"
#include "systick.h"

static volatile uint16_t t = 0;
static int8_t adc_id[ADC_CHANNEL_NB];
char reponse3 [50];

/**
 * @brief reproduit le temps avec une unité en millisecondes
 * @pre avoir déclaré la variable t en volatile
 * @post décrémente la varible t pour fabriquer un timer
 */
void MCP_adc_process_1ms(void)
{
	if(t)
		t--;
}

/**
 * @brief appelle la fonction MCP_adc_process_1ms en continue
 * @post ajoute la fonction MCP_adc_process_1ms à la routine d'interruption
 */
void init_MCP(void){
	Systick_add_callback_function(MCP_adc_process_1ms);
}

/**
 * @brief lit la température mesurée par le capteur
 * @pre avoir initialisé l'ADC (methode init_Light)
 * @pre avoir lancé la fonction init_MCP pour le timer
 * @return renvoie la température ambiante en chaîne de caractère et en degré Celsius
 */
char* recup_data_MCP(void){
	int16_t value;
	int16_t millivolt;
	if(!t){
		t = 400;
		if(adc_id[ADC_0] != -1){
			value = ADC_getValue(ADC_0);
			millivolt = (int16_t)((((int32_t)value)*3300)/4096);		//On la convertie en volts
			millivolt /= 33;
			printf("Il fait une temperature de : %1d°C", millivolt);
			sprintf(reponse3, "Il fait une temperature de : %1d°C", millivolt);
			return reponse3;
		}
		printf("\n");
	}
	return NULL;
}
