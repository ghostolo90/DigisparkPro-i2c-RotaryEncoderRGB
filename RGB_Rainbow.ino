bool Rainbow_Restart = false;
unsigned int Rainbow_Variation_Timer = 0; //0.1s to 25.5s
unsigned int Rainbow_Progress_Delay = 0;
byte Rainbow_Progress_R = 0;
byte Rainbow_Progress_G = 0;
byte Rainbow_Progress_B = 0;
unsigned long Rainbow_Progress_Time_Micros = 0;
int Rainbow_Progress_Index = 0;
int Rainbow_Progress_Phase = 0;

void RGB_Rainbow_Restart(bool restart)
{
  Rainbow_Restart = restart;
}
void RGB_Rainbow_Update(byte Timer)
{
  unsigned int TimerMs = ((unsigned int)Timer) * 100;
  if(Rainbow_Variation_Timer != TimerMs)
  {
    Rainbow_Restart = true;
  }
 
  if(Rainbow_Restart)
  {
    Rainbow_Restart = false;
    Rainbow_Variation_Timer = TimerMs;
    Rainbow_Progress_Delay = ((Rainbow_Variation_Timer * 1000.0) / 255.0) / 6.0; //micros
    Rainbow_Progress_Index = 0;
    Rainbow_Progress_Phase = 0;
    Rainbow_Progress_R = 255; //Red
    Rainbow_Progress_G = 0; //Green
    Rainbow_Progress_B = 0; //Blue
    IO_WriteColor(Rainbow_Progress_R, Rainbow_Progress_G, Rainbow_Progress_B, false);
    Rainbow_Progress_Time_Micros = micros();
  }
  
  unsigned long tmp_micros = micros();
  double tmp_delay = Rainbow_Progress_Delay;
  //the micros value reset each 70 minutes, when it resets is necessary to adapt the value of the delay to wait
  if(Rainbow_Progress_Time_Micros > tmp_micros)
  {
    tmp_delay = tmp_delay - (0xFFFFFFFF - (Rainbow_Progress_Time_Micros + 1));
    Rainbow_Progress_Time_Micros = 1;
  }
  if((tmp_micros - Rainbow_Progress_Time_Micros >= tmp_delay) && (Rainbow_Progress_Time_Micros > 0))
  {
    switch(Rainbow_Progress_Phase)
    {
      case 0:   //fade in Green
      {
        Rainbow_Progress_Index = Rainbow_Progress_Index + 1;
        Rainbow_Progress_G = Rainbow_Progress_Index; //Green
        if(Rainbow_Progress_Index >= 255)
        {
          Rainbow_Progress_Index = 255;
          Rainbow_Progress_Phase = 1;
        }
      }break;
      case 1:   //fade out Red
      {
        Rainbow_Progress_Index = Rainbow_Progress_Index - 1;
        Rainbow_Progress_R = Rainbow_Progress_Index; //Red
        if(Rainbow_Progress_Index <= 0)
        {
          Rainbow_Progress_Index = 0;
          Rainbow_Progress_Phase = 2;
        }
      }break;
      case 2:   //fade in Blue
      {
        Rainbow_Progress_Index = Rainbow_Progress_Index + 1;
        Rainbow_Progress_B = Rainbow_Progress_Index; //Blue
        if(Rainbow_Progress_Index >= 255)
        {
          Rainbow_Progress_Index = 255;
          Rainbow_Progress_Phase = 3;
        }
      }break;
      case 3:   //fade out Green
      {
        Rainbow_Progress_Index = Rainbow_Progress_Index - 1;
        Rainbow_Progress_G = Rainbow_Progress_Index; //Green
        if(Rainbow_Progress_Index <= 0)
        {
          Rainbow_Progress_Index = 0;
          Rainbow_Progress_Phase = 4;
        }
      }break;
      case 4:   //fade in Red
      {
        Rainbow_Progress_Index = Rainbow_Progress_Index + 1;
        Rainbow_Progress_R = Rainbow_Progress_Index; //Red
        if(Rainbow_Progress_Index >= 255)
        {
          Rainbow_Progress_Index = 255;
          Rainbow_Progress_Phase = 5;
        }
      }break;
      case 5:   //fade out Blue
      {
        Rainbow_Progress_Index = Rainbow_Progress_Index - 1;
        Rainbow_Progress_B = Rainbow_Progress_Index; //Blue
        if(Rainbow_Progress_Index <= 0)
        {
          Rainbow_Progress_Index = 0;
          Rainbow_Progress_Phase = 0;
        }
      }break;
    }
    IO_WriteColor(Rainbow_Progress_R, Rainbow_Progress_G, Rainbow_Progress_B, false);
    Rainbow_Progress_Time_Micros = micros();
  }  
}
