#include "smps.h"

volatile uint16_t	_lastSample		= 0;
volatile uint8_t	_rampsteps		= 0;
volatile uint8_t	_pwmmax			= 0;
volatile bool		_process		= FALSE;

static Adc			_adc();

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
	// Setup PLLCSR - note: PLL is enabled via fuses because we are driving uC clock from the PLL! (pg 97)
	PLLCSR	=	(0<<LSM)	|		// disable low-speed mode (assuming two 1.2NiMh batteries)
				(1<<PLLE)	|		// make sure PLL is enabled
				(1<<PCKE);			// enable high speed PLL clock

	TCCR1	=	(0<<CTC1)	|
				(0<<COM1A1)	|
				(0<<COM1A0)	|
				(0<<PWM1A)	|		// disable PWM, channel A
				(0<<CS13)	|		// PLLCLK/1
				(0<<CS12)	|		// 
				(0<<CS11)	|		// 
				(1<<CS10);			// 

	// setup timer1, 500kHz, PWM, OCR1B
	GTCCR	=	(1<<PWM1B)	|		// enable PWM, channel B
				(1<<COM1B1)	|		// Set OC1B to clear on match (1 from 0 to OCR1B, 0 from OCR1B to OCR1C - reset clock)
				(0<<COM1B0)	|
				(0<<TSM)	|		// disable counter/timer sync mode
				(1<<COM1A1);		// we want to toggle OC1B with our PWM signal

	OCR1C	=	CYCLE_CLK - 1;		// 500KHz (64MHz / 128 = 500KHz)
	OCR1A	=	0;					// default duty cycle
	OCR1B	=	CYCLE_CLK * 0.67;	// testing

	PORTB	&=	~(1<<PWM_OUTPUT);	// start PWM low
}

// -------------------------------------------------------------------------------------
// Setup the ADC to sample on ADC_INPUT
static void initADC(void)
{
	/*
	ADMUX	=	(0<<REFS2)	|		// 1.1V: REF[2:0] = 010
				(1<<REFS1)	|
				(0<<REFS0)	|
				(0<<ADLAR)	|		// Right-adjust result (read ADCL, then ADCH)
				(0<<MUX3)	|		// Setup to read ADC2/PB4
				(0<<MUX2)	|
				(1<<MUX1)	|
				(0<<MUX0);

	ADCSRA	=	(1<<ADEN)	|		// Enable the ADC
				(1<<ADSC)	|
				(1<<ADATE)	|
				(1<<ADIE)	|
				(1<<ADPS2)	|		// Setup ADC clock to be PCK/128
				(1<<ADPS1)	|
				(1<<ADPS0);

	ADCSRB	=	(0<<BIN)	|
				(0<<IPR)	|
				(0<<ADTS2)	|		// Enable auto-triggering on Timer0 Compare Match A
				(1<<ADTS1)	|
				(1<<ADTS0);

	DIDR0	=	(1<<ADC3D)	|		// disable digital input on PB3
				(0<<ADC2D)	|
				(0<<ADC1D)	|
				(0<<ADC0D);

	DDRB	&=	~(1<<ADC_INPUT);	// Set PB4 as input
	*/

	_adc.init();
}

// -------------------------------------------------------------------------------------
static void initDiagLed(void)
{
	DDRB	|=	(1<<DIAG_LED);
	PORTB	|=	(1<<DIAG_LED);
}

// -------------------------------------------------------------------------------------
static void pwmOff(void)
{
	// disable PWM output pin
	DDRB	&=	~(1<<PWM_OUTPUT);
}

// -------------------------------------------------------------------------------------
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

	// initialize globals
	_lastSample	= 0;
	_rampsteps	= RAMP_STEPS;
	_pwmmax		= RAMP_START;

	initDiagLed();
	initTimer0();
	initTimer1();
	initADC();

	pwmOff();

	sei();
}

// -------------------------------------------------------------------------------------
// Process the data from the ADC
static void processADC(void)
{
	static uint16_t pwmTarget = 0;

	// What's the best way to detect an open-circuit condition for the LED?
	//	- connect another high-ohm resister between Vout and Rsense (10k or more?)
	//	- check for 0-volt reading from Rsense?
	//		* how to start circuit from an off-state?
	//
	// Ramp-up timer

	//if (0 == _lastSample)
	//{
	//	// protect against LED open circuit
	//	pwmTarget = 0;
	//}
	//else
	if (_lastSample > TARGET_ADC + 1)
	{
		PORTB	|=	(1<<DIAG_LED);

		// we've overshot the target voltage - diminish the duty cycle
		if (pwmTarget > 0)
			pwmTarget--;
	}
	else if (_lastSample < TARGET_ADC - 1)
	{
		PORTB	|=	(1<<DIAG_LED);

		// we've undershot the target voltage - increase the duty cycle
		if (pwmTarget < CYCLE_MAX)
			pwmTarget++;
	}
	else
	{
		// we're on target - turn on diag LED
		PORTB	&=	~(1<<DIAG_LED);
	}

	OCR1A = pwmTarget;
}

// -------------------------------------------------------------------------------------
static void processTimer0(void)
{
	// when powering up, limit the duty cycle to _pwmmax.
	if (_rampsteps > 0)
	{
		_rampsteps--;
		_pwmmax += RAMP_STEPS;
	}
	else
	{
		_pwmmax = CYCLE_MAX;
	}

	// check for open-circuit condition & restart the ramp-up process
	if (_rampsteps == 0 && _lastSample == 0)
	{
		_rampsteps = RAMP_DELAY;
		_pwmmax = RAMP_START;
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
		if (_process)
		{
			processADC();
			_process = FALSE;
		}
	}

	return 0;
}

// -------------------------------------------------------------------------------------
// ADC conversion complete interrupt handler
ISR(ADC_vect)
{
	uint16_t sample = (ADCH << 8) | (ADCL & 0xFF);

	_lastSample = sample;

	_process = TRUE;
}

// -------------------------------------------------------------------------------------
// Timer0 compareA interrupt handler (running @ 1KHz)
ISR(TIM0_COMPA_vect)
{
	processTimer0();
}
