#include "gbConfig.h"
#include "gbGlobals.h"
#include "hardware.h"
#include "osd.h"
#include "hardware.h"
#include "PS2Kbd.h"
//#include "gbsdlfont6x8.h"
#include "vga6bit.h"
#ifdef use_lib_sound_digital 
 #ifdef use_lib_sound_dac
  #include <driver/dac.h>
 #endif 
#endif 
//#include "dataFlash/gbpfile.h"
//#include "gbrom82s1237f.h"


//#define BLACK   0
//#define BLUE    4
//#define RED     1
//#define MAGENTA 5
//#define GREEN   2
//#define CYAN    6
//#define YELLOW  3
//#define WHITE   15

//#ifdef COLOR_3B           //       BGR 
// #define BLACK   0x08      // 0000 1000
// #define BLUE    0x0C      // 0000 1100
// #define RED     0x09      // 0000 1001
// #define MAGENTA 0x0D      // 0000 1101
// #define GREEN   0x0A      // 0000 1010
// #define CYAN    0x0E      // 0000 1110
// #define YELLOW  0x0B      // 0000 1011
// #define WHITE   0x0F      // 0000 1111
//#endif



unsigned char gb_show_osd_main_menu=0;




//#define max_gb_osd_screen 1
//const char * gb_osd_screen[max_gb_osd_screen]={
// "Pixels Left"//,
// //"Pixels Top",
// //"Color 8",
// //"Mono Blue 8",
// //"Mono Green 8",
// //"Mono Red 8",
// //"Mono Grey 8"
//};

//#define max_gb_osd_screen_values 5
//const char * gb_osd_screen_values[max_gb_osd_screen_values]={
// "0",
// "2",
// "4", 
// "8", 
// "16"
//};


#define max_gb_main_menu 8
const char * gb_main_menu[max_gb_main_menu]={
 "Reset", 
 "Speed",
 "Video Mode",
 "Video Poll",
 "Keyboard Poll",
 "Show FPS",
 "Sound",
 "Exit"
};

#ifdef use_lib_sound_dac
 #define max_gb_sound_menu 10
 const char * gb_sound_menu[max_gb_sound_menu]={
  "Digital ON",
  "Digital OFF",
  "DAC (125%)",
  "DAC (100%)",
  "DAC (75%)",
  "DAC (50%)",
  "DAC (25%)",
  "DAC (0%)",
  "DAC MedUnsigned",
  "DAC MedSigned"
 };
#else
 #define max_gb_sound_menu 2
 const char * gb_sound_menu[max_gb_sound_menu]={
  "Digital ON",
  "Digital OFF"
 };
#endif 

/*
#define max_gb_delay_cpu_menu 15
const char * gb_delay_cpu_menu[max_gb_delay_cpu_menu]={
 "Auto",
 "0",
 "1",
 "2",
 "3",
 "4",
 "5",
 "6",
 "7",
 "8",
 "9",
 "10",
 "16",
 "20",
 "40"
};
*/

#define max_gb_speed_menu 5
const char * gb_speed_menu[max_gb_speed_menu]={
 "1 (normal)",
 "2",
 "3",
 "4",
 "5 (fast)"
}; 


//#define max_gb_speed_sound_menu 7
//const char * gb_speed_sound_menu[max_gb_speed_sound_menu]={
// "0",
// "1",
// "2",
// "4",
// "8",
// "16",
// "32"
//};

//#define max_gb_value_binary_menu 2
//const char * gb_value_binary_menu[max_gb_value_binary_menu]={
// "0",
// "1"
//};


//#define max_gb_speed_videoaudio_options_menu 2
//const char * gb_speed_videoaudio_options_menu[max_gb_speed_videoaudio_options_menu]={
// "Video poll",
// "Keyboard poll"
//};

#define max_gb_speed_video_poll_menu 7
const char * gb_speed_video_poll_menu[max_gb_speed_video_poll_menu]={
 "0",
 "10 (100 FPS)",
 "16 (60 FPS)",
 "20 (50 FPS)",
 "30 (33 FPS)",
 "40 (25 FPS)",
 "50 (20 FPS)"
};

#define max_gb_speed_keyboard_poll_menu 5
const char * gb_speed_keyboard_poll_menu[max_gb_speed_keyboard_poll_menu]={
 "10",
 "20",
 "30",
 "40",
 "50"
};


#define max_gb_video_mode_menu 6
const char * gb_video_mode_menu[max_gb_video_mode_menu]={ 
 "320x240x60hz V bitluni",
 "320x240x60hz V fabgl",
 "320x350x70hz H bitluni",
 "320x350x70hz V bitluni",
 "400x300x56.2hz H bitluni",
 "400x300x56.2hz V bitluni"
};


//#define max_gb_speed_menu 5
//const char * gb_speed_menu[max_gb_speed_menu]={
// "Normal",
// "2x",
// "4x",
// "8x",
// "16x"
//};


//#define max_gb_osd_mouse_menu 2
//const char * gb_osd_mouse_menu[max_gb_osd_mouse_menu]={
// "right handed",
// "left handed"
//};

#define max_gb_reset_menu 2
const char * gb_reset_menu[max_gb_reset_menu]={
 "Soft",
 "Hard"
};


#define gb_pos_x_menu 60
#define gb_pos_y_menu 20
#define gb_osd_max_rows 10


//void jj_fast_putpixel(int x,int y,unsigned char c)
//{
// if ((x<0)||(x>319)||(y<0)||(y>199))
//  return;
//// vga.dotFast(x,y,gb_color_vga[c]);
// gb_buffer_vga[y][x^2]= gb_color_vga[c];
//}



