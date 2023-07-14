//Port Arcade Pacman SDL to ESP32 by ackerman
//Compatible TTGO VGA32 v1.x (1.0, 1.1, 1.2, 1.4)
//Only SRAM (not PSRAM required)
//Tiny Bitluni 0.3.3 mod C fix 8 colors (Ricardo Massaro)
//Visual Studio 1.66.1 PLATFORMIO 2.4.3 Espressif32 v3.5.0(python 3.6)
//Arduino IDE 1.8.11 Espressif System 1.0.6
//Video mode 64 colors (change video mode on the fly)
// - 320x240 Vertical
// - 320x350 Horizontal and Vertical
// - 400x300 Horizontal and Vertical
//Sound DAC (GPIO 25,26) ISR routine
//Sound Digital (GPIO 25 digital) ISR routine
//Keyboard PS/2
//OSD F1 Key
// Left, right, up, down
// 5 coin (slot 1)
// 4 coin (slot 2)
// 1 start (1p)
// 2 start (2p)
// T test board
// P pause
//Original author PC x86 SDL: superzazu  France  SDL 1
//https://github.com/superzazu/pac
//

#include "gbConfig.h"
#include "PS2Kbd.h"
#include "hardware.h"
#include "PS2KeyCode.h"
#include "gbGlobals.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
//#include <time.h> //no lo necesito

#include "paccpp.h"
#include "osd.h"

#include <Arduino.h>

#include "hardware.h"
#include "driver/timer.h"
#include "soc/timer_group_struct.h"

#include "gbsdlfont6x8.h"


#ifdef use_lib_sound_digital 
 #ifdef use_lib_sound_dac
  #include <driver/dac.h>
 #endif 
 volatile unsigned char gbVolMixer_now[3]={0,0,0};;
 volatile unsigned int gbFrecMixer_now[3]={0,0,0};;
 volatile unsigned short int gb_ct[3]={0,0,0};
 volatile unsigned short int gb_ct_Pulse[3]={0,0,0};
 volatile unsigned char gb_flip[3]={0,0,0};
 volatile unsigned char gb_dac_vol=50;
 unsigned char gb_use_sound_digital=0;
 unsigned char gb_mute=0;
 unsigned char gb_sound_signed=0;
#endif

//#ifdef use_lib_sound_ay8912
// #include "fabgl.h" //Para fabgl
// #include "fabutils.h" //Para fabgl
// //#include "WiFiGeneric.h" //Quitar para dependencia al compilar
//
// unsigned char gb_sillence_all=0;
// SoundGenerator soundGenerator;
// SineWaveformGenerator gb_sineArray[3];
// unsigned char gbVolMixer_before[3]={0,0,0};
// unsigned short int gbFrecMixer_before[3]={0,0,0};
// unsigned char gbVolMixer_now[3]={0,0,0};
// unsigned short int gbFrecMixer_now[3]={0,0,0}; 
// unsigned char gb_silence_all_channels=1; 
// unsigned char gbShiftLeftVolumen=0; //Maximo volumen shift left 2
// unsigned char gb_mute_sound=0;
// static unsigned int gb_sdl_time_sound_before;
//  
// void sound_cycleFabgl(void);
// void jj_mixpsg(void);
// void SilenceAllChannels(void); 
//#endif 

#ifdef use_lib_tinybitluni_fast
 #include "vga6bit.h"
 #ifdef use_lib_vga8colors
  //DAC 3 bits 8 colores
  // 3 bit pins  
  const unsigned char pin_config[] = {  
   PIN_RED_HIGH,
   PIN_GREEN_HIGH,  
   PIN_BLUE_HIGH,
   255,
   255,
   255,
   PIN_HSYNC,
   PIN_VSYNC
  };
 #else 
  const unsigned char pin_config[] = {
   PIN_RED_LOW,
   PIN_RED_HIGH,
   PIN_GREEN_LOW,
   PIN_GREEN_HIGH,
   PIN_BLUE_LOW,
   PIN_BLUE_HIGH,
   PIN_HSYNC,
   PIN_VSYNC
  };
 #endif  
#endif 


#define CONTROLLER_DEADZONE 8000

const unsigned char * gb_sdl_font= gb_sdl_font_6x8;

unsigned char gb_keyboard_cur_poll_ms= gb_ms_keyboard;
unsigned char gb_vga_cur_poll_ms= gb_ms_vga;
unsigned char gb_reset=0;
unsigned char gb_show_fps=0;

