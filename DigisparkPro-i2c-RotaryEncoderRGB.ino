//Accept control requests over I2C and use them to display on a RGB LED.
//Accept status request over I2C and return button state and rotary encoder increments

//include
#include <Wire.h>
#include "avr/interrupt.h"
#include "math.h"

//general definitions
#define I2C_SLAVE_ADDRESS         0x52 
#define RGB_CONNECTED_TO_CATHODE
#define RGB_BLINK_WITH_FADE

//define the pins
//  i2c I2C_PIN_DATA               0
#define NOT_USED                   1 // NOT USED OTHERWISE PWM WILL NOT WORK ON CHANNEL A (CHANNEL A => PIN 0,2,4 CHANNEL B => PIN 1,3 CHANNEL C => PIN 8) 
//  i2c I2C_PIN_CLOCK              2
#define IO_PIN_GREEN               3
#define IO_PIN_RED                 4
#define IO_PIN_BUTTON              5
#define IO_PIN_BLUE                8
#define IO_PIN_ENCODER_B           9
#define IO_PIN_ENABLE             10 // powering the RGB led of the rotary encoder with the 5V source causes an unwanted flashing during start-up; powering it via transistor avoids the problem
#define IO_PIN_ENCODER_A          11
//      free                      12

//define I2C message format
#define I2C_in_Index_Mode         0
#define I2C_in_Index_Mode_Args    1
#define I2C_in_Index_R            2
#define I2C_in_Index_G            3
#define I2C_in_Index_B            4
#define I2C_out_Index_Button      0
#define I2C_out_Index_Encoder     1

//variables containing data received and sent via I2C
byte I2C_in_Mode = 0;       //RGB command mode
byte I2C_in_Mode_Args = 0;  //RGB command mode arguments
byte I2C_in_R = 0;          //RGB value Red (0-255)
byte I2C_in_G = 0;          //RGB value Green (0-255)
byte I2C_in_B = 0;          //RGB value Blue (0-255)
byte I2C_out_Encoder = 127; //Rotary encoder clicks ==> encoder raw increments / ( pulse per revolution / pulse per click )
byte I2C_out_Button = 0;    //Button state

//startup operations flag
volatile bool firstMessageReceived = false;
volatile bool firstEncoderReadDone = false;

//encoder data from interrupts 
byte Encoder_RawValue = 127;                    //Rotary encoder raw value
volatile int Encoder_TempValue = 127;           //Rotary encoder value from interrupt
volatile int Encoder_LastEncoded = 0;           //Rotary encoder last transition
long Encoder_LastTransitionTime = 0;            //Last encoder transition time from millis() used stabilize value between ticks (at least 5ms from last transition) because sometimes the count is incremented of 3, sometimes 4, sometimes 5 at each knob tick
long Encoder_LastTransitionToStabilizeTime = 0; //Same as before, but detect (probably) end of rotation, so reset the increment

void setup()
{
  IO_Init();
  
  //Set the LED to OFF to start, it will init to match the registers on first interrupt
  IO_WriteColor(0, 0, 0, true);

  I2C_Init();

  Interrupt_Init();
  
  delay(100);
  
  IO_Enable(true);  
}

void loop()
{
  I2C_out_Button = digitalRead(IO_PIN_BUTTON);

  // if the last transition occurred at least 5 ms ago, the encoder knob is probably between 2 ticks during a rotation, it's possible to send normalized step count over I2C
  if(millis() - Encoder_LastTransitionTime > 5 && Encoder_LastTransitionTime > 0)
  {
    int offsetRaw = Encoder_TempValue - 127;
    double valueI2C = Encoder_RawValue + offsetRaw - 127;
    valueI2C = valueI2C / 4;
    double oldvalueI2C = Encoder_RawValue - 127;
    oldvalueI2C = oldvalueI2C / 4;
    int offsetI2C = round(valueI2C) - round(oldvalueI2C);
    
    Encoder_TempValue = 127;
    Encoder_RawValue = Encoder_RawValue + offsetRaw;
    I2C_out_Encoder = I2C_out_Encoder + offsetI2C;
    
    Encoder_LastTransitionTime = 0;
  }
  
  // if the last transition occurred at least 100 ms ago, the encoder knob is steadly between 2 ticks, it's possible to reset the raw value
  if(millis() - Encoder_LastTransitionToStabilizeTime > 100 && Encoder_LastTransitionToStabilizeTime > 0)
  {
    Encoder_RawValue = 127;
    Encoder_LastTransitionToStabilizeTime = 0;
  }

  //if I2C doesn't send any message, is not possible to set colors
  if(!firstMessageReceived)
  {
    return;
  }
  
  switch(I2C_in_Mode)
  {
    case 0x00:
      //turn off
      RGB_Blink_Restart(true);
      RGB_Fade_Restart(true);
      RGB_Rainbow_Restart(true);
      IO_WriteColor(0, 0, 0, false);
      break;
    case 0x01:
      //turn on with setted value
      RGB_Blink_Restart(true);
      RGB_Fade_Restart(true);
      RGB_Rainbow_Restart(true);
      IO_WriteColor(I2C_in_R, I2C_in_G, I2C_in_B, false);
      break;
    case 0x02:
      //intermittent with fast fade (off - setted value)
      //delay come from I2C_in_Mode_Args in 0.01s steps
      RGB_Fade_Restart(true);
      RGB_Rainbow_Restart(true);
      RGB_Blink_Update(I2C_in_R, I2C_in_G, I2C_in_B, I2C_in_Mode_Args);
      break;
    case 0x03:
      //fade in (off - setted value)
      //delay between each color value change come from I2C_in_Mode_Args in 0.1s
      RGB_Blink_Restart(true);
      RGB_Rainbow_Restart(true);
      RGB_Fade_Update(I2C_in_R, I2C_in_G, I2C_in_B, I2C_in_Mode_Args);
      break;
    case 0x04:
      //rotate colors R-Y-G-C-B-M-R
      //delay between each color value change come from I2C_in_Mode_Args in 0.1s
      RGB_Blink_Restart(true);
      RGB_Fade_Restart(true);
      RGB_Rainbow_Update(I2C_in_Mode_Args);
      break;
  }

}