#ifdef use_lib_keyboard_uart
 void keyboard_uart_poll()
 {
  if(Serial.available() > 0)
  {
   gb_rlen_uart = Serial.readBytes(gb_buf_uart, BUFFER_SIZE_UART);

   //Serial.print("OSD I received: ");
   gb_buf_uart[gb_rlen_uart]='\0';
   #ifdef use_lib_log_keyboard_uart
    Serial.printf("UART key tot:%d\r\nASCII:%s\r\n",gb_rlen_uart,gb_buf_uart);        
    for (unsigned short int i=0;i<gb_rlen_uart;i++)
    {
     Serial.printf("%02X ",gb_buf_uart[i]);
    }
    Serial.printf("\r\n"); 
   #endif 
  }
  else
  {
   gb_rlen_uart=0;
   gb_buf_uart[0]='\0';
  }  
 }

 unsigned char checkKey_uart(unsigned char scancode)
 {
  unsigned int contBuf=0;
  unsigned char toReturn= 0;
  //if(Serial.available() > 0)
  //{
  // gb_rlen_uart = Serial.readBytes(gb_buf_uart, BUFFER_SIZE_UART);
  //
  // Serial.print("OSD I received: ");
  // gb_buf_uart[gb_rlen_uart]='\0';
  // Serial.printf("OSD Tot:%d\nASCII:%s\n",gb_rlen_uart,gb_buf_uart);     
  // Serial.printf("\n"); 
   
   //for(contBuf= 0; contBuf < gb_rlen_uart; contBuf++)
   //Serial.printf("OSD check tot:%d\n",gb_rlen_uart);
   while (contBuf < gb_rlen_uart)
   {
    //Serial.printf("%02X ",gb_buf_uart[contBuf]);
    switch (gb_buf_uart[contBuf])
    {
     case 0x1B: //Arriba 1B 5B 41
      if ((contBuf+2) < gb_rlen_uart)
      {
       contBuf++;
       if (gb_buf_uart[contBuf] == 0x5B)
       {
        contBuf++;
        switch (gb_buf_uart[contBuf])
        {
         case 0x41: toReturn = (scancode == KEY_CURSOR_UP) ? 1 : 0; break; //arriba 1B 5B 41
         case 0x42: toReturn = (scancode == KEY_CURSOR_DOWN) ? 1 : 0; break; //abajo 1B 5B 42
         case 0x43: toReturn = (scancode == KEY_CURSOR_RIGHT) ? 1 : 0; break; //derecha 1B 5B 43
         case 0x44: toReturn = (scancode == KEY_CURSOR_LEFT) ? 1 : 0; break; //izquierda 1B 5B 44        
        }
       }
      }
      else
      {       
       toReturn = (scancode == KEY_ESC) ? 1 : 0; //ESC              
      }
      break;

     case 0x0D: case 0x0A: //0D 0A ENTER
      //if ((contBuf+1) < gb_rlen_uart)
      //{
      // contBuf++;
      // if (gb_buf_uart[contBuf] == 0x0A)
      // {
      //  toReturn = (scancode == KEY_ENTER) ? 1 : 0; //ENTER
      //  //contBuf++;
      // }
      //}
       toReturn = (scancode == KEY_ENTER) ? 1 : 0; //ENTER
      break;

    }
    contBuf++;
   }
  //}
  //Serial.printf("\n");
  return toReturn;
 } 
#endif 


//***************************************************************************
inline void jj_fast_putpixel(short int x,short int y,unsigned char c)
{
 #ifdef use_lib_tinybitluni_fast
  gb_buffer_vga[y][x^2]= gb_jj_color[c];
 #else
  #ifdef use_lib_cvbs_bitluni   
   gb_buffer_cvbs[y][x]= gb_color_cvbs[(c & 0x01)];
  #endif
 #endif 
}



//**********************************************
void SDLClear()
{
 //for (int y=0; y<200; y++)
 //{
 // for (int x=0; x<320; x++)
 // {
 //  jj_fast_putpixel(x,y,0);
 // }
 //}

 unsigned int a0= (0|gb_sync_bits);
 unsigned int a32= (a0<<24)|(a0<<16)|(a0<<8)|a0;
 for (unsigned int y=0;y<gb_alto;y++)
 {
  for (unsigned int x=0;x<(gb_ancho>>2);x++)
  {   
   gb_buffer_vga32[y][x]= a32; //gb_color_vga[0];
  }
 }
}



//*************************************************************************************
void SDLprintCharOSD(char car,int x,int y,unsigned char color,unsigned char backcolor)
{ 
// unsigned char aux = gb_sdl_font_6x8[(car-64)];
 int auxId = car << 3; //*8
 unsigned char aux;
 unsigned char auxColor;
 for (unsigned char j=0;j<8;j++)
 {
  //aux = gb_sdl_font_8x8[auxId + j];
  aux = gb_sdl_font[auxId + j];
  for (int i=0;i<8;i++)
  {
   auxColor= ((aux>>i) & 0x01);
   //SDLputpixel(surface,x+(6-i),y+j,(auxColor==1)?color:backcolor);
   jj_fast_putpixel(x+(6-i),y+j,(auxColor==1)?color:backcolor);
  }
 }
}

void SDLprintText(const char *cad,int x, int y, unsigned char color,unsigned char backcolor)
{
//SDL_Surface *surface,
// gb_sdl_font_6x8
 int auxLen= strlen(cad);
 if (auxLen>50)
  auxLen=50;
 for (int i=0;i<auxLen;i++)
 {
  SDLprintCharOSD(cad[i],x,y,color,backcolor);
  x+=7;
 }
}

void OSDMenuRowsDisplayScroll(const char **ptrValue,unsigned char currentId,unsigned char aMax)
{//Dibuja varias lineas
 for (int i=0;i<gb_osd_max_rows;i++)
  SDLprintText("                        ",gb_pos_x_menu,gb_pos_y_menu+8+(i<<3),0,0);
 
 for (int i=0;i<gb_osd_max_rows;i++)
 {
  if (currentId >= aMax)
   break;
  //SDLprintText(gb_osd_sdl_surface,ptrValue[currentId],gb_pos_x_menu,gb_pos_y_menu+8+(i<<3),((i==0)?CYAN:WHITE),((i==0)?BLUE:BLACK),1);
  SDLprintText(ptrValue[currentId],gb_pos_x_menu,gb_pos_y_menu+8+(i<<3),((i==0)?ID_COLOR_BLACK:ID_COLOR_WHITE),((i==0)?ID_COLOR_WHITE:ID_COLOR_BLACK));
  currentId++;
 }     
}