//uint8_t* gb_pac_ptr_rom;
uint8_t* gb_pac_ptr_ram;
uint8_t* gb_pac_ptr_tiles;
uint8_t* gb_pac_ptr_sprites;
//uint8_t* gb_pac_ptr_tile_rom; //No se necesita se tiene en flash convertido
//uint8_t* gb_pac_ptr_sprite_rom; //No se necesita se tiene en flash convertido
const unsigned char* gb_pac_ptr_palette_rom;
uint8_t gb_pac_sprite_pos[0x10];
bool gb_pac_flip_screen=0;
unsigned char gb_pac_p1_up=0;
unsigned char gb_pac_p1_left=0;
unsigned char gb_pac_p1_right=0;
unsigned char gb_pac_p1_down=0;
unsigned char gb_pac_coin_s1=0;
unsigned char gb_pac_coin_s2=0;
unsigned char gb_pac_rack_advance=0;
unsigned char gb_pac_credits_btn=0;
unsigned char gb_pac_board_test=0;
unsigned char gb_pac_p1_start=0;
unsigned char gb_pac_p2_start=0;
uint8_t gb_pac_int_vector=0;
bool gb_pac_vblank_enabled=0;
bool gb_pac_sound_enabled=0;
uint32_t gb_wsg_voice_frequency[3]={0,0,0};
uint32_t gb_wsg_voice_accumulator[3]={0,0,0};
uint8_t gb_wsg_voice_waveform_no[3]={0,0,0};
uint8_t gb_wsg_voice_volume[3]={0,0,0};
const unsigned char* gb_ptr_id_rom[4]; //4 punteros a la rom
const unsigned char *gb_ptr_rom_82s1237f;

unsigned char gb_end_setup= 0;

unsigned int gb_keyboard_time_cur=0;
unsigned int gb_keyboard_time_last=0;
unsigned int gb_vga_time_last=0;
unsigned int gb_vga_time_cur=0;

unsigned int gb_fps_cur=0;
unsigned int gb_fps_last=0;
unsigned int gb_fps_min=999999;
unsigned int gb_fps_max=0;
unsigned int gb_fps_med=0;
unsigned int gb_fps_time_cur=0;
unsigned int gb_fps_time_last=0;
unsigned int gb_fps_real_vga=0;


unsigned char ** gb_buffer_vga;
unsigned int **gb_buffer_vga32;
unsigned char gb_sync_bits;
const int *gb_ptrVideo_cur;
//unsigned char gb_color_vga[256]; //revisar no lo necesito
unsigned char * gb_224x288[288]; //doble buffer revisar tiene que desaparecer

unsigned short int gb_ancho;
unsigned short int gb_alto;
unsigned char gb_ini_x=0;
unsigned char gb_ini_y=0;
unsigned char gb_vertical=0;
unsigned char gb_videomode_cur=4; //400x300x56.2hz Horizontal
//unsigned char gb_videomode_last=4;

//unsigned char gb_jj_screen[288*224]; //Ya no se necesita
unsigned char gb_jj_color[32];

//static bool should_quit = false;
//static bool has_focus = true;
static unsigned char is_paused = 0;
unsigned char gb_speed = 1;

static pac* p = new pac();
static uint32_t current_time = 0;
static uint32_t last_time = 0;
static uint32_t dt = 0;


#ifdef use_lib_keyboard_uart
 char gb_buf_uart[BUFFER_SIZE_UART];
 unsigned char gb_rlen_uart=0;
 unsigned char gb_current_ms_poll_keyboard_uart= 10;
 unsigned int gb_curTime_keyboard_uart=0;
 unsigned int gb_curTime_keyboard_before_uart=0;
#endif




