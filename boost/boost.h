#ifndef _SMPS_H_
#define _SMPS_H_



#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

#include <builtin/adcutils.h>


// Boost has in inverse relationship to duty cycle (shorter cycle = higher voltage, longer cycle = lower voltage)
#define	PWM_SIGN_BOOST			-1

// Buck has a direct relationship to duty cycle (shorter cycle = lower voltage, longer cycle = higher voltage)
#define PWM_SIGN_BUCK			1


const int8_t		DUTY_CYCLE_ADJUST		= PWM_SIGN_BOOST;


// -------------------------------------------------------------------------------------
// Pins

#define DIAG_LED	PB2
#define PWM_OUTPUT	PB4
#define ADC_INPUT	PB3

#define TRUE	1
#define FALSE	0
#define bool	uint8_t

#define CYCLE_CLK     		256
#define CYCLE_MAX	    	84
#define TARGET_ADC			56

const uint8_t RAMP_DELAY    = 10;
const uint8_t RAMP_START    = 5;
const uint8_t RAMP_STEPS    = 128 / 16;

const float		V_TARGET	= 12.0;



void pwmOff(void);
void pwmOn(void);
void processADC(void);


#endif

/*
Smaller toroid inductor
CS13:0	OCR1C	Freq	ObsF	Voltage		Duty Cycle
0011	110		150KHz			8.7			50%
0011	150		110KHz			8.6			50%
0010	245		130KHz			8.6			50%
0010	177		170KHz			8.7			50%
0010	160		200KHz			8.8			50%
0001	255		250KHz			8.8			50%
0001	212		300KHz			8.8			50%
0001	182		350KHz			8.9			50%
0001	160		400KHz			8.9			50%
0001	128		500KHz	878KHz	9.0			50%
*/