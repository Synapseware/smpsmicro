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

#define DIAG_LED			PB4
#define PWM_OUTPUT			PB1
#define PWM_OUTPUT_COMP		PB0
#define ADC_INPUT			ADC2D


// -------------------------------------------------------------------------------------
// Constants
#define	VOLTAGE_IN				5
#define VOLTAGE_OUT				3.3
#define DUTY_CYCLE_CLK			170
#define DUTY_CYCLE				(VOLTAGE_OUT/VOLTAGE_IN)
const uint8_t DUTY_CYCLE_PWM	= (uint8_t)(DUTY_CYCLE_CLK * DUTY_CYCLE);

// voltage divider:
// R1 = 110k
// R2 = 10k
// Vin = 12v
// Vout = 1.0v
// with the ADC set at 1.1v, 10bit sample target should reach 930~931
// dividing the result by 4, target should be at ~232
// PWM output is running at 23.6kHz...  a 1uF cap between R2 and ground
// should stabilize the output voltage

#endif
