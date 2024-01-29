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

#include "MAX30100_PulseOximeter.h"
#include "MAX30100.h"
#include "macro_types.h"

	void PulseOximeter_checkSample();
	void PulseOximeter_checkCurrentBias();

	static PulseOximeterState state = PULSEOXIMETER_STATE_INIT;
	static PulseOximeterDebuggingMode debuggingMode;
	static uint32_t tsFirstBeatDetected = 0;
	static uint32_t tsLastBeatDetected = 0;
	static uint32_t tsLastBiasCheck = 0;
	static uint32_t tsLastCurrentAdjustment;

	static uint8_t redLedCurrentIndex = (uint8_t)RED_LED_CURRENT_START;
	static LEDCurrent irLedCurrent = DEFAULT_IR_LED_CURRENT;

	static callback_fun_t onBeatDetected = NULL;


typedef struct
{
	float alpha;
	float dcw;
}dcr_t;

static dcr_t ir_dcr = (dcr_t){.alpha=DC_REMOVER_ALPHA, .dcw=0};
static dcr_t red_dcr = (dcr_t){.alpha=DC_REMOVER_ALPHA, .dcw=0};


float dcr_step(dcr_t * dcr, float x)
{
	float olddcw = dcr->dcw;
	dcr->dcw = (float)x + dcr->alpha * dcr->dcw;

	return dcr->dcw - olddcw;
}

float getDCW(dcr_t * dcr)
{
	return dcr->dcw;
}



bool_e PulseOximeter_begin(PulseOximeterDebuggingMode debuggingMode_)
{
    debuggingMode = debuggingMode_;

    bool_e ready = MAX30100_begin();

    if (!ready) {
        if (debuggingMode != PULSEOXIMETER_DEBUGGINGMODE_NONE) {
            printf("Failed to initialize the HRM sensor\n");
        }
        return FALSE;
    }

    MAX30100_setMode(MAX30100_MODE_SPO2_HR);
    MAX30100_setLedsCurrent(irLedCurrent, (LEDCurrent)redLedCurrentIndex);


    state = PULSEOXIMETER_STATE_IDLE;

    return TRUE;
}







void PulseOximeter_update()
{
	MAX30100_update();

	PulseOximeter_checkSample();
	PulseOximeter_checkCurrentBias();
}

float PulseOximeter_getHeartRate()
{
    return BEATDETECTOR_getRate();
}

uint8_t PulseOximeter_getSpO2()
{
    return SP02CALCULATOR_getSpO2();
}

uint8_t PulseOximeter_getRedLedCurrentBias()
{
    return redLedCurrentIndex;
}

void PulseOximeter_setOnBeatDetectedCallback(callback_fun_t cb)
{
    onBeatDetected = cb;
}

void PulseOximeter_setIRLedCurrent(LEDCurrent irLedNewCurrent)
{
    irLedCurrent = irLedNewCurrent;
    MAX30100_setLedsCurrent(irLedCurrent, (LEDCurrent)redLedCurrentIndex);
}

void PulseOximeter_shutdown()
{
	MAX30100_shutdown();
}

void PulseOximeter_resume()
{
	MAX30100_resume();
}

void PulseOximeter_checkSample()
{
    uint16_t rawIRValue, rawRedValue;

    // Dequeue all available samples, they're properly timed by the HRM
    while (MAX30100_getRawValues(&rawIRValue, &rawRedValue)) {
        float irACValue = dcr_step(&ir_dcr, rawIRValue);
        float redACValue = dcr_step(&red_dcr, rawRedValue);

        // The signal fed to the beat detector is mirrored since the cleanest monotonic spike is below zero
        float filteredPulseValue = Filters_step(-irACValue);
        bool_e beatDetected = BEATDETECTOR_addSample(filteredPulseValue);

        if (BEATDETECTOR_getRate() > 0) {
            state = PULSEOXIMETER_STATE_DETECTING;
            SP02CALCULATOR_update(irACValue, redACValue, beatDetected);
        } else if (state == PULSEOXIMETER_STATE_DETECTING) {
            state = PULSEOXIMETER_STATE_IDLE;
            SP02CALCULATOR_reset();
        }

        switch (debuggingMode) {
            case PULSEOXIMETER_DEBUGGINGMODE_RAW_VALUES:
                printf("R:%d,%d\n",rawIRValue, rawRedValue);
                break;

            case PULSEOXIMETER_DEBUGGINGMODE_AC_VALUES:
                printf("R:%f,%f\n",irACValue, redACValue);
                break;

            case PULSEOXIMETER_DEBUGGINGMODE_PULSEDETECT:
                printf("R:%lf,%lf\n",filteredPulseValue, BEATDETECTOR_getCurrentThreshold());
                break;

            default:
                break;
        }

        if (beatDetected && onBeatDetected) {
            onBeatDetected();
        }
    }
}

void PulseOximeter_checkCurrentBias()
{
    // Follower that adjusts the red led current in order to have comparable DC baselines between
    // red and IR leds. The numbers are really magic: the less possible to avoid oscillations
    if (millis() - tsLastBiasCheck > CURRENT_ADJUSTMENT_PERIOD_MS) {
        bool_e changed = FALSE;
        if (ir_dcr.dcw - red_dcr.dcw > 70000 && redLedCurrentIndex < MAX30100_LED_CURR_50MA) {
            ++redLedCurrentIndex;
            changed = TRUE;
        } else if (red_dcr.dcw - ir_dcr.dcw > 70000 && redLedCurrentIndex > 0) {
            --redLedCurrentIndex;
            changed = TRUE;
        }

        if (changed) {
        	MAX30100_setLedsCurrent(irLedCurrent, (LEDCurrent)redLedCurrentIndex);
            tsLastCurrentAdjustment = millis();

            if (debuggingMode != PULSEOXIMETER_DEBUGGINGMODE_NONE) {
                printf("I:");
                printf("%d\n",redLedCurrentIndex);
            }
        }

        tsLastBiasCheck = millis();
    }
}