//Maximo 256 elementos
unsigned char ShowTinyMenu(const char *cadTitle,const char **ptrValue,unsigned char aMax,short int aSel)
{
 unsigned char aReturn=0;
 unsigned char salir=0;
 unsigned int curTime_keyboard;
 unsigned int curTime_keyboard_before;

 #ifdef use_lib_keyboard_uart
  unsigned int curTime_keyboard_uart;
  unsigned int curTime_keyboard_before_uart;
  curTime_keyboard_uart = curTime_keyboard_before_uart= millis();
 #endif
 
 curTime_keyboard= curTime_keyboard_before= millis();

 SDLClear();
 SDLprintText("Pacman Arcade by Ackerman",gb_pos_x_menu-(4<<3),gb_pos_y_menu-16,ID_COLOR_WHITE,ID_COLOR_BLACK);
 //for (int i=0;i<20;i++) 
 for (int i=0;i<14;i++) 
  SDLprintCharOSD(' ',gb_pos_x_menu+(i<<3),gb_pos_y_menu,ID_COLOR_BLACK,ID_COLOR_WHITE);
 SDLprintText(cadTitle,gb_pos_x_menu,gb_pos_y_menu,ID_COLOR_BLACK,ID_COLOR_WHITE);

 aReturn = (aSel!=-1)?aSel:0;
 OSDMenuRowsDisplayScroll(ptrValue,aReturn,aMax);
 
 while (salir == 0)
 {
  //case SDLK_UP:
  curTime_keyboard = millis();
  if ((curTime_keyboard - curTime_keyboard_before) >= gb_keyboard_cur_poll_ms)
  {
   curTime_keyboard_before= curTime_keyboard;

   #ifdef use_lib_keyboard_uart
    curTime_keyboard_uart= curTime_keyboard;
    if ((curTime_keyboard_uart - curTime_keyboard_before_uart) >= gb_current_ms_poll_keyboard_uart)
    {
     curTime_keyboard_before_uart = curTime_keyboard_uart;
     keyboard_uart_poll();
    
     if (checkKey_uart(KEY_CURSOR_LEFT)==1)
     {
      if (aReturn>10) aReturn-=10;
      OSDMenuRowsDisplayScroll(ptrValue,aReturn,aMax);
     }
     if (checkKey_uart(KEY_CURSOR_RIGHT)==1)
     {
      if (aReturn<(aMax-10)) aReturn+=10;
      OSDMenuRowsDisplayScroll(ptrValue,aReturn,aMax);       
     }  
     if (checkKey_uart(KEY_CURSOR_UP)==1)
     {
      if (aReturn>0) aReturn--;
      OSDMenuRowsDisplayScroll(ptrValue,aReturn,aMax);
     }
     if (checkKey_uart(KEY_CURSOR_DOWN)==1)
     {
      if (aReturn < (aMax-1)) aReturn++;
      OSDMenuRowsDisplayScroll(ptrValue,aReturn,aMax);
     }
     if (checkKey_uart(KEY_ENTER)==1)
     {
      salir= 1;
     }
     if (checkKey_uart(KEY_ESC))
     {
      salir=1; aReturn= 255;    
     }
    }
   #endif


   if (checkAndCleanKey(KEY_F1))
   {
   }
   if (checkAndCleanKey(KEY_CURSOR_LEFT))
   {
    if (aReturn>10) aReturn-=10;
    OSDMenuRowsDisplayScroll(ptrValue,aReturn,aMax);       
   }
   if (checkAndCleanKey(KEY_CURSOR_RIGHT))
   {
    if (aReturn<(aMax-10)) aReturn+=10;
    OSDMenuRowsDisplayScroll(ptrValue,aReturn,aMax);       
   }
          
   if (checkAndCleanKey(KEY_CURSOR_UP))
   {
    if (aReturn>0) aReturn--;
    OSDMenuRowsDisplayScroll(ptrValue,aReturn,aMax);
   }
   if (checkAndCleanKey(KEY_CURSOR_DOWN))
   {
    if (aReturn < (aMax-1)) aReturn++;
    OSDMenuRowsDisplayScroll(ptrValue,aReturn,aMax);
   }
   if (checkAndCleanKey(KEY_ENTER))
   {
    salir= 1;
   }
   //case SDLK_KP_ENTER: case SDLK_RETURN: salir= 1;break;
   if (checkAndCleanKey(KEY_ESC))
   {
    salir=1; aReturn= 255;    
   }
   //case SDLK_ESCAPE: salir=1; aReturn= 255; break;
   //default: break;             
  }
 } 
 gb_show_osd_main_menu= 0;
 return aReturn;
}


//Menu resetear
void ShowTinyResetMenu()
{
 unsigned char aSelNum;
 aSelNum= ShowTinyMenu("Reset",gb_reset_menu,max_gb_reset_menu,-1);   
 if (aSelNum==255)
 {
  return;
 }
 
 if (aSelNum == 1)
 {
  ESP.restart();
 }
 else
 {
  gb_reset= 1;
 } 
}


//Menu velocidad emulador
void ShowTinySpeedMenu()
{
 //1 (normal)
 //2
 //3
 //4
 //5 (fast)
 unsigned char aSelNum;
 aSelNum = ShowTinyMenu("Speed",gb_speed_menu,max_gb_speed_menu,-1);
 if (aSelNum == 255)
 {
  return;
 } 
 gb_speed= aSelNum+1;
}

