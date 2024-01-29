/*
 * MCP9701.h
 *
 *  Created on: 21 déc. 2023
 *      Author: Alexandre GASCOIN
 */

#ifndef MCP9701_H_
#define MCP9701_H_
#include "config.h"
#include <stdint.h>

void MCP_adc_process_1ms(void);

void init_MCP(void);

char* recup_data_MCP(void);

#endif /* MCP9701_H_ */