//#ifdef use_lib_sound_ay8912
// void SilenceAllChannels()
// {  
//  for (unsigned char i=0;i<3;i++)  
//  {
//   gb_sineArray[i].setFrequency(0);
//   gb_sineArray[i].setVolume(0);
//   gbVolMixer_before[i] = gbVolMixer_now[i] = 0;
//   gbFrecMixer_before[i] = gbFrecMixer_now[i] = 0;
//  }  
// }
//
//
//
// inline void jj_mixpsg()
// {
//  int auxFrec;
//  /*
//  gbVolMixer_now[0] = psgregs[8]&15;//((psgregs[8]&15)>4)?16:0;
//  gbVolMixer_now[1] = psgregs[9]&15;//((psgregs[9]&15)>4)?16:0;
//  gbVolMixer_now[2] = psgregs[10]&15;//((psgregs[10]&15)>4)?16:0;
//  if (gbVolMixer_now[0] == 0) gbFrecMixer_now[0] = 0;
//  else{
//   gbVolMixer_now[0]=15;
//   auxFrec =((psgregs[0]|((psgregs[1]<<8)&0xF00)))+1;
//   auxFrec = (auxFrec>0)?(62500/auxFrec):0;
//   if (auxFrec>15000) auxFrec=15000;
//   gbFrecMixer_now[0] = auxFrec;
//  }
//  if (gbVolMixer_now[1] == 0) gbFrecMixer_now[1] = 0;
//  else{
//   gbVolMixer_now[1]=15; 
//   auxFrec = ((psgregs[2]|((psgregs[3]<<8)&0xF00)))+1;  
//   auxFrec = (auxFrec>0)?(62500/auxFrec):0;
//   if (auxFrec>15000) auxFrec=15000;   
//   gbFrecMixer_now[1] = auxFrec;
//  }
//  if (gbVolMixer_now[2] == 0) gbFrecMixer_now[2] = 0;
//  else{
//   gbVolMixer_now[2]=15;
//   auxFrec = ((psgregs[4]|((psgregs[5]<<8)&0xF00)))+1;
//   auxFrec = (auxFrec>0)?(62500/auxFrec):0;
//   if (auxFrec>15000) auxFrec=15000;   
//   gbFrecMixer_now[2] = auxFrec;
//  }
//  */
// } 
//#endif 



void keyboardLoop(void);

//void PreparaPaleta(void);

/*
void PreparaPaleta()
{
 #ifdef use_lib_tinybitluni_fast
  #ifdef use_lib_vga8colors
   for (unsigned char i=0;i<8;i++)
   {  
    gb_color_vga[i] = (gb_color_vga[i] & 0x07) | gb_sync_bits;  
   }  
  #else
   for (unsigned char i=0;i<64;i++)
   {    
    gb_color_vga[i] = i | gb_sync_bits;  
   }
  #endif 
 #else
  //#ifdef use_lib_tinybitluni_fast  
  // for (unsigned char i=0;i<8;i++)
  // {  
  //  gb_color_vga[i] =  (gb_color_vga[i] & 0x07) | gb_sync_bits;
  //  //gb_color_vga[i] = (gb_color_vga[i] & vga.RGBAXMask) | vga.SBits;  
  // }
  //#endif
 #endif 
}
*/

//static void update_screen(pac* const p) 
//{
//}

//static void push_sample(pac* const p, int16_t sample) 
//{
////JJ  SDL_QueueAudio(audio_device, &sample, sizeof(int16_t) * 1);
//}

//static void send_quit_event() 
//{
//  should_quit = true;
//}


//static void screenshot(pac* const p) 
//{
/*

  // generate filename
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);

  //JJ char* filename = SDL_calloc(50, sizeof(char));
  char* filename = (char *)malloc(50);

  sprintf(filename, "%d-%d-%d %d.%d.%d - %s.bmp", tm.tm_year + 1900,
      tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, "pac");

  // if file already exists, we don't want to erase it
  FILE* f = fopen(filename, "r");
  if (f != NULL) 
  {
//JJ     SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
//JJ         "Cannot save screenshot: file %s already exists", filename);
//JJ     SDL_free(filename);
    printf("Cannot save screenshot: file %s already exists", filename);
    fflush(stdout);
    fclose(f);
    return;
  }
  fclose(f);

  // render screen buffer to BMP file
  const uint32_t pitch = sizeof(uint8_t) * 3 * PAC_SCREEN_WIDTH;
  const uint8_t depth = 32;
//JJ  SDL_Surface* s = SDL_CreateRGBSurfaceWithFormatFrom(p->screen_buffer,
//JJ      PAC_SCREEN_WIDTH, PAC_SCREEN_HEIGHT, depth, pitch, SDL_PIXELFORMAT_RGB24);
//JJ  SDL_SaveBMP(s, filename);
//JJ  SDL_FreeSurface(s);

//JJ  SDL_Log("Saved screenshot: %s", filename);
//JJ  SDL_free(filename);
 printf("Saved screenshot: %s", filename);
 fflush(stdout);
*/ 
//}

