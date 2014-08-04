#include "smps.h"

volatile uint8_t	_process		= 0;
volatile uint8_t	_lastSample		= 0;
volatile uint8_t	_pwmtarget		= 0;


// -------------------------------------------------------------------------------------
// Used to drive system events
void initTimer0(void)
{
	// setup timer0 as our voltage comparator function
	TCCR0A	=	(1<<WGM01);			// CTC
	TCCR0B	=	(1<<CS01)	|		// clk/64 = 16mHz/1024 = 250kHz
				(1<<CS00);

	OCR0A	= 4;					// 250kHz/25 = 10kHz
	TIMSK	|= (1<<OCIE0A);			// enable TCNT01 == OCR0A interrupt
}

// -------------------------------------------------------------------------------------
// Timer1 is the SMPS duty cycle timer
void initTimer1(void)
{
	// Setup PLLCSR - note: PLL is enabled via fuses because we are driving uC clock from the PLL! (pg 97)
	PLLCSR	=	(0<<LSM)	|		// disable low-speed mode (assuming two 1.2NiMh batteries)
				(1<<PCKE);			// enable high speed PLL clock

	// setup timer1, 500kHz, PWM, OCR1A
	GTCCR	=	(0<<PWM1B)	|		// disable PWM, channel B
				(0<<TSM)	|		// disable counter/timer sync mode
				(1<<COM1A1);		// we want to toggle OC1B with our PWM signal

	TCCR1	=	(0<<CTC1)	|
				(1<<COM1A1)	|
				(0<<COM1A0)	|
				(1<<PWM1A)	|		// enable PWM, channel A
				(0<<CS13)	|		// PCK/1
				(0<<CS12)	|		// 
				(0<<CS11)	|		// 
				(1<<CS10);			// 

	OCR1C	=	CYCLE_CLK - 1;		// 500KHz (64MHz / 128 = 500KHz)
	OCR1A	=	0;					// default duty cycle
	OCR1B	=	0;					// testing

	DDRB	|=	(1<<PWM_OUTPUT);	// set as output
	PORTB	&=	~(1<<PWM_OUTPUT);	// start PWM low
}

// -------------------------------------------------------------------------------------
// Setup the ADC to sample on ADC_INPUT
void initADC(void)
{
	ADMUX	=	(0<<REFS2)	|		// 1.1V: REF[2:0] = 010
				(1<<REFS1)	|
				(0<<REFS0)	|
				(1<<ADLAR)	|		// Left-adjust result (read ADCL, then ADCH)
				(0<<MUX3)	|		// Setup to read ADC2/PB4
				(0<<MUX2)	|
				(1<<MUX1)	|
				(0<<MUX0);

	ADCSRA	=	(1<<ADEN)	|		// Enable the ADC
				(1<<ADSC)	|
				(0<<ADATE)	|
				(0<<ADIE)	|
				(1<<ADPS2)	|		// Setup ADC clock to be PCK/128
				(1<<ADPS1)	|
				(1<<ADPS0);

	ADCSRB	=	(0<<BIN)	|
				(0<<IPR)	|
				(0<<ADTS2)	|
				(0<<ADTS1)	|
				(0<<ADTS0);

	// wait for conversion to complete
	_delay_ms(100);

	// enable interrupt handling on the ADC
	ADCSRA	|=	(1<<ADIF)	|
				(1<<ADIE);

	DDRB	&=	~(1<<PB4);			// Set PB4 as input

	DIDR0	=	(0<<ADC3D)	|
				(1<<ADC2D)	|		// Disable digital input on ADC2
				(0<<ADC1D)	|
				(0<<ADC0D);
}

// -------------------------------------------------------------------------------------
void initDiagLed(void)
{
	DDRB	|=	(1<<DIAG_LED);
	PORTB	|=	(1<<DIAG_LED);
}

// -------------------------------------------------------------------------------------
void setup()
{
	// adjust power saving modes
	PRR		= 	(0<<PRTIM0) |		// enable timer0
				(0<<PRTIM1) |		// enable timer1
				(1<<PRUSI)	|		// disable USI
				(0<<PRADC);			// enable ADC

	initDiagLed();
	initTimer0();
	initTimer1();
	initADC();
}

// -------------------------------------------------------------------------------------
void processADC(void)
{
	// turn the LED on if it's within the sample range
	if (_lastSample > TARGET_ADC - 2 && _lastSample < TARGET_ADC + 2)
		PORTB	&=	~(1<<DIAG_LED);
	else
		PORTB	|=	(1<<DIAG_LED);

	//if (0 == _lastSample)
	//{
	//	// protect against LED open circuit
	//	_pwmtarget = 0;
	//}
	//else
	if (_lastSample > TARGET_ADC - 2)
	{
		if (_pwmtarget > 1)
			_pwmtarget--;
	}
	else if (_lastSample < TARGET_ADC + 2)
	{
		if (_pwmtarget < CYCLE_MAX)
			_pwmtarget++;
	}

	OCR1A = _pwmtarget;

	_process = 0;

	// start a conversion
	ADCSRA |= (1<<ADSC);
}

// -------------------------------------------------------------------------------------
int main()
{
	setup();

	WDTCR &= 0b11011000;	// set for 16ms timeout
	WDTCR |= (1<<WDE);		// enable watchdog timer

	sei();

	// start a conversion
	ADCSRA |= (1<<ADSC);

	while(1)
	{
		wdt_reset();

		if (_process)
			processADC();
	}

	return 0;
}

// -------------------------------------------------------------------------------------
// ADC conversion complete interrupt handler
ISR(ADC_vect)
{
	_process = 1;
	_lastSample = ADCH;
}

// -------------------------------------------------------------------------------------
// Timer0 compare
ISR(TIM0_COMPA_vect)
{
	/*
	// increment/decrement duty cycle based on comparator result
	// ACO in ACSR is set when AIN0 (ref) is > AIN1.  In this scenario,
	// ACO will be set when the voltage sags too low - more duty
	// cycle will be required to keep the voltage up
	if ((ACSR & (1<<ACO)) != 0)
	{
		DDRB |= (1<<PWM_OUTPUT);
		_dutycycle++;
		if (_dutycycle > CYCLE_MAX)
		{
			PORTB |= (1<<DIAG_LED);
			_dutycycle = CYCLE_MAX;
		}
	}
	else
	{
		PORTB &= ~(1<<DIAG_LED);
		_dutycycle--;
		if (_dutycycle < CYCLE_MIN)
		{
			_dutycycle = CYCLE_MIN;

			// disable PWM output since voltage is too high
			DDRB &= ~(1<<PWM_OUTPUT);
		}
	}

	OCR1B = _dutycycle;
	*/
}