void ShowTinyKeyboardPollMenu()
{
 unsigned char aSelNum;
 aSelNum = ShowTinyMenu("Poll ms Keyboard",gb_speed_keyboard_poll_menu,max_gb_speed_keyboard_poll_menu,-1);
 if (aSelNum==255)
 {
   return;
 }
 switch (aSelNum)
 {
  case 0: gb_keyboard_cur_poll_ms= 10; break;
  case 1: gb_keyboard_cur_poll_ms= 20; break;
  case 2: gb_keyboard_cur_poll_ms= 30; break;
  case 3: gb_keyboard_cur_poll_ms= 40; break;
  case 4: gb_keyboard_cur_poll_ms= 50; break;
 }
}


  void IRAM_ATTR onTimerSoundDigital()
  {
   //Para digital
   if (gb_spk_data != gb_spk_data_before)
   {
    //digitalWrite(SPEAKER_PIN, gb_spk_data);
    //GPIO 0..31
    if (gb_spk_data)
    {
     GPIO.out_w1ts= 1<<25; //1<<25;
    }
    else
    {
     GPIO.out_w1tc = 1<<25;
    }
    gb_spk_data_before= gb_spk_data;
   }
  
   if (gb_sillence_all==1)
   {
    gb_spk_data= 0;
    return;
   }

   //digitalWrite(SPEAKER_PIN, !digitalRead(SPEAKER_PIN));
   for (unsigned char j=0;j<3;j++)
   {
    if (gb_ct[j] >= (gb_ct_Pulse[j]-1))
    {
     gb_flip[j]= (~gb_flip[j]) & 0x01;
     gb_ct[j]= 0;
    }
    gb_ct[j]++;
   }

   if ((gb_flip[0]==1)&&(gbVolMixer_now[0]>0))
   {
    gb_spk_data= 1;
   }
   else
   {
    if ((gb_flip[1]==1)&&(gbVolMixer_now[1]>0))
    {
     gb_spk_data= 1;
    }
    else
    {
     gb_spk_data= ((gb_flip[2]==1)&&(gbVolMixer_now[2]>0)) ? 1 : 0;
    }
   }
  } 


 #ifdef use_lib_sound_dac
  
  static DRAM_ATTR unsigned char gb_sin[360]={
   64,66,68,70,72,75,77,79,81,83,85,87,90,92,94,95,
   97,99,101,103,105,106,108,110,111,113,114,115,117,118,119,120,
   121,122,123,124,124,125,126,126,127,127,127,127,127,128,127,127,
   127,127,127,126,126,125,124,124,123,122,121,120,119,118,117,115,
   114,113,111,110,108,106,105,103,101,99,97,95,94,92,90,87,
   85,83,81,79,77,75,72,70,68,66,64,61,59,57,55,52,
   50,48,46,44,42,40,37,35,33,32,30,28,26,24,22,21,
   19,17,16,14,13,12,10,9,8,7,6,5,4,3,3,2,
   1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,2,
   3,3,4,5,6,7,8,9,10,12,13,14,16,17,19,21,
   22,24,26,28,30,31,33,35,37,40,42,44,46,48,50,52,
   55,57,59,61,63,66,68,70,72,75,77,79,81,83,85,87,
   90,92,94,96,97,99,101,103,105,106,108,110,111,113,114,115,
   117,118,119,120,121,122,123,124,124,125,126,126,127,127,127,127,
   127,128,127,127,127,127,127,126,126,125,124,124,123,122,121,120,
   119,118,117,115,114,113,111,110,108,106,105,103,101,99,97,95,
   94,92,90,87,85,83,81,79,77,75,72,70,68,66,64,61,
   59,57,55,52,50,48,46,44,42,40,37,35,33,32,30,28,
   26,24,22,21,19,17,16,14,13,12,10,9,8,7,6,5,
   4,3,3,2,1,1,0,0,0,0,0,0,0,0,0,0,
   0,1,1,2,3,3,4,5,6,7,8,9,10,12,13,14,
   16,17,19,21,22,24,26,28,30,32,33,35,37,40,42,44,
   46,48,50,52,55,57,59,61
  };
  //260 grados Multiplicado por 64 y previo sumado 1 sin signo
  
  /*
  static DRAM_ATTR unsigned char gb_sin[256]={
   0x40,0x2E,0x1D,0x06,0x00,0x05,0x0E,0x1C,0x3E,0x50,0x6E,0x79,0x7E,0x7B,0x72,0x55,
   0x43,0x31,0x12,0x07,0x00,0x04,0x0C,0x29,0x3A,0x5D,0x6C,0x77,0x7F,0x7C,0x67,0x58,
   0x34,0x23,0x14,0x02,0x00,0x0A,0x16,0x26,0x49,0x5A,0x75,0x7D,0x7F,0x76,0x6A,0x4A,
   0x38,0x26,0x0B,0x03,0x02,0x08,0x14,0x34,0x46,0x67,0x73,0x7C,0x7E,0x77,0x5E,0x4D,
   0x29,0x19,0x0D,0x00,0x01,0x11,0x20,0x30,0x54,0x64,0x7B,0x7F,0x7F,0x6F,0x61,0x3E,
   0x2D,0x1C,0x05,0x00,0x05,0x0F,0x1D,0x3F,0x51,0x6F,0x79,0x7F,0x7A,0x71,0x54,0x42,
   0x1F,0x11,0x07,0x00,0x04,0x1A,0x2A,0x3C,0x5E,0x6D,0x7E,0x7F,0x7C,0x66,0x57,0x33,
   0x22,0x13,0x01,0x00,0x0B,0x17,0x27,0x4A,0x5B,0x76,0x7D,0x7F,0x75,0x69,0x49,0x36,
   0x16,0x0A,0x02,0x02,0x09,0x24,0x35,0x47,0x68,0x74,0x7F,0x7E,0x77,0x5D,0x4C,0x28,
   0x19,0x0C,0x00,0x01,0x12,0x21,0x31,0x55,0x65,0x7B,0x7F,0x7E,0x6E,0x60,0x3D,0x2B,
   0x0E,0x05,0x00,0x06,0x10,0x2E,0x40,0x52,0x70,0x7A,0x7F,0x7A,0x70,0x52,0x41,0x1E,
   0x10,0x06,0x00,0x04,0x1B,0x2B,0x3D,0x5F,0x6E,0x7E,0x7F,0x7B,0x66,0x56,0x32,0x21,
   0x08,0x01,0x00,0x0C,0x18,0x39,0x4B,0x5C,0x77,0x7D,0x7C,0x74,0x68,0x47,0x35,0x15,
   0x09,0x02,0x02,0x0A,0x25,0x36,0x48,0x69,0x75,0x7F,0x7D,0x76,0x5C,0x4B,0x27,0x18,
   0x03,0x00,0x01,0x13,0x22,0x45,0x56,0x66,0x7B,0x7F,0x78,0x6D,0x5F,0x3C,0x2A,0x0D,
   0x04,0x00,0x06,0x11,0x2F,0x41,0x53,0x71,0x7A,0x7F,0x79,0x70,0x51,0x40,0x1D,0x0F
  }; //Multiplicado por 64 y previo sumado 1 sin signo
  */

