//example program on how to use DigisparkPro-i2c-RotaryEncoderRGB
//this program uses button click to alternate color mode for RGB led and ancoder value to modify intensity
//on power on LED is turned off
//on button short pressure (less than 500ms)
// - solid color shown - the led color rotate at each pressure in this way: red-green-blue-yellow-cyan-magenta-white-off ad it's intensity is setted to the maximum (255)
//                     - once selected this mode, rotating encoder modify color intensity
//on button medium pressure (between 500ms and 2000ms)
// - blink color shown - the led selected with button short pressure start blinking every 750ms
//on button long pressure (between 2000ms and 5000ms)
// - fade color shown - the led selected with button short pressure start fading in 1500ms
//on button very long pressure (over 5000ms)
// - rainbow color shown - the GRB led rotate automatically all the colors (red-yellow-green-cyan-blue-magenta) in 6000ms

int countColor = 0;
bool firstLoop = true;

int Encoder_Address = 0x52;
bool Encoder_Present = false;

int encoderState = 0;
int encoderStateOld = 0;
int buttonState = 0;
int buttonStateOld = 0;

long buttonPressMillis = 0;

int buttonToShow = 0;
int countToShow = 0;

byte colorSelected_R = 0;
byte colorSelected_G = 0;
byte colorSelected_B = 0;

byte ModeToSend = 0;
byte ModeDelayToSend = 0;
byte ColorToSend_R = 0;
byte ColorToSend_G = 0;
byte ColorToSend_B = 0;

#define Index_Mode      0
#define Index_Mode_Args 1
#define Index_R         2
#define Index_G         3
#define Index_B         4

#include <Wire.h>

void setup()
{
  Serial.begin(9600); // init and start Serial
  delay(100);
  
  Wire.begin(); // join I2C bus (address optional for master)
  delay(100);

  // init take 5 seconds because digispark bootloadder need 5 seconds to start booting
  Serial.print("INIT");
  delay(1000);
  Serial.print(".");
  delay(1000);
  Serial.print(".");
  delay(1000);
  Serial.print(".");
  delay(1000);
  Serial.print(".");
  delay(1000);
    
  Serial.println();

  Serial.print("WAIT ENCODER");
  
  do
  {    
    Encoder_Present = Check_Slave_Present(Encoder_Address);    
    if(Encoder_Present)
      Serial.print(" - CONNECTED");
    else
      Serial.print(".");    
    Serial.println();    
    delay(1000);    
  }while(!Encoder_Present);

  //clear color selection
  WriteNoColor();
  
  delay(100);
  
}

void loop()
{
  //get info from encoder and send color requests
  ExchangeInfoWithEncoder();

  //button state change event
  if(buttonState != buttonStateOld || firstLoop)
  {
    //print button state
    buttonToShow = buttonState;
    PrintEncoderButtonState();

    //rise button press
    if(buttonState && !firstLoop)
    {
      //save time when button pressure occured
      buttonPressMillis = millis();
    }
    //fall button press
    if(!buttonState && buttonPressMillis > 0 && !firstLoop)
    {
      //short press - change color, set mode to 1, solid color
      if(millis() - buttonPressMillis < 500)
      {
        //rotate color count 0 to 7
        countColor++;
        if(countColor > 7)
          countColor = 0;

        //reset encoder count to maximum, rotating it modify the color intensity
        countToShow = 255;
        encoderState = countToShow;

        //print updated count
        PrintEncoderButtonState();

        //set color from buttom press count with maximum intensity
        SelectColorFromButtonPressureCount(countColor, countToShow);

        //send color and mode to I2C
        WriteSolidColor(colorSelected_R, colorSelected_G, colorSelected_B);
      }
      else
      {
        //medium press between 500ms to 2000ms - enable blink on predefined color
        if(millis() - buttonPressMillis < 2000)
        {
          //send color and mode to I2C
          WriteBlinkColor(colorSelected_R, colorSelected_G, colorSelected_B, 750);
        }
        else
        {
          //long press between 2000ms to 5000ms - enable fade on predefined color
          if(millis() - buttonPressMillis < 5000)
          {
            //send color and mode to I2C
            WriteFadeColor(colorSelected_R, colorSelected_G, colorSelected_B, 1500);
          }
          else //very long press over 5000ms - enable reinbow on all colors
          {
            //send color and mode to I2C
            WriteRainbowColor(6000);
          }
        }
      }
      //reset time when button pressure occured
      buttonPressMillis = 0;
    }
  }
  buttonStateOld = buttonState;

  //encoder value change event
  if(encoderState != encoderStateOld || firstLoop)
  {
    //print encoder value
    countToShow = encoderState;
    if(countToShow > 255)
      countToShow = 255;
    if(countToShow < 0)
      countToShow = 0;
    encoderState = countToShow;
    PrintEncoderButtonState();

    //change behaviour based on current mode
    switch(GetCurrentMode())
    {
      case 1: //solid color - modify color intensity
        SelectColorFromButtonPressureCount(countColor, countToShow);        
        WriteSolidColor(colorSelected_R, colorSelected_G, colorSelected_B);
        break;
    }    
  }
  encoderStateOld = encoderState;

  firstLoop = false;

  delay(10);
}

