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

#define TRUE	1
#define FALSE	0
#define bool	uint8_t

#define CYCLE_CLK     		128
#define CYCLE_MAX	    	84
#define TARGET_ADC			56

const uint8_t RAMP_DELAY    = 10;
const uint8_t RAMP_START    = 5;
const uint8_t RAMP_STEPS    = 128 / 16;


#endif
