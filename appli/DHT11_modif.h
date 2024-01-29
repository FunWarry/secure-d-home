/*
 * DHT11.h
 *
 *  Created on: 21 déc. 2023
 *      Author: Alexandre GASCOIN
 */

#ifndef DHT11_MODIF_H_
#define DHT11_MODIF_H_

#include "stm32f1_uart.h"
#include "stm32f1_gpio.h"
#include "stm32f1_extit.h"

void init_DHT11(void);
char* read_DHT11(void);


#endif /* DHT11_MODIF_H_ */
