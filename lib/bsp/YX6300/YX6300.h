#ifndef YX6300_H
#define YX6300_H

#include "config.h"



void YX6300_demo(void)

void YX6300_send_request_with_2bytes_of_datas(uint8_t command, bool_e feedback, uint16_t data16);


void YX6300_send_request(uint8_t command, bool_e feedback, uint8_t datasize, uint8_t * data);


#endif //YX6300_H