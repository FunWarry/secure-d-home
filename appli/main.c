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

/*

#include "MQTTClient.h"
#include "wizchip_conf.h"

void writeLED(bool_e b)
{
	HAL_GPIO_WritePin(LED_GREEN_GPIO, LED_GREEN_PIN, b);
}

bool_e readButton(void)
{
	return !HAL_GPIO_ReadPin(BLUE_BUTTON_GPIO, BLUE_BUTTON_PIN);
}
*/

static volatile uint32_t t = 0;
void process_ms(void)
{
	if(t)
		t--;
}
/*
wiz_NetInfo netInfo = { .mac 	= {0xa8, 0x61, 0x0A, 0xAE, 0x89, 0x43}};					// Gateway address
r = "eseodp.cloud.shiftr.io";
char* port = "1883";
char* topic = "maison";


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
	Systick_add_callback_function(&process_ms);





	while(1)	//boucle de tâche de fond
	{

	}
}

*/


//Include: W5500 iolibrary
#include "w5100.h"
#include "wizchip_conf.h"
#include "time.h"


//Include: Internet iolibrary
#include "MQTTClient.h"

//Include: Standard IO Library
#include <stdio.h>

//Socket number defines
#define TCP_SOCKET	0

//Receive Buffer Size define
#define BUFFER_SIZE	2048

//Global variables
unsigned char targetIP[4] = {34,77,13,55}; // mqtt server IP
unsigned int targetPort = 1883; // mqtt server port
uint8_t mac_address[6] = {0xa8, 0x61, 0x0A, 0xAE, 0x89, 0x43};
wiz_NetInfo gWIZNETINFO = { .mac = {0xa8, 0x61, 0x0A, 0xAE, 0x89, 0x43}, //user MAC
							.ip = {}, //user IP
							.sn = {},
							.gw = {},
							.dns = {},
							.dhcp = NETINFO_STATIC};

unsigned char tempBuffer[BUFFER_SIZE] = {};

struct opts_struct
{
	char* clientid;
	int nodelimiter;
	char* delimiter;
	enum QoS qos;
	char* username;
	char* password;
	char* host;
	int port;
	int showtopics;
} opts;

opts_struct opts ={"Carte", 0, "\n", QOS0, "eseodp", "eseoproj1", targetIP, targetPort, 1 };
// @brief messageArrived callback function
void messageArrived(MessageData* md)
{
	unsigned char testbuffer[100];
	MQTTMessage* message = md->message;

	if (opts.showtopics)
	{
		memcpy(testbuffer,(char*)message->payload,(int)message->payloadlen);
		*(testbuffer + (int)message->payloadlen + 1) = "\n";
		printf("%s\r\n",testbuffer);
	}

	if (opts.nodelimiter)
		printf("%.*s", (int)message->payloadlen, (char*)message->payload);
	else
		printf("%.*s%s", (int)message->payloadlen, (char*)message->payload, opts.delimiter);
}


// @brief 1 millisecond Tick Timer setting
void NVIC_configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
	SysTick_Config(72000);
	NVIC_InitStructure.NVIC_IRQChannel = SysTick_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // Highest priority
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

// @brief 1 millisecond Tick Timer Handler setting
void SysTick_Handler(void)
{
	MilliTimer_Handler();
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
		Systick_add_callback_function(&process_ms);



	int i;
	int rc = 0;
	unsigned char buf[100];
	//Usart initialization for Debug.
	USART2Initialze();
		printf("USART initialized.\n\r");

	I2C1Initialize();
		printf("I2C initialized.\n\r");

	MACEEP_Read(mac_address,0xfa,6);

	printf("Mac address\n\r");
	for(i = 0 ; i < 6 ; i++)
	{
		printf("%02x ",mac_address[i]);
	}
	printf("\n\r");


	//W5500 initialization.
	W5100HardwareInitilize();
		printf("W5100 hardware interface initialized.\n\r");

	W5100Initialze();
		printf("W5100 IC initialized.\n\r");

	//Set network informations
	wizchip_setnetinfo(&gWIZNETINFO);

	setSHAR(mac_address);

	print_network_information();

	Network n;
	MQTTClient c;

	NewNetwork(&n, TCP_SOCKET);
	ConnectNetwork(&n, targetIP, targetPort);
	MQTTClientInit(&c,&n,1000,buf,100,tempBuffer,2048);

	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	data.willFlag = 0;
	data.MQTTVersion = 3;
	data.clientID.cstring = opts.clientid;
	data.username.cstring = opts.username;
	data.password.cstring = opts.password;

	data.keepAliveInterval = 60;
	data.cleansession = 1;

	rc = MQTTConnect(&c, &data);
	printf("Connected %d\r\n", rc);
	opts.showtopics = 1;

	printf("Subscribing to %s\r\n", "hello/wiznet");
	rc = MQTTSubscribe(&c, "hello/wiznet", opts.qos, messageArrived);
	printf("Subscribed %d\r\n", rc);

    while(1)
    {
    	MQTTYield(&c, data.keepAliveInterval);
    }
}
