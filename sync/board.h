#ifndef __BOARD_H__
#define __BOARD_H__

#include <avr/io.h>


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