/*
  const static DRAM_ATTR signed char gb_sin[256]={
   0,    3,    6,    9,   12,   16,   19,   22,   25,   28,   31,   34,   37,   40,   43,   46,
  49,   51,   54,   57,   60,   63,   65,   68,   71,   73,   76,   78,   81,   83,   85,   88,
  90,   92,   94,   96,   98,  100,  102,  104,  106,  107,  109,  111,  112,  113,  115,  116,
 117,  118,  120,  121,  122,  122,  123,  124,  125,  125,  126,  126,  126,  127,  127,  127,
 127,  127,  127,  127,  126,  126,  126,  125,  125,  124,  123,  122,  122,  121,  120,  118,
 117,  116,  115,  113,  112,  111,  109,  107,  106,  104,  102,  100,   98,   96,   94,   92,
  90,   88,   85,   83,   81,   78,   76,   73,   71,   68,   65,   63,   60,   57,   54,   51,
  49,   46,   43,   40,   37,   34,   31,   28,   25,   22,   19,   16,   12,    9,    6,    3,
   0,   -3,   -6,   -9,  -12,  -16,  -19,  -22,  -25,  -28,  -31,  -34,  -37,  -40,  -43,  -46,
 -49,  -51,  -54,  -57,  -60,  -63,  -65,  -68,  -71,  -73,  -76,  -78,  -81,  -83,  -85,  -88,
 -90,  -92,  -94,  -96,  -98, -100, -102, -104, -106, -107, -109, -111, -112, -113, -115, -116,
-117, -118, -120, -121, -122, -122, -123, -124, -125, -125, -126, -126, -126, -127, -127, -127,
-127, -127, -127, -127, -126, -126, -126, -125, -125, -124, -123, -122, -122, -121, -120, -118,
-117, -116, -115, -113, -112, -111, -109, -107, -106, -104, -102, -100,  -98,  -96,  -94,  -92,
 -90,  -88,  -85,  -83,  -81,  -78,  -76,  -73,  -71,  -68,  -65,  -63,  -60,  -57,  -54,  -51,
 -49,  -46,  -43,  -40,  -37,  -34,  -31,  -28,  -25,  -22,  -19,  -16,  -12,   -9,   -6,   -3,
 };
 */

  /*
  const static DRAM_ATTR unsigned char gb_sin[256]={
   150,108,69,14,1,12,34,66,146,188,3,28,41,33,11,199,
   157,115,42,17,0,9,29,96,138,220,254,24,43,35,243,206,
   123,83,48,5,0,25,53,89,172,213,20,37,43,20,249,173,
   131,90,25,7,4,20,47,122,164,242,15,35,40,24,221,181,
   98,60,30,0,2,41,75,114,198,235,32,43,41,4,228,147,
   105,67,13,1,13,36,68,148,190,5,29,41,32,10,196,155,
   74,40,16,0,10,61,99,140,222,0,40,43,34,241,204,120,
   81,46,4,0,26,55,91,175,215,21,38,43,19,247,171,128,
   52,24,6,5,22,84,124,167,244,17,43,39,23,218,179,95,
   58,29,0,3,43,77,117,200,238,33,43,41,3,225,144,103,
   33,12,1,14,38,109,151,193,7,30,42,30,8,194,152,71,
   39,15,0,11,63,101,143,224,2,41,43,34,239,202,118,78,
   19,3,0,28,57,135,177,217,22,39,36,18,245,168,126,50,
   23,5,6,23,86,127,169,246,18,43,38,22,216,176,93,56,
   8,0,4,45,79,161,203,240,34,43,26,1,223,142,100,32,
   10,0,15,40,111,153,195,9,31,42,29,6,191,150,69,37
  };//Multiplicado por 150 y previo sumado 1 sin signo
  */




  volatile unsigned int gb_contador_muestra=0;
  volatile unsigned char dentro=0;

  void IRAM_ATTR onTimerSoundDAC()
  {
   //Para DAC   
   unsigned int auxByte;
   unsigned int media=0;
   unsigned int valor;

   if (dentro>0)
    return;

   dentro++;   
  
   if (gb_spk_data != gb_spk_data_before)
   {
    #ifdef SPEAKER_PIN == 25
     dac_output_voltage(DAC_CHANNEL_1, gb_spk_data);
    #else 
     dac_output_voltage(DAC_CHANNEL_2, gb_spk_data);
    #endif 
    gb_spk_data_before= gb_spk_data;
   }
  
   gb_contador_muestra++;    

   if (gb_sillence_all==1)
   {
    gb_spk_data= 0;
    dentro=0;
    return;
   }
  

   for (unsigned char i=0;i<3;i++)
   {
    //Tengo que multiplicar por 6, que es 4+2 aproximado a 2xPI
    if (gbVolMixer_now[i] == 0)
    {
     valor= 0;
    }
    else
    {
     /* 
     //BEGIN sale perfecto con seno double con osciloscopio  
     double aux= (sin(((2 * 3.1415926535 * gbFrecMixer_now[i] * gb_contador_muestra)/ (double)8000.0)))+1; //funciona con double, no vale con float
     valor= aux*64;          
     //Si multiplicamos por 6 en lugar de (2* 3.1415926535) y dividimos por 8192, 100 Hz sale 94, sino 100 Hz exacos
     //END sale perfecto con seno double
     */


     auxByte= (180 * gbFrecMixer_now[i] * gb_contador_muestra)>>13; //DIV 8192
     valor= gb_sin[(auxByte & 360)];

     /*
     auxByte= (gbVolMixer_now[i] * gbFrecMixer_now[i] * gb_contador_muestra)<<2; //x4    
     //auxByte= (gbVolMixer_now[i] * gbFrecMixer_now[i] * gb_contador_muestra * 3); //x3    
     auxByte= (auxByte + (auxByte<<1)) >>13; //DIV 8192 aproximado 8000 x2 DIV 8192
     valor= gb_sin[(auxByte & 0xFF)]; //MOD 256         
     //if (gb_sound_signed == 1){
     // if (valor<64){
     //  valor= (64-valor);
     // }
     //}
     */
     
          
    }
    //media= (media + valor)>>1; //Media aproximada   
    media= (i==0) ? valor : (media+valor)>>1; //Con esto sale exacto 100 Hz, 250 Hz y 500 Hz con el calculo sin double
   } 

   media= (media & 0xFF);
   switch (gb_dac_vol)
   {
    case 1: media= media>>1; break; //25%
    //case 2: media= media; break; //50%
    case 3: media= (media>127) ? 255 : (media<<1); break;//75%     
    case 4: media= (media>63) ? 255 : (media<<2); break; //100%
    case 5: media= (media>31) ? 255 : (media<<3); break; //125%
   }
   gb_spk_data= media;
   dentro=0;
  }  
 #else
 #endif


