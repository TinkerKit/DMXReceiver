DMXReceiver

Techinal Specifications
Nominal Input Voltage (DC) 9 to 24 Vdc
Input Current 	10 A Max
Output Current 	2.5A Max/Ch
Number of DMX Channels 	4
Addressing DMX Range 	1 to 509

Setting Address 
The DMX address is set using the first 9 switches of the DMX address switch. 
The address is set as a binary number (with switch 1 the LSB). As the unit is a 4 channel receiver, it will respond 
to data in the set address, and the following 3 addresses in the DMX data stream. 

Using With DMX Input 
In order to use the TinkerKit DMX to PWM Converter with a DMX feed, simply set the desired address and connect the 
DMX feed to the DMX input. Remember to terminate the DMX feed at the end of the chain with a 120Ω resistor between 
the DMX + and - lines.

Test Mode 
The TinkerKit set each individual output up to full brightness. This test mode is entered by setting the DMX address 
to 0 (all switches down). While in this mode, any incoming DMX data is ignored. Once the address is changed 
from 0, pressing the RESET button will return the device to normal operation.

The firmware allows you to use the DMX receiver module using a mosfet output or a relay one.
The output mode can be selected changing the value of the define OUTPUT_MODE in this way:
MOSFET for mosfet output
RELAY for relay output.
