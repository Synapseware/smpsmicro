#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <avr/wdt.h>


#define CYCLE_MAX 71
#define CYCLE_MIN 2
volatile unsigned char _dutycycle = CYCLE_MIN;


#define DIAG_LED	PINB3
#define PWM_OUTPUT	PINB4
#define ACA_INPUT	PINB2


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void setup()
{
	DDRB	|= (1<<PWM_OUTPUT);		// set as output
	PORTB	&= ~(1<<PWM_OUTPUT);	// start PWM low


	// adjust power saving modes
	PRR		&= ~(1<<PRTIM0);		// enable timer0
	PRR		&= ~(1<<PRTIM1);		// enable timer1
	PRR		|= (1<<PRUSI);			// disable USI
	PRR		|= (1<<PRADC);			// disable ADC


	// enable PLL
	PLLCSR	|= (1<<LSM);			// enable low-speed mode (assuming two 1.2NiMh batteries)
	PLLCSR	|= (1<<PLLE);			// enable PLL
	_delay_ms(2);					// wait for PLL lock
	while ((PLLCSR & (1<<PLOCK)) == 0){}
	PLLCSR	|= (1<<PCKE);			// enable high speed PLL clock


	// setup timer1, 400kHz, PWM, OCR1B
	//GTCCR	|= (1<<TSM);			// enable counter/timer sync mode
	GTCCR	|= (1<<PWM1B);			// enable PWM, channel B
	GTCCR	|= (1<<COM1B1);			// we want to toggle OC1B with our PWM signal
	TCCR1	|= (1<<PWM1A);			// ???
	TCCR1	&= ~(1<<CS13);			// 400kHz
	TCCR1	&= ~(1<<CS12);			// 400kHz
	TCCR1	&= ~(1<<CS11);			// 400kHz
	TCCR1	|= (1<<CS10);			// 400kHz
	OCR1C	= 81;					// 400kHz
	OCR1B	= _dutycycle;			// default duty cycle


	// setup comparator
	ADCSRA	&= ~(1<<ADEN);			// disable analog/digital converter
	DDRB	&= ~(1<<PINB3);			// set pin as input
	DIDR0	|= (1<<ADC3D);			// disable digital input on ADC3D
	ACSR	&= ~(1<<ACD);			// enable analog comparator
	ACSR	|= (1<<ACBG);			// enable band-pass (1.1v comparator on ANI0)
	DDRB 	|= (1<<DIAG_LED);		// set as output


	// setup timer0 as our voltage comparator function
	TCCR0A	|= (1<<WGM01);			// CTC
	TCCR0B	|= (1<<CS01);			// clk/64 = 16mHz/1024 = 250kHz
	TCCR0B	|= (1<<CS00);
	OCR0A	= 4;					// 250kHz/25 = 10kHz
	TIMSK	|= (1<<OCIE0A);			// enable TCNT01 == OCR0A interrupt


	// enable interrupts!
	sei();
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Timer0 compare
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
ISR(TIM0_COMPA_vect)
{
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
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int main()
{
	setup();

	WDTCR &= 0b11011000;	// set for 16ms timeout
	WDTCR |= (1<<WDE);		// enable watchdog timer

	while(1)
	{
		wdt_reset();
	}

	return 0;
}
