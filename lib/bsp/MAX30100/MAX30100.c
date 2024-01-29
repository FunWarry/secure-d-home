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
#if USE_MAX30100

#include "MAX30100.h"
#include "macro_types.h"

typedef struct
{
	uint16_t ir;
	uint16_t red;
}sensor_data_t;

typedef struct
{
	sensor_data_t buffer[RINGBUFFER_SIZE];
	uint8_t read_index;
	uint8_t write_index;
	uint8_t full_nb;
}circular_buffer_t;

static circular_buffer_t circular_buffer = {0};




    uint8_t readRegister(uint8_t address);
    void writeRegister(uint8_t address, uint8_t data);
    void burstRead(uint8_t baseAddress, uint8_t *buffer, uint8_t length);
    void readFifoData();

bool_e MAX30100_begin()
{
	I2C_Init(MAX3100_I2C, 100000, TRUE);

	uint8_t part_id;
	part_id = MAX30100_getPartId();
    if (part_id != EXPECTED_PART_ID && part_id != EXPECTED_PART_ID_30102) {
        return FALSE;
    }
    printf("found MAX3010%d", (part_id==EXPECTED_PART_ID)?0:2);

    MAX30100_setMode(DEFAULT_MODE);
    MAX30100_setLedsPulseWidth(DEFAULT_PULSE_WIDTH);
    MAX30100_setSamplingRate(DEFAULT_SAMPLING_RATE);
    MAX30100_setLedsCurrent(DEFAULT_IR_LED_CURRENT, DEFAULT_RED_LED_CURRENT);
    MAX30100_setHighresModeEnabled(TRUE);

    return TRUE;
}

void MAX30100_setMode(Mode mode)
{
    writeRegister(MAX30100_REG_MODE_CONFIGURATION, mode);
}

void MAX30100_setLedsPulseWidth(LEDPulseWidth ledPulseWidth)
{
    uint8_t previous = readRegister(MAX30100_REG_SPO2_CONFIGURATION);
    writeRegister(MAX30100_REG_SPO2_CONFIGURATION, (previous & 0xfc) | ledPulseWidth);
}

void MAX30100_setSamplingRate(SamplingRate samplingRate)
{
    uint8_t previous = readRegister(MAX30100_REG_SPO2_CONFIGURATION);
    writeRegister(MAX30100_REG_SPO2_CONFIGURATION, (previous & 0xe3) | (samplingRate << 2));
}

void MAX30100_setLedsCurrent(LEDCurrent irLedCurrent, LEDCurrent redLedCurrent)
{
    writeRegister(MAX30100_REG_LED_CONFIGURATION, redLedCurrent << 4 | irLedCurrent);
}

void MAX30100_setHighresModeEnabled(bool_e enabled)
{
    uint8_t previous = readRegister(MAX30100_REG_SPO2_CONFIGURATION);
    if (enabled) {
        writeRegister(MAX30100_REG_SPO2_CONFIGURATION, previous | MAX30100_SPC_SPO2_HI_RES_EN);
    } else {
        writeRegister(MAX30100_REG_SPO2_CONFIGURATION, previous & ~MAX30100_SPC_SPO2_HI_RES_EN);
    }
}

void MAX30100_update()
{
    readFifoData();
}

bool_e MAX30100_getRawValues(uint16_t *ir, uint16_t *red)
{
	bool_e ret;
	if(circular_buffer.full_nb > 0)
	{
		*ir = circular_buffer.buffer[circular_buffer.read_index].ir;
		*red = circular_buffer.buffer[circular_buffer.read_index].red;
		circular_buffer.read_index = (circular_buffer.read_index +1)%RINGBUFFER_SIZE;
		circular_buffer.full_nb--;
		ret = TRUE;
	}

	return ret;
}

void MAX30100_resetFifo()
{
    writeRegister(MAX30100_REG_FIFO_WRITE_POINTER, 0);
    writeRegister(MAX30100_REG_FIFO_READ_POINTER, 0);
    writeRegister(MAX30100_REG_FIFO_OVERFLOW_COUNTER, 0);
}

uint8_t readRegister(uint8_t address)
{
	uint8_t read;
	I2C_Read(MAX3100_I2C, MAX30100_I2C_ADDRESS, address, &read);
    return read;
}

void writeRegister(uint8_t address, uint8_t data)
{
	I2C_Write(MAX3100_I2C, MAX30100_I2C_ADDRESS, address, data);
}

void burstRead(uint8_t baseAddress, uint8_t *buffer, uint8_t length)
{
	I2C_ReadMulti(MAX3100_I2C, MAX30100_I2C_ADDRESS, baseAddress, buffer, length);
}

void readFifoData()
{
    uint8_t buffer[MAX30100_FIFO_DEPTH*4];
    uint8_t toRead;

    toRead = (readRegister(MAX30100_REG_FIFO_WRITE_POINTER) - readRegister(MAX30100_REG_FIFO_READ_POINTER)) & (MAX30100_FIFO_DEPTH-1);

    if (toRead) {
        burstRead(MAX30100_REG_FIFO_DATA, buffer, 4 * toRead);

        for (uint8_t i=0 ; i < toRead ; ++i) {
            // Warning: the values are always left-aligned
        	if(circular_buffer.full_nb < RINGBUFFER_SIZE)
        	{
        		circular_buffer.buffer[circular_buffer.write_index].ir = (uint16_t)((buffer[i*4] << 8) | buffer[i*4 + 1]) ;
        		circular_buffer.buffer[circular_buffer.write_index].red = (uint16_t)((buffer[i*4 + 2] << 8) | buffer[i*4 + 3]);
        		circular_buffer.write_index = (circular_buffer.write_index +1)%RINGBUFFER_SIZE;
        		circular_buffer.full_nb++;
        	}
        }
    }
}

void MAX30100_startTemperatureSampling()
{
    uint8_t modeConfig = readRegister(MAX30100_REG_MODE_CONFIGURATION);
    modeConfig |= MAX30100_MC_TEMP_EN;

    writeRegister(MAX30100_REG_MODE_CONFIGURATION, modeConfig);
}

bool_e MAX30100_isTemperatureReady()
{
    return !(readRegister(MAX30100_REG_MODE_CONFIGURATION) & MAX30100_MC_TEMP_EN);
}

float MAX30100_retrieveTemperature()
{
    int8_t tempInteger = readRegister(MAX30100_REG_TEMPERATURE_DATA_INT);
    float tempFrac = readRegister(MAX30100_REG_TEMPERATURE_DATA_FRAC);

    return tempFrac * (float)0.0625 + tempInteger;
}

void MAX30100_shutdown()
{
    uint8_t modeConfig = readRegister(MAX30100_REG_MODE_CONFIGURATION);
    modeConfig |= MAX30100_MC_SHDN;

    writeRegister(MAX30100_REG_MODE_CONFIGURATION, modeConfig);
}

void MAX30100_resume()
{
    uint8_t modeConfig = readRegister(MAX30100_REG_MODE_CONFIGURATION);
    modeConfig &= ~MAX30100_MC_SHDN;

    writeRegister(MAX30100_REG_MODE_CONFIGURATION, modeConfig);
}

uint8_t MAX30100_getPartId()
{
    return readRegister(0xff);
}
#endif
