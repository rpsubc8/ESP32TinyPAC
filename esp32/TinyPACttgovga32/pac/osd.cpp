#include "gbConfig.h"
#include "gbGlobals.h"
#include "hardware.h"
#include "osd.h"
#include "hardware.h"
#include "PS2Kbd.h"
//#include "gbsdlfont6x8.h"
#include "vga6bit.h"
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


#define max_gb_main_menu 7
const char * gb_main_menu[max_gb_main_menu]={
 "Reset", 
 "Speed",
 "Video Mode",
 "Video Poll",
 "Keyboard Poll",
 "Show FPS",
 "Exit"
};

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
 gb_ini_x= 0;
 gb_ini_y= 0;

 if (gb_videomode_cur != id)
 {
  if (
      ((gb_videomode_cur==2)&&(id==3))
      ||((gb_videomode_cur==3)&&(id==2))
      ||((gb_videomode_cur==4)&&(id==5))
      ||((gb_videomode_cur==5)&&(id==4))
     ) 
  {//Cambio de horizontal a vertical o alreves
   gb_videomode_cur= id;
   //gb_videomode_last= id;
   
   switch (gb_videomode_cur)
   {
    case 2: gb_vertical= 0; break; //320x350x70hz H bitluni
    case 3: gb_vertical= 1; break; //320x350x70hz V bitluni
    case 4: gb_vertical= 0; break;//400x300x56.2hz H bitluni
    case 5: gb_vertical= 1; break;//400x300x56.2hz V bitluni
   }

   gb_ini_x= 0;
   gb_ini_y= 0;

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
    break;

   case 2: //320x350x70hz H bitluni
    gb_ptrVideo_cur= VgaMode_vga_mode_320x350;
    gb_vertical= 0;
    gb_ancho= 320;
    gb_alto= 350;
    break;

   case 3: //320x350x70hz V bitluni
    gb_ptrVideo_cur= VgaMode_vga_mode_320x350;
    gb_vertical= 1;
    gb_ancho= 320;
    gb_alto= 350;    
    break;

   case 4: //400x300x56.2hz H bitluni
    gb_ptrVideo_cur= VgaMode_vga_mode_400x300;
    gb_vertical= 0;
    gb_ancho= 400;
    gb_alto= 300;    
    break;

   case 5: //400x300x56.2hz V bitluni
    gb_ptrVideo_cur= VgaMode_vga_mode_400x300;
    gb_vertical= 1;   
    gb_ancho= 400;
    gb_alto= 300;    
    break;
  }

  gb_ini_x= 0;
  gb_ini_y= 0;

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
 if (checkAndCleanKey(KEY_F1))
 {
  gb_show_osd_main_menu= 1;
  return;
 }

 if (gb_show_osd_main_menu == 1)
 {
  aSelNum = ShowTinyMenu("MAIN MENU",gb_main_menu,max_gb_main_menu,-1);
  switch (aSelNum)
  {
   case 0: ShowTinyResetMenu(); break;
   case 1: ShowTinySpeedMenu(); break;
   case 2: ShowTinyVideoModeMenu(); break;
   case 3: ShowTinyVGAPollMenu(); break;
   case 4: ShowTinyKeyboardPollMenu(); break;
   case 5: gb_show_fps= ((~gb_show_fps) & 0x01); break;
  }   
  gb_show_osd_main_menu=0; 
 }

 SDLClear();
 //SDLSetBorder(); //TRuco rapido borde color
 
 #ifdef use_lib_sound_ay8912
  gb_silence_all_channels = 0;
 #endif 
}

