/*
 * photoresistance.h
 *
 *  Created on: 21 déc. 2023
 *      Author: Alexandre GASCOIN
 */

#ifndef PHOTORESISTANCE_H_
#define PHOTORESISTANCE_H_

#include "config.h"
#include <stdint.h>

void light_adc_process_1ms(void);

void init_light(void);

char* get_data_light(void);

#endif /* PHOTORESISTANCE_H_ */