#ifdef use_lib_keyboard_uart
 void do_keyboard_uart()
 {
  unsigned int contBuf=0;
  if (gb_rlen_uart>0)
  {
   while (contBuf < gb_rlen_uart)
   {
    switch (gb_buf_uart[contBuf])
    {
     case 0x01: //F2   01 62 4F 51      
      if ((contBuf+3) < gb_rlen_uart)
      {
       gb_show_osd_main_menu= 1;
       contBuf+= 3;
      }
      break;

     case 0x09: //TAB saco menu tambien
      gb_show_osd_main_menu= 1;      
      break;

     case 0x0D: case 0x0A: //0D 0A ENTER 
      //keyboard[6]&= 0xFE; //ENTER
      //IO_keyboard_matrix_[0]= 0xFD;
      break;

     case 0x2B: 
      //keyboard[3]&= 0xFE; //El + como ALT_GR
      break;

     case 0x1B: //Arriba 1B 5B 41
      if ((contBuf+2) < gb_rlen_uart)
      {
       contBuf++;
       if (gb_buf_uart[contBuf] == 0x5B)
       {
        contBuf++;
        switch (gb_buf_uart[contBuf])
        {
         case 0x41:           
          //keyboard[4]&= 0xF7; //kempston arriba //SHIFT + 7          
          //IO_keyboard_matrix_[0]&= 0x7F;
          //IO_keyboard_matrix_[6]&= 0xBF;
          //arriba 1B 5B 41
          gb_pac_p1_up= 1;
          break;
         case 0x42: 
          //keyboard[4]&= 0xEF;  //kempston abajo //SHIFT + 6
          gb_pac_p1_down= 1;
          break; //abajo 1B 5B 42
         case 0x43: 
          //keyboard[4]&= 0xFB; //derecha 1B 5B 43 //SHIFT + 8
          gb_pac_p1_right= 1;
          break;
         case 0x44: 
          //keyboard[3]&= 0xEF; //izquierda 1B 5B 44 //SHIFT + 5
          //IO_keyboard_matrix_[0]&= 0xFB;
          //IO_keyboard_matrix_[7]&= 0xFD;          
          gb_pac_p1_left= 1;
          break; 
        }
       }
      }
      break;       

     //row0
     //case 0x2D: keyboard[0]&= 0xFE; break; //SHIFT LEFT -
     case 0x31: gb_pac_p1_start= 1; break; //1 start (1p)
     case 0x32: gb_pac_p2_start= 1; break; //2 start (2p)
     //case 0x33: IO_keyboard_matrix_[1]&= 0xFE; break; //3
     //case 0x34: IO_keyboard_matrix_[1]&= 0xF7; break; //4
     case 0x35: gb_pac_coin_s1=1; break; //5 coin (slot 1)   
     case 0x76: case 0x56: gb_pac_coin_s2= 1; break; //V coin (slot 2)
     case 0x74: case 0x54: 
      gb_pac_board_test= 1; 
      //gbVolMixer_now[0]= gbVolMixer_now[1]= gbVolMixer_now[2]= 0; //silencio= 1; //silencio
      break; //T board test
//     case 0x70: case 0x50: is_paused= ((~is_paused)&0x01); break; //P paused
     //case 0x6D: case 0x4D: p->mute_audio= 1; break; //M mute audio
     //case 0x69: case 0x49: pac_cheat_invincibility(p); break; //I
//     case 0x72: case 0x52: speed= (speed==5) ? 1 : 5; break; //R turbo speed
     //case 0x6E: case 0x4E: speed= 1; break; //N normal speed

     case 0x30: 
      //gb_videomode_cur++;
//      gb_vertical= 0;
//      SetVideoMode(gb_videomode_cur);
      break; //0 inc video mode
     case 0x39: 
      //gb_videomode_cur++;      
//      gb_vertical= 1;
//      SetVideoMode(gb_videomode_cur);
      break; //9 dec video mode
    }//fin switch gb_buf_uart
    contBuf++;
   }//fin while contBuf   
  }//fin if gb_rlen_uart     
 }
 
/*
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
 */
#endif


