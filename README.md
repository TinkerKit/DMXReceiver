# DMXReceiver

Techinal Specifications
Nominal Input Voltage (DC) 9 to 24 Vdc (24V is to much for Relays board)
Input Current 	10 A Max
Output Current 	2.5A Max/Ch
Number of DMX Channels 	4
Addressing DMX Range 	1 to 509

# Advanced features - Smart mode (Activated by turning Switch#10 on):

Guidance to use

1. Turn Switch #10 on (SmartMode on now), Switch#5-#8 on (default delay), rest switches off  and restart board
2. Put board in Learning mode by turning Switch #9 on
3. Turn off all DMX light
4. Turn on Switch#1 on and then use all lights, which you want to associate with Relay#1
5. Turn off Switch#1 and all DMX lights
6. Repeat learning for channels 2-4 (with appropriate switches 2-4)
7. Turn off Switch#9 (Learning mode off, all changes will be written to EEPROM)
8. Ajust default delay by switches 1-8 and restart board (delay in tens of seconds. If 0: it will be 1 second delay)

* When Switch#9 off - Switches 1-8 will define delay, between all DMX channels, associated with Relay turned to off and Relay turned off (in 10th seconds)
* Relay turnes on immediatelly when at least, 1 associated DMX channel in use
* You can ajust individual delay for every channel at step 4, using switches 5-8 (in tens of seconds). Value 15 (on-on-on-on) is reserved for default delay.

[SmartMode designed to use togetger with LightHub Smarthome controller](https://github.com/anklimov/lighthub)
Recommeded for control AC power of LED transformers 

* When Switch #10 turned  off - the board will working as normal DMX relay

* Setting Address 

The DMX address is set using the first 9 switches of the DMX address switch. 
The address is set as a binary number (with switch 1 the LSB). As the unit is a 4 channel receiver, it will respond 
to data in the set address, and the following 3 addresses in the DMX data stream. 

* Using With DMX Input  

In order to use the TinkerKit DMX to PWM Converter with a DMX feed, simply set the desired address and connect the 
DMX feed to the DMX input. Remember to terminate the DMX feed at the end of the chain with a 120Ω resistor between 
the DMX + and - lines.

* Test Mode 

The TinkerKit set each individual output up to full brightness. This test mode is entered by setting the DMX address 
to 0 (all switches down). While in this mode, any incoming DMX data is ignored. Once the address is changed 
from 0, pressing the RESET button will return the device to normal operation.

* The firmware allows you to use the DMX receiver module using a mosfet output or a relay one.
The output mode can be selected changing the value of the define OUTPUT_MODE in this way:
MOSFET for mosfet output
RELAY for relay output.

