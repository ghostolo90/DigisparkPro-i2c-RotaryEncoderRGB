bool Blink_Restart = false;
byte Blink_Start_R = 0;
byte Blink_Start_G = 0;
byte Blink_Start_B = 0;
unsigned int Blink_Delay_Timer = 0; //0.1s to 25.5s
unsigned long Blink_Delay_Time_Millis = 0;
bool Blink_Progress_Value = false;
int Blink_Fade_Variation_Timer = 50; //ms
unsigned int Blink_Fade_Progress_Delay = 0;
byte Blink_Fade_Progress_R = 0;
byte Blink_Fade_Progress_G = 0;
byte Blink_Fade_Progress_B = 0;
unsigned long Blink_Fade_Progress_Time_Micros = 0;
int Blink_Fade_Progress_Index = 0;
int Blink_Fade_Progress_Increment = 0;

void RGB_Blink_Restart(bool restart)
{
  Blink_Restart = restart;
}
void RGB_Blink_Update(byte R, byte G, byte B, byte Timer)
{
  //timer is multiplied by 100 ms
  unsigned int TimerMs = (((unsigned int)Timer) * 100) / 2.0;
  #ifdef RGB_BLINK_WITH_FADE
  TimerMs = TimerMs - Blink_Fade_Variation_Timer;
  #endif
  if(Blink_Start_R != R || Blink_Start_G != G || Blink_Start_B != B || Blink_Delay_Timer != TimerMs)
  {
      Blink_Restart = true;
  }

  if(Blink_Restart)
  {
    Blink_Restart = false;
    Blink_Start_R = R;
    Blink_Start_G = G;
    Blink_Start_B = B;
    Blink_Delay_Timer = TimerMs;
    #ifdef RGB_BLINK_WITH_FADE
    //blink with short fade on color change
    Blink_Fade_Progress_Delay = (Blink_Fade_Variation_Timer * 1000.0) / 255.0; //micros
    Blink_Fade_Progress_Index = 255;
    Blink_Fade_Progress_Increment = -1;
    Blink_Fade_Progress_R = Blink_Start_R * Blink_Fade_Progress_Index / 256; //Red
    Blink_Fade_Progress_G = Blink_Start_G * Blink_Fade_Progress_Index / 256; //Green
    Blink_Fade_Progress_B = Blink_Start_B * Blink_Fade_Progress_Index / 256; //Blue
    IO_WriteColor(Blink_Fade_Progress_R, Blink_Fade_Progress_G, Blink_Fade_Progress_B, false);
    Blink_Fade_Progress_Time_Micros = 0;
    #else
    //blink with immediate color change
    IO_WriteColor(Blink_Start_R, Blink_Start_G, Blink_Start_B, false);
    #endif
    Blink_Progress_Value = true;
    Blink_Delay_Time_Millis = millis();
  }

  if((millis() - Blink_Delay_Time_Millis >= Blink_Delay_Timer) && (Blink_Delay_Time_Millis > 0))
  {
    #ifdef RGB_BLINK_WITH_FADE
    Blink_Delay_Time_Millis = 0;
    Blink_Fade_Progress_Time_Micros = micros();
    #else
    if(Blink_Progress_Value)
    {
      IO_WriteColor(0, 0, 0, false);
      Blink_Progress_Value = false;
    }
    else
    {
      IO_WriteColor(Blink_Start_R, Blink_Start_G, Blink_Start_B, false);
      Blink_Progress_Value = true;
    }
    Blink_Delay_Time_Millis = millis();
    #endif
  }

  unsigned long tmp_micros = micros();
  unsigned long tmp_delay = Blink_Fade_Progress_Delay;
  //the micros value reset each 70 minutes, when it resets is necessary to adapt the value of the delay to wait
  if(Blink_Fade_Progress_Time_Micros > tmp_micros)
  {
    tmp_delay = tmp_delay - (0xFFFFFFFF - (Blink_Fade_Progress_Time_Micros + 1));
    Blink_Fade_Progress_Time_Micros = 1;
  }
  if((tmp_micros - Blink_Fade_Progress_Time_Micros >= tmp_delay) && (Blink_Fade_Progress_Time_Micros > 0))
  {
    Blink_Fade_Progress_Index = Blink_Fade_Progress_Index + Blink_Fade_Progress_Increment;
    bool changeDirection = false;
    if(Blink_Fade_Progress_Index >= 255)
    {
      Blink_Fade_Progress_Index = 255;
      Blink_Fade_Progress_Increment = -1;
      Blink_Delay_Time_Millis = millis();
      changeDirection = true;
    }
    if(Blink_Fade_Progress_Index <= 0)
    {
      Blink_Fade_Progress_Index = 0;
      Blink_Fade_Progress_Increment = +1;
      Blink_Delay_Time_Millis = millis();
      changeDirection = true;
    }
    Blink_Fade_Progress_R = Blink_Start_R * Blink_Fade_Progress_Index / 256; //Red
    Blink_Fade_Progress_G = Blink_Start_G * Blink_Fade_Progress_Index / 256; //Green
    Blink_Fade_Progress_B = Blink_Start_B * Blink_Fade_Progress_Index / 256; //Blue
    IO_WriteColor(Blink_Fade_Progress_R, Blink_Fade_Progress_G, Blink_Fade_Progress_B, false);    
    Blink_Fade_Progress_Time_Micros = micros();
    if(changeDirection)
    {
      Blink_Fade_Progress_Time_Micros = 0;
    }
  }
}