void ShowTinySoundMenu()
{  
 //Digital ON
 //Digital OFF
 //DAC (125%)
 //DAC (100%)
 //DAC (75%)
 //DAC (50%)
 //DAC (25%)
 //DAC (0%) 
 //DAC MedUnsigned 
 //DAC MedSigned
 unsigned char aSelNum;
 unsigned char modoDigi=0;
 unsigned char modoDAC=0;
 aSelNum = ShowTinyMenu("Sound Options",gb_sound_menu,max_gb_sound_menu,-1);
 if (aSelNum==255)
 {
   return;
 }

 #ifdef use_lib_sound_dac 
  switch (aSelNum)
  {
   case 0: gb_sillence_all= 0; gb_mute= 0; modoDigi= 1; break;
   case 1: gb_sillence_all= 1; gb_mute= 1;  modoDigi= 1; break;
   case 2: gb_dac_vol= 5; gb_sillence_all= 0; gb_mute= 0; modoDAC= 1; break;
   case 3: gb_dac_vol= 4; gb_sillence_all= 0; gb_mute= 0; modoDAC= 1; break;
   case 4: gb_dac_vol= 3; gb_sillence_all= 0; gb_mute= 0; modoDAC= 1; break;
   case 5: gb_dac_vol= 2; gb_sillence_all= 0; gb_mute= 0; modoDAC= 1; break;
   case 6: gb_dac_vol= 1; gb_sillence_all= 0; gb_mute= 0; modoDAC= 1; break;
   case 7: gb_sillence_all= 1; gb_mute= 1; modoDAC= 1; break;
   case 8: 
    //Le he dado a unsigned y estaba en signed
    if (modoDAC==0){
     modoDAC= 1;
    }
    if (gb_sound_signed==1)
    {
     gb_sound_signed= 0; 
     for (unsigned short int i=0;i<256;i++)
     {
      if (gb_sin[i]<64){
       gb_sin[i]= 64-gb_sin[i];
      }
     }
     //gb_sound_signed= ((~gb_sound_signed) & 0x01);
    }
    break;    
   case 9:
    //Le he dado a signo y estaba en unsigned
    if (modoDAC==0){
     modoDAC= 1;
    }
    if (gb_sound_signed==0)
    {
     gb_sound_signed= 1;
     for (unsigned short int i=0;i<256;i++)
     {
      if (gb_sin[i]<64){
       gb_sin[i]= 64-gb_sin[i];
      }
     }    
     //gb_sound_signed= ((~gb_sound_signed) & 0x01);
    }

    break;
  }

  if (modoDigi==1)
  {
   gb_use_sound_digital= 1;
   timerAlarmDisable(gb_timerSound);   
   delay(100);       
   timerDetachInterrupt(gb_timerSound);
   delay(100);
   #ifdef SPEAKER_PIN == 25
    dac_output_disable(DAC_CHANNEL_1);
   #else 
    dac_output_disable(DAC_CHANNEL_2);
   #endif 

   pinMode(SPEAKER_PIN, OUTPUT);
   timerAttachInterrupt(gb_timerSound, &onTimerSoundDigital, true);
   timerAlarmWrite(gb_timerSound, 125, true); //1000000 1 segundo  125 es 8000 hz
   timerAlarmEnable(gb_timerSound);  
  }
  else
  {
   if (modoDAC==1)
   {
    gb_use_sound_digital= 0;
    timerAlarmDisable(gb_timerSound);   
    delay(100);       
    timerDetachInterrupt(gb_timerSound);
    delay(100);
    
    #ifdef SPEAKER_PIN == 25
     dac_output_enable(DAC_CHANNEL_1);
    #else
     dac_output_enable(DAC_CHANNEL_2);
    #endif 
    timerAttachInterrupt(gb_timerSound, &onTimerSoundDAC, true);
    timerAlarmWrite(gb_timerSound, 125, true); //1000000 1 segundo  125 es 8000 hz
    timerAlarmEnable(gb_timerSound);  
   }
  }
 #else
  //Digital solo tiene
  //Digital ON
  //Digital OFF
  gb_use_sound_digital= 1;
  switch (aSelNum)
  {
   case 0: gb_sillence_all= 0; gb_mute= 0; break;
   case 1: gb_sillence_all= 1; gb_mute= 1; break;
  }
 #endif
}


