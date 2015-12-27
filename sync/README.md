Buck Converter
==============

This is a simple buck converter which uses an ATTiny85 and PWM output.  The circuit uses a simple P-Channel MOSFET, an inductor, a schottky diode and an output capacitor to derive the regulated output.  The output voltage is passed through a resistor divider network and the result is fed back to the processor's negative comparator input pin.  Using skip-mode control, the output can be carefully regulated.

Theoretically, any output voltage that is lower than the supply voltage to the buck circuit can be achieved.  However, since the current version uses the analog comparator's input, the lowest possible output voltage is 1.1v.  Using an op-amp or an external comparator source would be required in order to achieve output voltages of less than 1.1 volts.

Optionally, an ATTiny13 could be used instead of an ATTinyX5 since the compiled code is only a couple hundred bytes in size.  However, the ATTiny13 does not have the PWM resolution of the ATTinyX5 chips.
