#ifndef OSDFILE_H
 #define OSDFILE_H

 #include "gbConfig.h"
 
 void do_tinyOSD(void);
 void SDLActivarOSDMainMenu();
 void SDLEsperaTeclado(void);
 void SDLClear(void);

 
 #ifdef use_lib_keyboard_uart
  void keyboard_uart_poll(void);
  unsigned char checkKey_uart(unsigned char scancode);
 #endif 

 void SetVideoMode(unsigned char id,unsigned char aVertical);
 void IRAM_ATTR onTimerSoundDigital(void);
 #ifdef use_lib_sound_dac
  void IRAM_ATTR onTimerSoundDAC(void);
 #endif 

#endif
