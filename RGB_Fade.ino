bool Fade_Restart = false;
byte Fade_Start_R = 0;
byte Fade_Start_G = 0;
byte Fade_Start_B = 0;
unsigned int Fade_Variation_Timer = 0; //0.1s to 25.5s
unsigned int Fade_Progress_Delay = 0;
byte Fade_Progress_R = 0;
byte Fade_Progress_G = 0;
byte Fade_Progress_B = 0;
unsigned long Fade_Progress_Time_Micros = 0;
int Fade_Progress_Index = 0;
int Fade_Progress_Increment = 0;

void RGB_Fade_Restart(bool restart)
{
  Fade_Restart = restart;
}
void RGB_Fade_Update(byte R, byte G, byte B, byte Timer)
{
  //timer is multiplied by 100 ms
  unsigned int TimerMs = ((unsigned int)Timer) * 100;
  
  if(Fade_Start_R != R || Fade_Start_G != G || Fade_Start_B != B || Fade_Variation_Timer != TimerMs)
  {
    Fade_Restart = true;
  }

  if(Fade_Restart)
  {
    Fade_Restart = false;
    Fade_Start_R = R;
    Fade_Start_G = G;
    Fade_Start_B = B;
    Fade_Variation_Timer = TimerMs;
    Fade_Progress_Delay = ((Fade_Variation_Timer * 1000.0) / 255.0) / 2; //micros
    Fade_Progress_Index = 255;
    Fade_Progress_Increment = -1;
    Fade_Progress_R = Fade_Start_R * Fade_Progress_Index / 256; //Red
    Fade_Progress_G = Fade_Start_G * Fade_Progress_Index / 256; //Green
    Fade_Progress_B = Fade_Start_B * Fade_Progress_Index / 256; //Blue
    IO_WriteColor(Fade_Progress_R, Fade_Progress_G, Fade_Progress_B, false);
    Fade_Progress_Time_Micros = micros();
  }

  unsigned long tmp_micros = micros();
  unsigned int tmp_delay = Fade_Progress_Delay;
  //the micros value reset each 70 minutes, when it resets is necessary to adapt the value of the delay to wait
  if(Fade_Progress_Time_Micros > tmp_micros)
  {
    tmp_delay = tmp_delay - (0xFFFFFFFF - (Fade_Progress_Time_Micros + 1));
    Fade_Progress_Time_Micros = 1;
  }
  if((tmp_micros - Fade_Progress_Time_Micros >= tmp_delay) && (Fade_Progress_Time_Micros > 0))
  {
    Fade_Progress_Index = Fade_Progress_Index + Fade_Progress_Increment;
    if(Fade_Progress_Index >= 255)
    {
      Fade_Progress_Index = 255;
      Fade_Progress_Increment = -1;
    }
    if(Fade_Progress_Index <= 0)
    {
      Fade_Progress_Index = 0;
      Fade_Progress_Increment = +1;
    }
    Fade_Progress_R = Fade_Start_R * Fade_Progress_Index / 256; //Red
    Fade_Progress_G = Fade_Start_G * Fade_Progress_Index / 256; //Green
    Fade_Progress_B = Fade_Start_B * Fade_Progress_Index / 256; //Blue
    IO_WriteColor(Fade_Progress_R, Fade_Progress_G, Fade_Progress_B, false);
    Fade_Progress_Time_Micros = micros();
  }
}
