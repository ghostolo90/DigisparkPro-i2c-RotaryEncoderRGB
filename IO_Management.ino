void IO_Init()
{
  //Set the ROTARY ENCODER pins
  pinMode(IO_PIN_BUTTON,INPUT); 
  pinMode(IO_PIN_ENCODER_A, INPUT_PULLUP);
  pinMode(IO_PIN_ENCODER_B, INPUT_PULLUP);
  
  //Set the RGB pins
  pinMode(IO_PIN_RED,   OUTPUT);
  pinMode(IO_PIN_GREEN, OUTPUT);
  pinMode(IO_PIN_BLUE,  OUTPUT);
  
  //Set the ENABLE pin, to power on stuff when setup is completed
  pinMode(IO_PIN_ENABLE,OUTPUT);
}

void Interrupt_Init()
{
  /*
  Attiny167 pinout
  
                                      ----------
          (RXLIN/RXD/ADC0/PCINT0)PA0 | 1     20 | PB0(PCINT8/OC1AU/DI/SDA)
          (TXLIN/TXD/ADC1/PCINT1)PA1 | 2     19 | PB1(PCINT9/OC1BU/DO)
       (MISO/DO/OC0A/ADC2/PCINT2)PA2 | 3     18 | PB2(PCINT10/OC1AV/USCK/SCL)
         (INT1/ISRC/ADC3/PCINT3) PA3 | 4     17 | PB3(PCINT11/OC1BV)
                                AVCC | 5     16 | GND
                                AGND | 6     15 | VCC
  (MOSI/SDA/DI/ICP1/ADC4/PCINT4) PA4 | 7     14 | PB4(PCINT12/OC1AW/XTAL1/CLKI)
    (SCK/SCL/USCK/T1/ADC5/PCINT5)PA5 | 8     13 | PB5(PCINT13/ADC8/OC1BW/XTAL2/CLKO)
            (SS/AIN0/ADC6/PCINT6)PA6 | 9     12 | PB6(PCINT14/ADC9/OC1AX/INT0)
     (AREF/XREF/AIN1/ADC7/PCINT7)PA7 | 10    11 | PB7(PCINT15/ADC10/OC1BX/RESET/dW)
                                      ----------

  Digispark Pro interrupt pinout

              -------------USB--------
 PCINT15/PB7 | RST                VIN | VIN
  PCINT6/PA6 | 12                 GND | GND
  PCINT5/PA5 | 11                   5 | PA7/PCINT7
  PCINT4/PA4 | 10                   4 | PB3/PCINT11
  PCINT3/PA3 | 9                    3 | PB6/PCINT14
  PCINT2/PA2 | 8                    2 | PB2/PCINT10
  PCINT1/PA1 | 7                    1 | PB1/PCINT9
  PCINT0/PA0 | 6                    0 | PB0/PCINT8
         VCC | VCC|VCC|GND|VIN    VCC | VCC
              ------------------------
  */

  //PCINT fires event when there is a status change of a pin in the mask PCMSK
  
  //In case of pin from PCINT0 to PCINT7 the configuration is as follow:
  // PCICR |= B00000001;
  // PCMSK0 |= Bxxxxxxxx; => replace x with 1 of the pin enable to detect staatus change, otherwise 0. e.g. PCINT0 -> bit 0, PCINT5 -> bit 5 =>PCMSK0 |= B00100001;
  //Pin status change from PCINT0 to PCINT7 fires event ISR(PCINT0_vect) 
  
  //In case of pin from PCINT8 to PCINT15 the configuration is as follow:
  // PCICR |= B00000010;
  // PCMSK0 |= Bxxxxxxxx; => replace x with 1 of the pin enable to detect staatus change, otherwise 0. e.g. PCINT0 -> bit 0, PCINT5 -> bit 5 =>PCMSK0 |= B00100001;
  //Pin status change from PCINT8 to PCINT15 fires event ISR(PCINT1_vect) 

  //In my configuration ENC_A is pin 9 and ENC_B is pin 11 on Digispark Pro
  //These pins correspond to Attiny167 pin 4 and 8, respectevely PA3 (PCINT3) and PA5(PCINT5)
  //Both of PA3 (PCINT3) and PA5(PCINT5) are between PCINT0 to PCINT7 so I have to enable PCIE0 that correspond to bit 0 =>PCICR |= B00000001;
  //In the mask I have to enable bit 3 and bit 5 => PCMSK0 |= B00101000;
  
  PCICR |= B00000001;
  PCMSK0 |= B00101000;
}

ISR (PCINT0_vect) 
{
  int MSB = digitalRead(IO_PIN_ENCODER_A); //MSB = most significant bit
  int LSB = digitalRead(IO_PIN_ENCODER_B); //LSB = least significant bit
 
  int encoded = (MSB << 1)|LSB; //converting the 2 pin value to single number
  int sum = (Encoder_LastEncoded << 2) | encoded; //adding it to the previous encoded value

  if(firstEncoderReadDone) //start to count the second change, otherwise the count is not correct
  {
    if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011)
      Encoder_TempValue++;
    if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000)
      Encoder_TempValue--;
  }
  firstEncoderReadDone = true;
 
  Encoder_LastEncoded = encoded; //store this value for next time
 
  if (Encoder_TempValue < 0)
    Encoder_TempValue = 0;
  if (Encoder_TempValue > 255)
    Encoder_TempValue = 255;
  
  Encoder_LastTransitionTime = millis();
  Encoder_LastTransitionToStabilizeTime = millis();
} 


void IO_Enable(bool enable)
{
	if(enable)
	{
		digitalWrite(IO_PIN_ENABLE, HIGH);
	}
	else
	{
		digitalWrite(IO_PIN_ENABLE, LOW);
	}
}

byte oldR = 0;
byte oldG = 0;
byte oldB = 0;
void IO_WriteColor(byte R, byte G, byte B, bool force)
{
  if(R != oldR || force)
  {
    oldR = R;
    #ifdef RGB_CONNECTED_TO_CATHODE
    R = 255 - R;
    #endif
    analogWrite(IO_PIN_RED, R);
  }
  if(G != oldG || force)
  {
    oldG = G;
    #ifdef RGB_CONNECTED_TO_CATHODE
    G = 255 - G;
    #endif
    analogWrite(IO_PIN_GREEN, G);
  }
  if(B != oldB || force)
  {
    oldB = B;
    #ifdef RGB_CONNECTED_TO_CATHODE
    B = 255 - B;
    #endif
    analogWrite(IO_PIN_BLUE, B);
  }
}
