/**
  ******************************************************************************
  * @file    main.c
  * @author  Nirgal
  * @date    03-July-2019
  * @brief   Default main function.
  ******************************************************************************
*/
#include "stm32f1xx_hal.h"
#include "stm32f1_uart.h"
#include "stm32f1_sys.h"
#include "stm32f1_gpio.h"
#include "macro_types.h"
#include "systick.h"
#include "dwt_delay.h"

#include "stm32f1_adc.h"
#include "cardio_simulator.h"

static volatile uint32_t t = 0;

void writeLED(bool_e b)
{
	HAL_GPIO_WritePin(LED_GREEN_GPIO, LED_GREEN_PIN, b);
}

bool_e readButton(void)
{
	return !HAL_GPIO_ReadPin(BLUE_BUTTON_GPIO, BLUE_BUTTON_PIN);
}


void process_ms(void)
{
	if(t)
		t--;
}

void my_strcpy(char * dest, char * src);

int additionner_7(int a, int b, int c, int d, int e, int f, int g);
int additionner_n(int n, int a, int b, ...);
int factorielle_n(int n);

void TD_mission_scratch(void);

#define TAB_SIZE 5000
static uint32_t idx = 0;
static uint8_t tab[TAB_SIZE];

void callback_adc(void)
{
	static bool_e flip = 0;
	int16_t v;

	flip = !flip;
	if(flip && idx < TAB_SIZE)
	{
		v = ADC_getValue(ADC_0);
		tab[idx] = (uint8_t)(v/16);
		idx++;
	}
}

static uint8_t last_value = 0;

void cardio_cb(uint8_t v)
{
	last_value = v;
}

int main(void)
{
	//Initialisation de la couche logicielle HAL (Hardware Abstraction Layer)
	//Cette ligne doit rester la première étape de la fonction main().
	HAL_Init();



	//Initialisation de l'UART2 à la vitesse de 115200 bauds/secondes (92kbits/s) PA2 : Tx  | PA3 : Rx.
		//Attention, les pins PA2 et PA3 ne sont pas reliées jusqu'au connecteur de la Nucleo.
		//Ces broches sont redirigées vers la sonde de débogage, la liaison UART étant ensuite encapsulée sur l'USB vers le PC de développement.
	UART_init(UART2_ID,115200);

	//"Indique que les printf sortent vers le périphérique UART2."
	SYS_set_std_usart(UART2_ID, UART2_ID, UART2_ID);

	//Initialisation du port de la led Verte (carte Nucleo)
	BSP_GPIO_PinCfg(LED_GREEN_GPIO, LED_GREEN_PIN, GPIO_MODE_OUTPUT_PP,GPIO_NOPULL,GPIO_SPEED_FREQ_HIGH);

	//Initialisation du port du bouton bleu (carte Nucleo)
	BSP_GPIO_PinCfg(BLUE_BUTTON_GPIO, BLUE_BUTTON_PIN, GPIO_MODE_INPUT,GPIO_PULLUP,GPIO_SPEED_FREQ_HIGH);

	//On ajoute la fonction process_ms à la liste des fonctions appelées automatiquement chaque ms par la routine d'interruption du périphérique SYSTICK
//	Systick_add_callback_function(&process_ms);

//	ADC_init();
	//ADC_set_callback_function(&callback_adc);

//	CARDIO_SIMULATOR_init();
//	CARDIO_SIMULATOR_set_callback(&cardio_cb);

//	DHT11_demo();
	#define TAB_SIZE	1024
	uint32_t tab[TAB_SIZE];
	uint32_t index = 0;
	uint32_t last, now;


	volatile char buf[20];
	sprintf(buf,"%s%XG%1X%C",(!42)?"OFF":"ON", 10<<3|5<<1, -2&0xF,'Q'+~((uint8_t)(-1)<<1));
	while(1)	//boucle de tâche de fond
	{
		process_main();
/*		now = SYSTICK_get_time_us();
		tab[index] = now-last;
		//printf("%ld\n",now-last);
		last = now;
		index++;
		if(index==TAB_SIZE)
		{
			index = 0;
			printf("#");
			for(uint32_t i = 0; i<TAB_SIZE; i++)
				if(tab[i]>2)
					printf("%ld\n",tab[i]);
		}*/
		/*
		if(!t)
		{
			t = 200;
			HAL_GPIO_TogglePin(LED_GREEN_GPIO, LED_GREEN_PIN);
		}*/
		//uint8_t last = 0;

	/*	if(idx == TAB_SIZE)
		{
			debug_printf("#\n");
			for(uint32_t i = 0; i<idx; i++)
			{
				//if(tab[i] != last)
				//{
				//	last = tab[i];
					debug_printf("%d\n", tab[i]);
				//}
			}

			idx = 0;
		}

*/
		/*if(last_value)
		{
			uint8_t local_value;
			local_value = last_value;
			last_value = 0;
			printf("%d\n", local_value);

		}*/
	/*	static CARDIO_SIMULATOR_mode_e mode = CARDIO_SIMULATOR_MODE_0;
		static bool_e previous = FALSE;
		bool_e button;
		button = readButton();
		if(!previous && button)
		{
			mode = (mode+1)%CARDIO_SIMULATOR_MODE_NUMBER;
			CARDIO_SIMULATOR_set_mode(mode);
		}*/
	}
}



#include <stdarg.h>

#ifndef BUFFER_PRINTF_IT_SIZE
	#define BUFFER_PRINTF_IT_SIZE	((int32_t)(256))
#endif
static char buffer_printf_it[BUFFER_PRINTF_IT_SIZE];
static volatile int32_t index_write;




static void vprintf_in_it(const char * format, va_list arg_ptr)
{
	//Il reste BUFFER_PRINTF_IT_SIZE - index_write octets dispos dans le buffer.
	if(index_write < BUFFER_PRINTF_IT_SIZE)
		index_write += vsnprintf(&buffer_printf_it[index_write], (size_t)(BUFFER_PRINTF_IT_SIZE-index_write), format, arg_ptr);
	//vsnprintf renvoie le nombre de caractères même si elle n'en a copié qu'une partie dans le buffer à taille limitée.
	if(index_write >= BUFFER_PRINTF_IT_SIZE)
		index_write = BUFFER_PRINTF_IT_SIZE;	//Ecretage.
}

void printf_in_it(const char * format, ...)
{
	va_list args_list;
	va_start(args_list, format);
	vprintf_in_it(format, args_list);
	va_end(args_list);
}

//Cette fonction à pour but de consommer le buffer qui aurait été rempli en IT.
void process_main(void)
{
	char buffer_printf_main[BUFFER_PRINTF_IT_SIZE+1];	//En stack !
	int32_t i;
	bool_e bloop;
	bloop = (index_write!=0)?TRUE:FALSE;
	i=0;
	while(bloop)
	{
		buffer_printf_main[i] = buffer_printf_it[i];
		i++;
		__disable_irq();	//Section critique.
		if(i>=index_write)	//On est enfin arrivé au bout des données à afficher
		{
			bloop = FALSE;
			index_write = 0;
			buffer_printf_main[i] = '\0';
		}
		__enable_irq();
	}
	if(i)
		printf("%s",buffer_printf_main);	//on fait sortir la chaine "pour de vrai" !
}