int GetCurrentMode()
{
  return ModeToSend;
}
void WriteNoColor()
{
  ModeToSend = 0;
  ModeDelayToSend = 0;
  ColorToSend_R = 0;
  ColorToSend_G = 0;
  ColorToSend_B = 0;
}
void WriteSolidColor(byte R, byte G, byte B)
{
  ModeToSend = 1;
  ModeDelayToSend = 0;
  ColorToSend_R = R;
  ColorToSend_G = G;
  ColorToSend_B = B;
}
void WriteBlinkColor(byte R, byte G, byte B, int millisec)
{
  ModeToSend = 2;
  ModeDelayToSend = millisec / 100;
  ColorToSend_R = R;
  ColorToSend_G = G;
  ColorToSend_B = B;
}
void WriteFadeColor(byte R, byte G, byte B, int millisec)
{
  ModeToSend = 3;
  ModeDelayToSend = millisec / 100;
  ColorToSend_R = R;
  ColorToSend_G = G;
  ColorToSend_B = B;
}
void WriteRainbowColor(int millisec)
{
  ModeToSend = 4;
  ModeDelayToSend = millisec / 100;
  ColorToSend_R = 0;
  ColorToSend_G = 0;
  ColorToSend_B = 0;
}
void ExchangeInfoWithEncoder()
{
  int count = 0;

  //prepare the buffer to send for color request
  byte bufferToWrite[] = 
  {
      ModeToSend,     // 0: Mode: 0=off, 1=solid, 2=blink, 3=fade, 4=rainbow loop
      ModeDelayToSend,// 1: Mode arguments(mode 0/1 = nothing, mode 2 = delay between blink in 0.01s (0.1-25.5s), mode 3,4 = delay between color changes in 0.1s (0.1-25.5s)
      ColorToSend_R,  // 2: Red value
      ColorToSend_G,  // 3: Green value
      ColorToSend_B,  // 4: Blue value
  };
  
  //send the color request to I2C
  Wire.beginTransmission(Encoder_Address);
  count = 0;
  while(count < sizeof(bufferToWrite))
  {
    Wire.write(bufferToWrite[count]);
    count++;
  }
  Wire.endTransmission();
  
  delay(10);

  //request the button and encoder state
  count = 0;
  Wire.requestFrom(Encoder_Address, 2);
  while(Wire.available())
  {
    byte b = Wire.read();
    if(count == 0)
    {
      buttonState = b;
    }
    if(count == 1)
    {
      //encoder value 0 is the equivalent to 127; in this way is possible to receive negative steps on a byte (value less than 127) 
      encoderState = encoderState + (b - 127);
    }
    count++;
  }
  
}

bool Check_Slave_Present(int address)
{
  for(int count = 0; count < 10; count++)
  {
    Wire.requestFrom(address, 1);
    delay(10);
    bool conn = false;
    while(Wire.available())
    {
      conn = true;
      byte b = Wire.read();
    }
    if(conn)
      return true;
    delay(10);
  }
  return false;
}

void PrintEncoderButtonState()
{
  Serial.print("E");
  if(countToShow >= 0)
    Serial.print(" ");
  if(countToShow > -10 && countToShow < 0)
    Serial.print(" ");
  if(countToShow > -100 && countToShow < 0)
    Serial.print(" ");
  if(countToShow < 10 && countToShow >= 0)
    Serial.print(" ");
  if(countToShow < 100 && countToShow >= 0)
    Serial.print(" ");
  Serial.print(countToShow);
  Serial.print("     ");
  Serial.print("B");
  Serial.print(buttonToShow);    
  Serial.println();
}

void SelectColorFromButtonPressureCount(int buttonCount, byte encoderCount)
{
  switch(buttonCount)
  {
    case 0: //black
      colorSelected_R = 0;
      colorSelected_G = 0;
      colorSelected_B = 0;
      break;
    case 1: //red
      colorSelected_R = encoderCount;
      colorSelected_G = 0;
      colorSelected_B = 0;
      break;
    case 2: //green
      colorSelected_R = 0;
      colorSelected_G = encoderCount;
      colorSelected_B = 0;
      break;
    case 3: //blue
      colorSelected_R = 0;
      colorSelected_G = 0;
      colorSelected_B = encoderCount;
      break;
    case 4: //yellow
      colorSelected_R = encoderCount;
      colorSelected_G = encoderCount;
      colorSelected_B = 0;
      break;
    case 5: //cyan
      colorSelected_R = 0;
      colorSelected_G = encoderCount;
      colorSelected_B = encoderCount;
      break;
    case 6: //purple
      colorSelected_R = encoderCount;
      colorSelected_G = 0;
      colorSelected_B = encoderCount;
      break;
    case 7: //white
      colorSelected_R = encoderCount;
      colorSelected_G = encoderCount;
      colorSelected_B = encoderCount;
      break;
  }
}