//***************************
void SetVideoMode(unsigned char id, unsigned char aVertical)
{
 unsigned int a0= (0|gb_sync_bits);
 unsigned int a32= (a0<<24)|(a0<<16)|(a0<8)|a0; 

 //0 320x240x60hz V bitluni
 //1 320x240x60hz V fabgl
 //2 320x350x70hz H bitluni
 //3 320x350x70hz V bitluni
 //4 400x300x56.2hz H bitluni
 //5 400x300x56.2hz V bitluni
 //gb_ini_x= 0;
 //gb_ini_y= 0;

 if (gb_videomode_cur != id)
 {
  if (
      ((gb_videomode_cur==2)&&(id==3))
      ||((gb_videomode_cur==3)&&(id==2))
      ||((gb_videomode_cur==4)&&(id==5))
      ||((gb_videomode_cur==5)&&(id==4))
     ) 
  {//Cambio de horizontal a vertical o alreves

   //gb_ini_x= 0;
   //gb_ini_y= 0;  

   gb_videomode_cur= id;
   //gb_videomode_last= id;
   
   switch (gb_videomode_cur)
   {
    case 2: 
     gb_vertical= 0;
     gb_ini_x=11; //(320-224)=96/2/4=12
     gb_ini_y=30; //(350-288)=62/2=31
     break; //320x350x70hz H bitluni
    case 3: 
     gb_vertical= 1; 
     gb_ini_x=3; //(320-288)=32/2/4=4
     gb_ini_y=62; //(350-224)=126/2=63
     break; //320x350x70hz V bitluni
    case 4: 
     gb_vertical= 0; 
     gb_ini_x=21; //(400-224)=176/2/4=22
     gb_ini_y=5; //(300-288)=12/2=6
     break;//400x300x56.2hz H bitluni
    case 5: 
     gb_vertical= 1; 
     gb_ini_x=13; //(400-288)=112/2/4=14
     gb_ini_y=37; //(300-224)=76/2=38
     break;//400x300x56.2hz V bitluni
   }

   //gb_ini_x= 0;
   //gb_ini_y= 0;

   return;
  }

  gb_videomode_cur= id;
  //gb_videomode_last= id;

  SetVideoInterrupt(0);
  delay(100);
  vga_free();
  delay(100);

  unsigned char usepllcteforce=0;
  unsigned int p0=0;
  unsigned int p1=0;
  unsigned int p2=0;
  unsigned int p3=0;    

  Serial.printf("id:%d\r\n",gb_videomode_cur);
  switch (gb_videomode_cur)
  {
   case 0: //320x240x60hz V bitluni
    gb_ptrVideo_cur= VgaMode_vga_mode_320x240;
    gb_vertical= 1;
    gb_ancho= 320;
    gb_alto= 240;
    //224x288
    gb_ini_x=3; //(320-288)=32/2/4=4
    gb_ini_y=7; //(240-224)=16/2=8
    break;

   case 1: //320x240x60hz V fabgl
    gb_ptrVideo_cur= VgaMode_vga_mode_320x240;    
    gb_vertical= 1;
    usepllcteforce= 1;
    p0= 0x000A;
    p1= 0x0057;
    p2= 0x0007;
    p3= 0x0007;
    gb_ancho= 320;
    gb_alto= 240;
    //224x288
    gb_ini_x=3; //(320-288)=32/2/4=4
    gb_ini_y=7; //(240-224)=16/2=8
    break;

   case 2: //320x350x70hz H bitluni
    gb_ptrVideo_cur= VgaMode_vga_mode_320x350;
    gb_vertical= 0;
    gb_ancho= 320;
    gb_alto= 350;
    //224x288
    gb_ini_x=11; //(320-224)=96/2/4=12
    gb_ini_y=30; //(350-288)=62/2=31
    break;    

   case 3: //320x350x70hz V bitluni
    gb_ptrVideo_cur= VgaMode_vga_mode_320x350;
    gb_vertical= 1;
    gb_ancho= 320;
    gb_alto= 350;
    //224x288
    gb_ini_x=3; //(320-288)=32/2/4=4
    gb_ini_y=62; //(350-224)=126/2=63
    break;

   case 4: //400x300x56.2hz H bitluni
    gb_ptrVideo_cur= VgaMode_vga_mode_400x300;
    gb_vertical= 0;
    gb_ancho= 400;
    gb_alto= 300;    
    gb_ini_x=21; //(400-224)=176/2/4=22
    gb_ini_y=5; //(300-288)=12/2=6
    break;

   case 5: //400x300x56.2hz V bitluni
    gb_ptrVideo_cur= VgaMode_vga_mode_400x300;
    gb_vertical= 1;   
    gb_ancho= 400;
    gb_alto= 300;    
    gb_ini_x=13; //(400-288)=112/2/4=14
    gb_ini_y=37; //(300-224)=76/2=38
    break;
  }

  //gb_ini_x= 0;
  //gb_ini_y= 0;

  Serial.printf("w:%d h:%d v:%d pll:%d\r\n",gb_ancho,gb_alto,gb_vertical,usepllcteforce);
  vga_init(pin_config,gb_ptrVideo_cur,false,usepllcteforce,p0,p1,p2,p3);
  SetVideoInterrupt(1);
  gb_sync_bits= vga_get_sync_bits();
  gb_buffer_vga = vga_get_framebuffer();
  gb_buffer_vga32=(unsigned int **)gb_buffer_vga;

  unsigned int a0= (0|gb_sync_bits);
  unsigned int a32= (a0<<24)|(a0<<16)|(a0<<8)|a0;
  for (unsigned int y=0;y<gb_alto;y++)
  {
   for (unsigned int x=0;x<(gb_ancho>>2);x++)
   {  
    gb_buffer_vga32[y][x]= a32; //gb_color_vga[0];
   }
  } 

  for (unsigned short int i=0;i<32;i++)
  {
   //unsigned char data= p->color_rom[i];
   unsigned char data= gb_ptr_rom_82s1237f[i];
   unsigned char r= (data>>1)&0x03;
   unsigned char g= (data>>4)&0x03;
   unsigned char b= (data>>6)&0x03;
   //gb_jj_color[i]= (r<<4)|(g<<2)|(b&0x03);
   //gb_jj_color[i]= (b<<4)|(g<<2)|(r&0x03); //invierto BBGGRR
   gb_jj_color[i]= (b<<4)|(g<<2)|(r&0x03)|gb_sync_bits; //invierto BBGGRR revisar
  }  

  #ifdef use_lib_log_serial  
   //Serial.printf("Set Video %d\r\n",aSelNum);     
   Serial.printf("RAM free %d\r\n", ESP.getFreeHeap()); 
  #endif           

 }

/*
 if (gb_videomode_cur == id)
 {//Mismo modo de video. Miro solo vertical horizontal  
  if (gb_videomode_cur=0)
  {//Modo 320x240 solo puede ser vertical
   gb_vertical= 1;
  }
  else
  {
   gb_vertical= aVertical;
  }
 }
 else
 {
  gb_videomode_last= gb_videomode_cur;
  gb_videomode_cur= id;
  if (gb_videomode_cur=0)
  {//Modo 320x240 solo puede ser vertical
   gb_vertical= 1;
  }
  else
  {
   gb_vertical= aVertical;
  }
 }

 for (unsigned int y=0;y<gb_alto;y++)
 {
  for (unsigned int x=0;x<(gb_ancho>>2);x++)
  {   
   gb_buffer_vga[y][x]= a32;
  }
 }
 */


 //relacion aspecto
 //320x200
 //(320-224)= 96 / 2
 //320x350
 //(320-224)= 96/2=48
 //(350-288)= 62/2=31
 /*
 if (gb_vertical == 0)
 {
  gb_ini_x= ((gb_ancho-224)/2)-8;
  gb_ini_y= ((gb_alto-288)/2)-8;
 }
 else
 {
  gb_ini_x= (((gb_ancho-288)-1)/2)-8;
  gb_ini_y= (((gb_alto-224)-1)/2)-8;
 }

 gb_ini_x= gb_ini_y=0;
 */

/*
 //vga_init(pin_config,VgaMode_vga_mode_400x300,false); //400x300 Funciona bien 
 vga_init(pin_config,VgaMode_vga_mode_320x350,false); //320x350 Funciona bien 

 gb_sync_bits= vga_get_sync_bits();
 gb_buffer_vga = vga_get_framebuffer();
 gb_buffer_vga32=(unsigned int **)gb_buffer_vga;  
 */
}

