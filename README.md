# DigisparkPro-i2c-RotaryEncoderRGB
Via a Digispark Pro, obtain information from rotary encoder and its button, control the colour and pattern of the integrated RGB led over I2C.

I took inspiration from https://github.com/G42makes/ATTiny-i2c-RGB combining its code to Rotary Encoder management and controlling everything via I2C.

## Features
- Get incremental steps count
- Get button state
- Separate Red, Green, and Blue PWM colour channels
- RGB led 5 modes: off, full on, blink, fade in/out, rainbow loop
- I2C slave (default 7bit address: 0x52) to configure

## Using
Wire up a Rotary encoder with RGB led to the pins of the Digispark Pro according to the Fritzing diagram.
Is possbile to command a RGB led with common cathode or common anode defining it at the beginning of the code with "#define RGB_CONNECTED_TO_CATHODE"
![alt text](https://user-images.githubusercontent.com/4014421/199130435-134f134b-e675-4b08-9f37-5dcf366014ac.png)

Over I2C write the following byte numbers to make changes:
- Byte 0x00: Mode: Set the mode for the device.
	- Write 0x00: Off - don't actually do anything
	- Write 0x01: Solid On. Use the RGB values and just turn on the light at those intensities
	- Write 0x02: Blink Mode. Blink between off and the RGB values set, is possible to fade between colour and off defining "#define RGB_BLINK_WITH_FADE" (fade duration 50ms), speed is controlled by Byte 0x01 and it is expressed in 0.1s divisions from 0.1s. to 25.5s (10Hz to 0.04Hz)
	- Write 0x03: Fade Mode. Fades between off and the RGB values set, speed is controlled by Byte 0x01 and it is expressed in 0.1s divisions from 0.1s. to 25.5s (10Hz to 0.04Hz)
	- Write 0x04: Rainbow Loop. Fades between R-Y-G-C-B-M-R in a loop, speed is controlled by Byte 0x01 and it is expressed in 0.1s divisions from 0.1s. to 25.5s (10Hz to 0.04Hz)
- Byte 0x01: Mode Argument: Set mode specific values (see modes above)
- Byte 0x02: Red
	- Set Red value of the RGB LED. 0x00 is off, 0xFF is max intensity
	- If you are using only one current limiting resistor, on the cathode (-) terminal of the RGB LED, you will probably find that red acts weird near max intensity. Dialling back the max value of this register to something like 0xAA usually resolves this with no loss of colour ability.
- Byte 0x03: Green
	- Set Green value of the RGB LED. 0x00 is off, 0xFF is max intensity
- Byte 0x04: Blue
	- Set Blue value of the RGB LED. 0x00 is off, 0xFF is max intensity

Over I2C read the following byte numbers to get rotary encoder information:
- Byte 0x00: Button: Get the button state 
	- Read 0x00: Button released
	- Read 0x01: Button pressed
- Byte 0x01: Rotary encoder incremental steps
	Get the rotary encoder value from 0x7f, it resets at each reading.
	- Read 0x7f: No rotation from previous reading
	- Read >0x7f: Clockwise rotation increments value. Subtract 0x3f from the value to get step increments.
	- Read <0x7f: Counter-clockwise rotation decrements value. Subtract the value from 0x3f to get step decrements.

## Note
### Red Taking Over
Red generally needs a different resistor then Green and Blue (which often use the same value), so to ensure a balanced white colour at maximum brightness, the red resistor is 330Ohm and blue and green ones are 220Ohm.

### Led Blinking At Startup
The Digispark Pro uses micronucleus bootloader that takes 5 second to boot (https://github.com/micronucleus/micronucleus/) and during boot LED blinks uncontrolled; using 2 transistor (1 NPN and 1 PNP) I'm able to enable power on LED and button once the program is loaded only.

### No Delay In The Code
Instead of use delay as ATTiny-i2c-RGB, that interrupt the code execution and sometimes cause problem on I2C receiving, I structured the code to be fully pass-through and uses micros time difference to change colours. 

### Digispark Pro Fritzing Part
I created the Digispark Pro Fritzing component, it's not perfect, but it helps to understand how the circuit is made.

### Why
I need to use a rotary encoder with led backlight, but it needs a lot of pins (6 in total, which 3 PWM, 2 interrupts and 1 digital input) that normally are used for other stuffs, so the idea of remote it and access to information through i2c come up.
I initially used the ATTiny-i2c-RGB with an ATTIny85 for RGB control and I created a library to get information from the rotary encoder through another ATTIny85, but the 1 byte buffer limit was annoying.
So, I used a Digispark Pro (but it is possible also to use an ATTiny167) since it has 3 PWM individual channels, plenty of pin that support PCINT for the rotary encoder (INT0 and INT1 already used in RGB PWM output), and I cand send and receive information through I2C without splitting in 1 byte.
I added an example for Arduino Uno that print over serial the data received over I2C from the rotary encoder and the button, and it change RGB mode though button pressure. The example is documented in the code.

## References
- https://github.com/G42makes/ATTiny-i2c-RGB Project that inspired me