void keyboardLoop()
{
 //p->p1_start= 0;
 //p->p2_start= 0;
 //p->p1_up= 0;
 //p->p1_down= 0;
 //p->p1_left= 0;
 //p->p1_right= 0;
 //p->coin_s1= 0;
 //p->coin_s2= 0;
 //p->board_test= 0;
 
 //speed= 1;
 //is_paused= 0;
 //p->mute_audio= 0;
 gb_show_osd_main_menu = (!keymap[PS2_KC_F1])&0x01;

 gb_pac_p1_start= !keymap[PS2_KC_1];
 gb_pac_p2_start= !keymap[PS2_KC_2];
 gb_pac_coin_s1= !keymap[PS2_KC_5];
 gb_pac_coin_s2= !keymap[PS2_KC_V];
 gb_pac_board_test= !keymap[PS2_KC_T];
 gb_pac_p1_up= !keymap[KEY_CURSOR_UP];
 gb_pac_p1_down= !keymap[KEY_CURSOR_DOWN];
 gb_pac_p1_left= !keymap[KEY_CURSOR_LEFT];
 gb_pac_p1_right= !keymap[KEY_CURSOR_RIGHT];

/*
 if (keymap[PS2_KC_0]==0)
 {    
  while (keymap[PS2_KC_0]==0);
  //Serial.printf("key 0\n");
  gb_vertical= 0;
  SetVideoMode(0);
 }

 
 if (keymap[PS2_KC_9]==0)
 {    
  while (keymap[PS2_KC_9]==0);
  //Serial.printf("key 9\n");
  gb_vertical= 1;
  SetVideoMode(0);
 }
 */

/*
 if (keymap[PS2_KC_A]==0)
 {
  while (keymap[PS2_KC_A]==0);
  Serial.printf("key 0\n");
  gb_vertical= 0;
  gb_ini_x=0;
  gb_ini_y=0;
 }
 */

/*
 if (keymap[PS2_KC_0]==0)
 {
  gb_ini_key_kc0= 1;  
  //gb_event_videoMode= 1;  
 }
 else
 {//cuando se libera
  if (gb_ini_key_kc0 == 1)
  {
   gb_vertical= 0;
   gb_ini_key_kc0= 0;
   Serial.printf("key 0\n"); 
  }
 }
 */


 //if (keymap[PS2_KC_P]==0)
 //{
 // while (keymap[PS2_KC_P]==0);
 // is_paused= (~is_paused) & 0x01;
 //}
  
 //if (keymap[PS2_KC_R]==0)
 //{
 // while (keymap[PS2_KC_R]==0);
 // speed= (speed == 5) ? 0 : 5;
 //}   

/*
 if (keymap[PS2_KC_0]==0)
 {
  while (keymap[PS2_KC_0]==0) {}   
  gb_vertical= 0;
  SetVideoMode(gb_videomode_cur);   
 }

 if (keymap[PS2_KC_9]==0)
 {
  while (keymap[PS2_KC_9]==0) {}      
  gb_vertical= 1;
  SetVideoMode(gb_videomode_cur);   
 }
 */
 


 //is_paused= 0;
 //speed= 1;

    /*
 SDL_PollEvent (&gb_event);
 Uint8 *keystates = SDL_GetKeyState(NULL); 
  
 p->p1_start= (keystates[SDLK_1] == 1) ? 1 : 0; // start (1p)
 p->p2_start= (keystates[SDLK_2] == 1) ? 1 : 0; // start (2p)
 p->p1_up= (keystates[SDLK_UP] == 1) ? 1 : 0;
 p->p1_down= (keystates[SDLK_DOWN] == 1) ? 1 : 0;
 p->p1_left= (keystates[SDLK_LEFT] == 1) ? 1 : 0;
 p->p1_right= (keystates[SDLK_RIGHT] == 1) ? 1 : 0;
 //case SDL_SCANCODE_C:
 p->coin_s1= (keystates[SDLK_5] == 1) ? 1 : 0;
 p->coin_s2= (keystates[SDLK_v] == 1) ? 1 : 0; // coin (slot 2)
 p->board_test= (keystates[SDLK_t] == 1) ? 1 : 0;// board test
 p->mute_audio= (keystates[SDLK_m] == 1) ? 1 : 0;
 is_paused= (keystates[SDLK_p] == 1) ? 1 : 0;
 //case SDL_SCANCODE_S: screenshot(p); break;
 if (keystates[SDLK_i] == 1)
 {
  pac_cheat_invincibility(p);
 }
 //speed= (keystates[SDLK_TAB] == 1) ? 5 : 1; //speed
 
 if (keystates[SDLK_F2] == 1)
  speed= 5;
 if (keystates[SDLK_F3] == 1)
  speed= 1;  
  
 
 switch (gb_event.type)
 {
  case SDL_KEYDOWN:
  {
   switch (gb_event.key.keysym.sym)
   {     
    case SDLK_F12:
     should_quit = true; 
     break;
   }                 
   break;                  
  }
 }    
 */ 
}


