#ifndef _SMPS_H_
#define _SMPS_H_



#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <avr/wdt.h>


// -------------------------------------------------------------------------------------
// Pins

#define DIAG_LED	PB2
#define PWM_OUTPUT	PB4
#define COMP_INPUT	PB3

#define TRUE		1
#define FALSE		0
#define bool		uint8_t


// -------------------------------------------------------------------------------------
// Constants
#define	VOLTAGE_IN				5
#define VOLTAGE_OUT				3.3

#define DUTY_CYCLE_CLK			160
#define DUTY_CYCLE				(VOLTAGE_OUT/VOLTAGE_IN)
const uint8_t DUTY_CYCLE_PWM	= (uint8_t)(DUTY_CYCLE_CLK * DUTY_CYCLE);


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