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


#define CYCLE_CLK	250
#define CYCLE_MAX	(CYCLE_CLK * 0.70)
#define TARGET_ADC	74



#endif
