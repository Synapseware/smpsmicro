#ifndef _SMPS_H_
#define _SMPS_H_



#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

#include <inttypes.h>


// -------------------------------------------------------------------------------------
// Pins

#define DIAG_LED	PB2
#define PWM_OUTPUT	PB4
#define COMP_INPUT	PB3

#define bool		uint8_t


// -------------------------------------------------------------------------------------
// Constants
#define	VOLTAGE_IN				5
#define VOLTAGE_OUT				3.3

#define DUTY_CYCLE_CLK			160
#define DUTY_CYCLE				(VOLTAGE_OUT/VOLTAGE_IN)
const uint8_t DUTY_CYCLE_PWM	= (uint8_t)(DUTY_CYCLE_CLK * DUTY_CYCLE);


#endif
