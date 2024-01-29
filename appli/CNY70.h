/*
 * CNY70.h
 *
 *  Created on: 21 déc. 2023
 *      Author: Alexandre GASCOIN
 */

#ifndef CNY70_H_
#define CNY70_H_
#include "config.h"
#include <stdint.h>
#include "macro_types.h"

void init_CNY70(void);

uint8_t read_CNY70(void);

#endif /* CNY70_H_ */
