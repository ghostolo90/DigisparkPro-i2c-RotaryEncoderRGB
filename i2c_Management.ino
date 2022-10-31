void I2C_Init()
{
  // Setup as an i2c slave, the ports are fixed in the USI
  //  Pin 0: SDA - data
  //  Pin 1: SCL - clock
  Wire.begin(I2C_SLAVE_ADDRESS);
  Wire.onReceive(I2C_receiveEvent);
  Wire.onRequest(I2C_requestEvent);
}

void I2C_requestEvent()
{
  Wire.write(I2C_out_Button);
  Wire.write(I2C_out_Encoder);
  I2C_out_Encoder = 127; //reset value to be incremental
}

void I2C_receiveEvent(int howMany)
{
  if (howMany < 1)
  {
      return;
  }

  firstMessageReceived = true;
  int regPosition = 0;
  while(howMany--)
  {
    int tmpValue = Wire.read();
    switch(regPosition)
    {
      case I2C_in_Index_Mode:
      {
         I2C_in_Mode = tmpValue;
      }break;
      case I2C_in_Index_Mode_Args:
      {
         I2C_in_Mode_Args = tmpValue;
      }break;
      case I2C_in_Index_R:
      {
         I2C_in_R = tmpValue;
      }break;
      case I2C_in_Index_G:
      {
         I2C_in_G = tmpValue;
      }break;
      case I2C_in_Index_B:
      {
         I2C_in_B = tmpValue;
      }break;
    }
    regPosition++;
  }
}
