/*
 * hc-sr04.h
 *
 *  Created on: 8 janv. 2024
 *      Author: ESEO
 */

#ifndef HC_SR04_H_
#define HC_SR04_H_

#include "macro_types.h"

void init_HCSR04(void);
char* HCSR04_read(void);

#endif /* HC_SR04_H_ */
