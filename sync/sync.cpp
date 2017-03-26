#include "sync.h"


volatile uint8_t _sample = 0;
const uint8_t DUTY_CYCLE_PWM	= (uint8_t)(DUTY_CYCLE_CLK * DUTY_CYCLE);


// -------------------------------------------------------------------------------------
// Used to drive system events
static void initTimer0(void)
{
	// setup timer0 as our voltage comparator function
	TCCR0A	=	(1<<WGM01)	|		// CTC
				(0<<WGM00);

	TCCR0B	=	(1<<CS02)	|		// CLK/1024: 16MHz/1024 = 15,625Hz
				(0<<CS01)	|
				(1<<CS00)	|
				(0<<WGM02);			// CTC

	OCR0A	= 125-1;				// 15.625kHz/125 = 125Hz
	TIMSK	|= (1<<OCIE0A);			// enable TCNT01 == OCR0A interrupt
}

// -------------------------------------------------------------------------------------
// Timer1 is the SMPS duty cycle timer
static void initTimer1(void)
{
	// Configure Timer1
	{
		// Stop T/C1
		TCCR1	=	0;

		// Configure the PLL as the clock source for T/C1
		PLLCSR	=	(1<<PLLE);			// Enable the PLL	
		while ((PLLCSR & (1<<PLOCK)) == 0);
		PLLCSR	|=	(1<<PCKE);			// Set the PLL as T/C1 clock source

		// Configure T/C1
		TCCR1	=	(0<<CTC1)	|		// 
					(0<<COM1A1)	|		// see page 86 for details
					(1<<COM1A0)	|		// 
					(1<<PWM1A)	|		// enable channel A PWM
					(0<<CS13)	|		// PLLCLK/16
					(1<<CS12)	|		// PLLCLK/16
					(0<<CS11)	|		// PLLCLK/16
					(1<<CS10);			// PLLCLK/16

		GTCCR	|=	(0<<TSM)	|		// disable counter/timer sync mode
					(0<<PWM1B)	|		// no PWM on channel B
					(0<<COM1B1)	|		// 
					(0<<COM1B0)	|		// 
					(0<<FOC1B)	|		// 
					(0<<FOC1A)	|		// 
					(0<<PSR1)	|		// 
					(0<<PSR0);			// 

		TIMSK	&= ~(
					(1<<OCIE1A)	|		// clear the interrupt flags for Timer1
					(1<<OCIE1B)	|
					(1<<TOIE1)
				);

		// Set PWM delay values
		OCR1C	=	DUTY_CYCLE_CLK - 1;
		OCR1A	=	(DUTY_CYCLE_CLK / 4.7) - 1;
		OCR1B	=	(DUTY_CYCLE_CLK / 2) - 1;
	}

	// Configure the deadtime generator
	{
		DTPS1	=	(0<<DTPS11)	|		// Deadtime prescalar
					(0<<DTPS10);		// 

		DT1A	=	(0<<DT1AH3)	|		// 
					(0<<DT1AH2)	|		// 
					(1<<DT1AH1)	|		// 
					(1<<DT1AH0)	|		// 
					(0<<DT1AL3)	|		// 
					(0<<DT1AL2)	|		// 
					(1<<DT1AL1)	|		// 
					(1<<DT1AL0);		// 

		DT1B	=	(0<<DT1BH3)	|		// 
					(0<<DT1BH2)	|		// 
					(0<<DT1BH1)	|		// 
					(0<<DT1BH0)	|		// 
					(0<<DT1BL3)	|		// 
					(0<<DT1BL2)	|		// 
					(0<<DT1BL1)	|		// 
					(0<<DT1BL0);		// 
	}
}

// -------------------------------------------------------------------------------------
// Setup the ADC to sample on ADC_INPUT
static void initAdc(void)
{
	ADMUX	=	(0<<REFS2)	|	// 1.1v internal reference
				(1<<REFS1)	|	// 1.1v internal reference
				(0<<REFS0)	|	// 1.1v internal reference
				(1<<ADLAR)	|	// Left-adjust ADC result (so we just need to read ADCH)
				(0<<MUX3)	|	// 
				(0<<MUX2)	|	// 
				(1<<MUX1)	|	// Select ADC2D as ADC input
				(0<<MUX0);		// 

	ADCSRA	=	(1<<ADEN)	|	// Enable the ADC
				(1<<ADSC)	|	// Start a conversion
				(1<<ADATE)	|	// Enable auto-trigger
				(1<<ADIE)	|	// Enable interrupts
				(1<<ADPS2)	|	// clk/64
				(1<<ADPS1)	|	// clk/64
				(0<<ADPS0);		// clk/64

	ADCSRB	=	(0<<BIN)	|	// 
				(0<<IPR)	|	// 
				(0<<ADTS2)	|	// free-running
				(0<<ADTS1)	|	// free-running
				(0<<ADTS0);		// free-running

	DIDR0	|=	(1<<ADC_INPUT);	// Disable digital input on ADC2

	DDRB	&=	~(1<<ADC_INPUT);	// Set comparator pin as input
}

// -------------------------------------------------------------------------------------
static void initDiagLed(void)
{
	DDRB	|=	(1<<DIAG_LED);
	PORTB	|=	(1<<DIAG_LED);
}

// -------------------------------------------------------------------------------------
// Disables PWM output
static void pwmOff(void)
{
	// disable the output pin
	DDRB	&= ~(
				(1<<PWM_OUTPUT) |
				(1<<PWM_OUTPUT_COMP)
			);
}

// -------------------------------------------------------------------------------------
// Enables the PWM output
static void pwmOn(void)
{
	// enable PWM output pin
	DDRB	|=	(1<<PWM_OUTPUT) |
				(1<<PWM_OUTPUT_COMP);
}

// -------------------------------------------------------------------------------------
static void setup(void)
{
	cli();
	
	// adjust power saving modes
	PRR		= 	(0<<PRTIM0) |		// enable timer0
				(0<<PRTIM1) |		// enable timer1
				(1<<PRUSI)	|		// disable USI
				(0<<PRADC);			// enable ADC

	initDiagLed();
	initTimer0();
	initTimer1();
	initAdc();

	pwmOff();

	_sample = 0;

	sei();
}

// -------------------------------------------------------------------------------------
// Processes the ADC result to keep the voltage on target
static void ProcessLatestADCSample(uint8_t sample)
{
	//static uint8_t dutyCycle = 0;

	if (sample > 128)
	{
		//pwmOff();
		return;
	}

	//pwmOn();
}

// -------------------------------------------------------------------------------------
int main(void)
{
	setup();

	pwmOn();

	while(1)
	{
		ProcessLatestADCSample(_sample);
	}

	return 0;
}