//**************************************
static void mainloop(void) 
{
 current_time = millis();
 dt = current_time - last_time;

  
 gb_keyboard_time_cur= millis();
 if ((gb_keyboard_time_cur-gb_keyboard_time_last)>=(gb_keyboard_cur_poll_ms-1))
 {
  gb_keyboard_time_last= gb_keyboard_time_cur;
  keyboardLoop();  
 }

 #ifdef use_lib_keyboard_uart  
  gb_curTime_keyboard_uart= millis();
  if ((gb_curTime_keyboard_uart-gb_curTime_keyboard_before_uart)>=gb_current_ms_poll_keyboard_uart)
  {   
   gb_curTime_keyboard_before_uart= gb_curTime_keyboard_uart;   
   keyboard_uart_poll();       
  }
  do_keyboard_uart();
 #endif 


 if (gb_show_osd_main_menu == 1)
 {  
  do_tinyOSD();
  current_time = millis(); //Evita aceleracion emu al salir del OSD
  last_time= current_time;
  dt = current_time - last_time;  
 }
 
 /*
 gb_vga_time_cur= SDL_GetTicks(); 
 if ((gb_vga_time_cur-gb_vga_time_last)>20)
 {
  gb_vga_time_last= gb_vga_time_cur;
  vga_dump_poll(); //Termina llamando a SDL_Flip
 }
 */

  //if (!is_paused && has_focus) 
  if (is_paused==0) 
  {    
   pac_update(p, dt * (unsigned int)gb_speed);   
  }


   gb_fps_time_cur= millis();
   unsigned int auxTime= (gb_fps_time_cur - gb_fps_time_last);
   if (auxTime >= 999)
   {
    gb_fps_time_last= gb_fps_time_cur;
    if (gb_show_fps == 1)
    {
     Serial.printf("FPS %d %d\r\n",gb_fps_cur,gb_fps_real_vga);
    }
    gb_fps_cur=0;
    gb_fps_real_vga=0;
    //ShowFPSCall();
   }             

  last_time = current_time;
}  



 #ifdef use_lib_sound_digital
 hw_timer_t *gb_timerSound = NULL;

 volatile unsigned char gb_spk_data=0;
 volatile unsigned char gb_spk_data_before=0; 
 volatile unsigned char gb_sillence_all=0;

 
 //volatile unsigned int gb_time=0;