void ShowTinyVideoModeMenu()
{
 //0 320x240x60hz V bitluni
 //1 320x240x60hz V fabgl
 //2 320x350x70hz H bitluni
 //3 320x350x70hz V bitluni
 //4 400x300x56.2hz H bitluni
 //5 400x300x56.2hz V bitluni  
 
 //Para SetVideoMode
 //0 320x240
 //1 320x350
 //2 400x300
 unsigned char aSelNum;
 aSelNum = ShowTinyMenu("Video Mode",gb_video_mode_menu,max_gb_video_mode_menu,-1);
 if (aSelNum==255)
 {
   return;
 }
 switch (aSelNum)
 {
  case 0: SetVideoMode(0,1); break; //320x240x60hz bitluni Vertical       
  case 1: SetVideoMode(1,1); break; //320x240x60hz fabgl Vertical   
  case 2: SetVideoMode(2,0); break; //320x350x70hz bitluni Horizontal
  case 3: SetVideoMode(3,1); break; //320x350x70hz bitluni Vertical   
  case 4: SetVideoMode(4,0); break; //400x300x56.2hz fabgl Horizontal   
  case 5: SetVideoMode(5,1); break; //400x300x56.2hz fabgl Vertical   
 }

/*
 if (gb_vertical == 0)
 {
  gb_ini_x= ((gb_ancho-224)/2)-8;
  gb_ini_y= ((gb_alto-288)/2)-8;
 }
 else
 {
  gb_ini_x= (((gb_ancho-288)-1)/2)-8;
  gb_ini_y= (((gb_alto-224)-1)/2)-8;
 }
*/
/*
 gb_ini_x= gb_ini_y= 0;


 unsigned char aux= 0 | gb_sync_bits;
 for (unsigned int y=0;y<gb_alto;y++)
 {
  for (unsigned int x=0;x<gb_ancho;x++)
  {
   gb_buffer_vga[y][x^2]= aux;
  }
 } 
 */
}

void ShowTinyVGAPollMenu()
{
 //0
 //10 (100 FPS)
 //16 (60 FPS)
 //20 (50 FPS)
 //30 (33 FPS)
 //40 (25 FPS)
 //50 (20 FPS) 
 unsigned char aSelNum;
 aSelNum = ShowTinyMenu("Poll ms VGA",gb_speed_video_poll_menu,max_gb_speed_video_poll_menu,-1);
 if (aSelNum==255)
 {
   return;
 }

 switch(aSelNum)
 {
  case 0: gb_vga_cur_poll_ms= 1; break;
  case 1: gb_vga_cur_poll_ms= 10; break;
  case 2: gb_vga_cur_poll_ms= 16; break;
  case 3: gb_vga_cur_poll_ms= 20; break;
  case 4: gb_vga_cur_poll_ms= 30; break;
  case 5: gb_vga_cur_poll_ms= 40; break;
  case 6: gb_vga_cur_poll_ms= 50; break;
 }
 
}





//*******************************************
void SDLActivarOSDMainMenu()
{     
 gb_show_osd_main_menu= 1;   
}



//Very small tiny osd
void do_tinyOSD() 
{
 unsigned char aSelNum; 
 unsigned char vol[3];

 //Serial.printf("do_tinyOSD\r\n");
 if (checkAndCleanKey(KEY_F1))
 {
  gb_show_osd_main_menu= 1;
 }

 if (gb_show_osd_main_menu == 1)
 {
  for (unsigned char i=0;i<3;i++)
  {
   vol[i]= gbVolMixer_now[i];
   gbVolMixer_now[i]= 0;
  }

  
  gb_sillence_all=1; //silencio= 1; //silencio  

  aSelNum = ShowTinyMenu("MAIN MENU",gb_main_menu,max_gb_main_menu,-1);
  switch (aSelNum)
  {
   case 0: ShowTinyResetMenu(); break;
   case 1: ShowTinySpeedMenu(); break;
   case 2: ShowTinyVideoModeMenu(); break;
   case 3: ShowTinyVGAPollMenu(); break;
   case 4: ShowTinyKeyboardPollMenu(); break;
   case 5: gb_show_fps= ((~gb_show_fps) & 0x01); break;
   case 6: ShowTinySoundMenu(); break;
  }   
  gb_show_osd_main_menu=0; 
 

  SDLClear();
  //SDLSetBorder(); //TRuco rapido borde color 
  for (unsigned char i=0;i<3;i++)
  {
   gbVolMixer_now[i]= vol[i];  
  }
  
  gb_sillence_all= (gb_mute == 1) ? 1 : 0;  
 
  //#ifdef use_lib_sound_ay8912
  // gb_silence_all_channels = 0;
  //#endif 
 }
}
