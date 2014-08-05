#ifndef _SMPS_H_
#define _SMPS_H_



#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <avr/wdt.h>



// -------------------------------------------------------------------------------------
// Pins

#define DIAG_LED	PB3
#define PWM_OUTPUT	PB1
#define ADC_INPUT	PB4

const uint8_t CYCLE_CLK     = 250;
const uint8_t CYCLE_MAX	    = (CYCLE_CLK * 0.70)
const uint8_t TARGET_ADC	= 74;

const uint8_t RAMP_DELAY    = 10;
const uint8_t RAMP_START    = 5;
const uint8_t RAMP_STEPS    = CYCLE_CLK / RAMP_DELAY;


#endif