/*
 void IRAM_ATTR onTimerSound()
 {
  #ifdef use_lib_sound_dac
  //Para DAC   
  unsigned int auxByte;
  unsigned int media=0;
  unsigned int valor;
  
  if (gb_spk_data != gb_spk_data_before)
  {
   //dac_output_voltage(25,gb_spk_data);
   //dacWrite(25,gb_spk_data);
   #ifdef SPEAKER_PIN == 25
    dac_output_voltage(DAC_CHANNEL_1, gb_spk_data);
   #else 
    dac_output_voltage(DAC_CHANNEL_2, gb_spk_data);
   #endif 
   gb_spk_data_before= gb_spk_data;
  }
  
  gb_contador_muestra++;    
  //gb_time= gb_contador_muestra/8000;
  //gb_time++;
  

  
  //if (
  //    (gb_sillence_all==1)
  //    //||
  //    //((gbFrecMixer_now[0]==0)&&(gbFrecMixer_now[1]==0)&&(gbFrecMixer_now[2]==0))
  //    //||
  //    //((gbVolMixer_now[0]==0)&&(gbVolMixer_now[1]==0)&&(gbVolMixer_now[2]==0))
  //   ) 
  //{
  // //gb_spk_data= 127;
  // gb_spk_data= 0;
  // return;
  //}
  
  if (gb_sillence_all==1)
  {
   gb_spk_data= 0;
   return;
  }
  

  for (unsigned char i=0;i<3;i++)
  {
   //auxByte= (gbVolMixer_now[i] * gbFrecMixer_now[i]) * gb_time;
   //auxByte= (gbVolMixer_now[i] * gbFrecMixer_now[i] * 6 * gb_contador_muestra)/8000;
   //Tengo que multiplicar por 6, que es 4+2 aproximado a 2xPI
   if (gbVolMixer_now[i] == 0)
   {
    valor= 0;
   }
   else
   {
    auxByte= (gbVolMixer_now[i] * gbFrecMixer_now[i] * gb_contador_muestra)<<2; //x4    
    auxByte= (auxByte + (auxByte<<1)) >>13; //DIV 8192 aproximado 8000 x2 DIV 8192
    valor= gb_sin[(auxByte & 0xFF)]; //MOD 256    
   }
   //if (valor<64)
   //{
   // valor= 63-valor;
   //}      
   media= (media + valor)>>1; //Media aproximada   
  } 
  //media= (media/3);  
  //media= media>>6;
  //media= media;
  //media= (media>>6);
  //media= (media>>1);
  //media= (unsigned char)(media) & 0xFF;
  //media= media<<3;
  //media= media;  
  //if (media>250) media=250;
  //if (media<5) media=5;

  media= (media & 0xFF);
  switch (gb_dac_vol)
  {
   case 1: media= media>>1; break; //25%
   //case 2: media= media; break; //50%
   case 3: media= media<<1; break;
   case 4: media= media<<2; break; //100%
  }
  gb_spk_data= media;
  //gb_time++;
#else


   
  //Para digital
  if (gb_spk_data != gb_spk_data_before)
  {
   //digitalWrite(SPEAKER_PIN, gb_spk_data);
   //GPIO 0..31
   if (gb_spk_data)
    GPIO.out_w1ts= 1<<25; //1<<25;
   else
    GPIO.out_w1tc = 1<<25;
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

  if ((gb_flip[0]==1)&&(gbVolMixer_now[0]>0)){
   gb_spk_data= 1;
  }
  else{
   if ((gb_flip[1]==1)&&(gbVolMixer_now[1]>0)){
    gb_spk_data= 1;
   }
   else{
    gb_spk_data= ((gb_flip[2]==1)&&(gbVolMixer_now[2]>0)) ? 1 : 0;
   }
  }  
 #endif 
  
 }
*/

#endif 


