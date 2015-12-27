#include "buck.h"


// -------------------------------------------------------------------------------------
// Used to drive system events
static void initTimer0(void)
{
	// setup timer0 as our voltage comparator function
	TCCR0A	=	(1<<WGM01)	|		// CTC
				(0<<WGM00);

	TCCR0B	=	(0<<CS02)	|		// CLK/64: 16MHz/64 = 250KHz
				(1<<CS01)	|
				(1<<CS00)	|
				(0<<WGM02);			// CTC

	OCR0A	= 249;					// 250kHz/250 = 1KHz
	TIMSK	|= (1<<OCIE0A);			// enable TCNT01 == OCR0A interrupt
}

// -------------------------------------------------------------------------------------
// Timer1 is the SMPS duty cycle timer
static void initTimer1(void)
{
	PLLCSR	=	(0<<LSM)	|		// disable low-speed mode (assuming two 1.2NiMh batteries)
				(1<<PLLE)	|		// make sure PLL is enabled
				(1<<PCKE);			// enable high speed PLL clock

	TCCR1	=	(0<<CTC1)	|
				(0<<COM1A1)	|
				(0<<COM1A0)	|
				(0<<PWM1A)	|		// disable PWM, channel A
				(0<<CS13)	|		// PLLCLK/1
				(0<<CS12)	|		// 
				(1<<CS11)	|		// 
				(0<<CS10);			// 

	GTCCR	=	(1<<PWM1B)	|		// enable PWM, channel B
				(1<<COM1B1)	|		// Clear at 0, set at match (we're driving p-channel mosfet!)
				(0<<COM1B0)	|
				(0<<TSM)	|		// disable counter/timer sync mode
				(1<<COM1A1);		// we want to toggle OC1B with our PWM signal

	OCR1C	=	DUTY_CYCLE_CLK - 1;
	OCR1A	=	0;
	OCR1B	=	DUTY_CYCLE_CLK - DUTY_CYCLE_PWM;
}

// -------------------------------------------------------------------------------------
// Setup the ADC to sample on ADC_INPUT
static void initComparator(void)
{
	ADCSRA	=	(0<<ADEN)	|		// Disable the ADC
				(0<<ADSC)	|
				(0<<ADATE)	|
				(0<<ADIE)	|
				(0<<ADPS2)	|
				(0<<ADPS1)	|
				(0<<ADPS0);

	ADMUX	=	(0<<REFS2)	|
				(0<<REFS1)	|
				(0<<REFS0)	|
				(0<<ADLAR)	|
				(0<<MUX3)	|
				(0<<MUX2)	|
				(1<<MUX1)	|		// Select ADC3 as negative input to comparator
				(1<<MUX0);

	ADCSRB	=	(1<<ACME)	|
				(0<<BIN)	|
				(0<<IPR)	|
				(0<<ADTS2)	|
				(0<<ADTS1)	|
				(0<<ADTS0);

	ACSR	=	(0<<ACD)	|
 				(1<<ACBG)	|		// Use band-gap reference (1.1v)
				(0<<ACO)	|
				(0<<ACI)	|
				(0<<ACIE)	|
				(0<<ACIS1)	|
				(0<<ACIS0);

	DIDR0	=	(1<<ADC3D)	|		// disable digital input on PB3
				(0<<ADC2D)	|
				(0<<ADC1D)	|
				(0<<ADC0D);

	DDRB	&=	~(1<<COMP_INPUT);	// Set comparator pin as input
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
	DDRB	&=	~(1<<PWM_OUTPUT);
}

// -------------------------------------------------------------------------------------
// Enables the PWM output
static void pwmOn(void)
{
	// enable PWM output pin
	DDRB	|=	(1<<PWM_OUTPUT);
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
	initComparator();

	pwmOff();

	sei();
}

// -------------------------------------------------------------------------------------
// Process the data from the ADC
static void processComparator(void)
{
	// check comparator - we'll do simple skip-mode type PWM.
	// If the voltage is higher, skip a cycle.  If the voltage is lower,
	// don't skip a cycle.  We'll skip by turning the PWM output
	// on and off (by setting OC1B to either 0 or the PWM value)

	if ((ACSR & (1<<ACO)) == 0)
	{
		// comparater output is 0 - positive input is greater than negative input
		// AIN < REF
		// Enable PWM output because voltage is too low
		pwmOff();
	}
	else
	{
		// comparator output is 1 - positive input is less than negative input
		// AIN > REF
		// Disable PWM output because voltage is too high
		pwmOn();
	}
}

// -------------------------------------------------------------------------------------
int main(void)
{
	setup();

	//WDTCR &= 0b11011000;	// set for 16ms timeout
	//WDTCR |= (1<<WDE);		// enable watchdog timer

	pwmOn();

	while(1)
	{
		//wdt_reset();
		processComparator();
	}

	return 0;
}

// -------------------------------------------------------------------------------------
// Timer0 compareA interrupt handler (running @ 1KHz)
ISR(TIM0_COMPA_vect)
{
	// no-op
}
