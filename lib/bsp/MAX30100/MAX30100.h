/*
Arduino-MAX30100 oximetry / heart rate integrated sensor library
Copyright (C) 2016  OXullo Intersecans <x@brainrapers.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MAX30100_H
#define MAX30100_H

#include <stdint.h>

#include "MAX30100_Registers.h"

#define DEFAULT_MODE                MAX30100_MODE_HRONLY
#define DEFAULT_SAMPLING_RATE       MAX30100_SAMPRATE_100HZ
#define DEFAULT_PULSE_WIDTH         MAX30100_SPC_PW_1600US_16BITS
#define DEFAULT_RED_LED_CURRENT     MAX30100_LED_CURR_50MA
#define DEFAULT_IR_LED_CURRENT      MAX30100_LED_CURR_50MA
#define EXPECTED_PART_ID            0x11
#define EXPECTED_PART_ID_30102       0x15
#define RINGBUFFER_SIZE             16

#define I2C_BUS_SPEED               400000UL

typedef struct {
    uint16_t ir;
    uint16_t red;
} SensorReadout;


#include "stm32f1_i2c.h"

#define MAX3100_I2C		I2C1

    bool_e MAX30100_begin();
    void MAX30100_setMode(Mode mode);
    void MAX30100_setLedsPulseWidth(LEDPulseWidth ledPulseWidth);
    void MAX30100_setSamplingRate(SamplingRate samplingRate);
    void MAX30100_setLedsCurrent(LEDCurrent irLedCurrent, LEDCurrent redLedCurrent);
    void MAX30100_setHighresModeEnabled(bool_e enabled);
    void MAX30100_update();
    bool_e MAX30100_getRawValues(uint16_t *ir, uint16_t *red);
    void MAX30100_resetFifo();
    void MAX30100_startTemperatureSampling();
    bool_e MAX30100_isTemperatureReady();
    float MAX30100_retrieveTemperature();
    void MAX30100_shutdown();
    void MAX30100_resume();
    uint8_t MAX30100_getPartId();



#endif