void setup() 
{
 #ifdef use_lib_sound_digital
  #ifdef use_lib_sound_dac
   #ifdef SPEAKER_PIN == 25
    dac_output_enable(DAC_CHANNEL_1);
   #else
    dac_output_enable(DAC_CHANNEL_2);
   #endif    
  #else 
   pinMode(SPEAKER_PIN, OUTPUT);
  #endif 
  gb_timerSound = timerBegin(0, 80, true);
  #ifdef use_lib_sound_dac
   timerAttachInterrupt(gb_timerSound, &onTimerSoundDAC, true);
  #else
   timerAttachInterrupt(gb_timerSound, &onTimerSoundDigital, true);
  #endif 
  timerAlarmWrite(gb_timerSound, 125, true); //1000000 1 segundo  125 es 8000 hz
  timerAlarmEnable(gb_timerSound);  
 #endif

 #if defined(use_lib_log_serial) || defined(use_lib_keyboard_uart)
  Serial.begin(115200);
  Serial.printf("Begin Setup\n");   
 #endif   
 #ifdef use_lib_keyboard_uart
  Serial.setTimeout(use_lib_keyboard_uart_timeout);
 #endif 

 //gb_pac_ptr_rom= (uint8_t *)malloc(0x4000); 
 //gb_pac_ptr_tiles= (uint8_t *)malloc(0x4000); //256 * 8 * 8 //Apunta a Flash
 //gb_pac_ptr_sprites= (uint8_t *)malloc(0x4000); //64 * 16 * 16 //Apunta a Flash 

 gb_pac_ptr_ram= (uint8_t *)malloc(0x1000);
 //gb_pac_ptr_tile_rom= (uint8_t *)malloc(0x1000); //No se necesita se tiene en flash convertido
 //gb_pac_ptr_sprite_rom= (uint8_t *)malloc(0x1000); //No se necesita se tiene en flash convertido

 //pac_assign_ptr(p); //No lo necesito
 
 //Doble buffer
 for (unsigned short int y=0;y<288;y++)
 {
  gb_224x288[y]= (unsigned char *)malloc(224);
  memset(gb_224x288[y],0,224);
 }     

 #ifdef use_lib_vga400x300
  vga_init(pin_config,VgaMode_vga_mode_400x300,false,0,0,0,0,0); //400x300 Funciona bien
  gb_ptrVideo_cur= VgaMode_vga_mode_400x300;
  gb_ancho= 400;
  gb_alto= 300;
  gb_vertical= 0;
  gb_videomode_cur= 4;
  gb_ini_x=21; //(400-224)=176/2/4=22
  gb_ini_y=5; //(300-288)=12/2=6  
 #else
  #ifdef use_lib_vga320x350_bitluni
   vga_init(pin_config,VgaMode_vga_mode_320x350,false,0,0,0,0,0); //320x350 Funciona bien
   gb_ptrVideo_cur= VgaMode_vga_mode_320x350;
   gb_ancho= 320;
   gb_alto= 350;
   gb_vertical= 0;
   gb_videomode_cur= 2;
   gb_ini_x=11; //(320-224)=96/2/4=12
   gb_ini_y=30; //(350-288)=62/2=31   
  #else
   #ifdef use_lib_vga320x240
    vga_init(pin_config,VgaMode_vga_mode_320x240,false,0,0,0,0,0); //320x240
    gb_ptrVideo_cur= VgaMode_vga_mode_320x240;
    gb_ancho= 320;
    gb_alto= 240;
    gb_vertical= 1;
    gb_videomode_cur= 0;
    gb_ini_x=3; //(320-288)=32/2/4=4
    gb_ini_y=7; //(240-224)=16/2=8    
   #else
   #endif
  #endif
 #endif 




 //vga_init(pin_config,VgaMode_vga_mode_320x200,false); //360x200
 gb_sync_bits= vga_get_sync_bits();
 gb_buffer_vga = vga_get_framebuffer();
 gb_buffer_vga32=(unsigned int **)gb_buffer_vga;  
 //PreparaPaleta(); //revisar no se necesita
 
 unsigned int a0= (0|gb_sync_bits);
 unsigned int a32= (a0<<24)|(a0<<16)|(a0<<8)|a0;
 for (unsigned int y=0;y<gb_alto;y++)
 {
  for (unsigned int x=0;x<(gb_ancho>>2);x++)
  {
   //gb_buffer_vga[y][x^2]= 0 | gb_sync_bits; //gb_color_vga[0];
   gb_buffer_vga32[y][x]= a32; //gb_color_vga[0];
  }
 }
 
 //delay(100);  
 
 //SetVideoMode(gb_videomode_cur,gb_vertical); //Borra pantalla y ajusta

 
 kb_begin(); 


 //#ifdef use_lib_sound_ay8912
 // gb_sdl_time_sound_before = millis();
 // for (unsigned char i=0;i<3;i++)
 // {
 //  soundGenerator.attach(&gb_sineArray[i]);
 //  gb_sineArray[i].enable(true);
 //  gb_sineArray[i].setFrequency(0);
 // }
 // soundGenerator.play(true);
 //#endif

 //JJ if (pac_init(p) != 0) 
 //JJ {   
 //JJ   Serial.printf("Please copy rom files next to pac executable.\r\n");    
 //JJ }
 pac_init(p);
 Serial.printf("ROMS PACman cargadas con exito\r\n");

 
 //JJ p->sample_rate = audio_spec.freq;
 //p->push_sample = push_sample;
 //p->update_screen = update_screen;
 //update_screen(p); //Ya no se necesita copiar de gb_jj_screen a gb_buffer_vga 


 current_time = last_time = millis();

 gb_keyboard_time_cur= gb_keyboard_time_last= gb_fps_time_cur= gb_fps_time_last= millis();
 Serial.printf("END SETUP %d\r\n", ESP.getFreeHeap());

 gb_end_setup= 1;
}

void loop() 
{    
 //while ((should_quit== false)&&(gb_end_setup == 1))
 while (gb_end_setup == 1)
 {
  mainloop();
  if (gb_reset==1)
  {//reset software
   gb_reset= 0;
   gbVolMixer_now[0]= gbVolMixer_now[1]= gbVolMixer_now[2]= 0; //silencio
   pac_init(p);
  }  
 }
}
