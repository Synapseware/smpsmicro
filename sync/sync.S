; ASM routines
#include <avr/io.h>
#include "board.h"


.data

delay:			.zero	1


.text


;------------------------------------------------------------------------------
; extern's from the rest of the program
.extern _sample							; 8 bit value



;------------------------------------------------------------------------------
;ADC conversion complete
.global ADC_vect

	ADC_vect:
		push 	r24
		in		r24, _SFR_IO_ADDR(SREG)
		push	r24

		lds		r24, ADCH
		sts		_sample, r25

		pop		r24
		out		_SFR_IO_ADDR(SREG), r24
		pop 	r24

		reti

;------------------------------------------------------------------------------
; TIMER0 Compare A
; Runs at 125Hz.  Used to toggle a heart-beat LED
.global TIM0_COMPA_vect

	TIM0_COMPA_vect:
		push 	r24
		in 		r24, _SFR_IO_ADDR(SREG)
		push 	r24

		lds 	r24, delay							; load delay

		cpi 	r24, 62								; delay == 62?
		brne 	tcv0								; false

		sbi 	_SFR_IO_ADDR(PORTB), DIAG_LED		; true: PORTB |= (1<<DIAG_LED)
		rjmp 	tc_updateDelay						; jump to delay update

	tcv0:
		cpi 	r24, 124							; delay == 124?
		brne 	tc_updateDelay						; false

		cbi 	_SFR_IO_ADDR(PORTB), DIAG_LED		; true: PORTB &= ~(1<<DIAG_LED)

	tc_updateDelay:									; updates the delay
		tst 	r24									; delay == 0?
		brne   	tcud1								; false
		ldi 	r24, 125							; true
	tcud1:
		dec 	r24									; delay--
		sts 	delay, r24							; save the new delay

		pop 	r24
		out 	_SFR_IO_ADDR(SREG), r24
		pop 	r24

		reti
