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

// Gateway address
r = "eseodp.cloud.shiftr.io";
char* topic = "maison";



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
uint8_t mac_address[6] = {0xA8, 0x61, 0x0A, 0xAE, 0x89, 0x43};
wiz_NetInfo gWIZNETINFO = { .mac = {0xA8, 0x61, 0x0A, 0xAE, 0x89, 0x43}, //user MAC
							.ip = {172,14,3,1}, //user IP
							.sn = {},
							.gw = {},
							.dns = {},
							.dhcp = NETINFO_STATIC};

unsigned char tempBuffer[BUFFER_SIZE] = {};

// @brief Options structure for MQTTClient
struct opts_struct
{
    const char* clientid;
    int nodelimiter;
    const char* delimiter;
    enum QoS qos;
    const char* username;
    const char* password;
    const char* host;
    int port;
    int showtopics;
};

struct opts_struct opts = {
    .clientid = "eseodp",
    .nodelimiter = 0,
    .delimiter = "\n",
    .qos = QOS0,
    .username = "eseodp",
    .password = "eseoproj1",
    .host = targetIP,
    .port = &targetPort,
    .showtopics = 1
};

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
	Systick_add_callback_function(&MilliTimer_Handler);



	int i;
	int rc = 0;
	unsigned char buf[100];
	//Usart initialization for Debug.



	printf("Mac address\n\r");
	for(i = 0 ; i < 6 ; i++)
	{
		printf("%02x ",mac_address[i]);
	}
	printf("\n\r");


	//Set network informations
	wizchip_setnetinfo(&gWIZNETINFO);

	setSHAR(mac_address);

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
