#ifndef _GB_CONFIG
 #define _GB_CONFIG

 //Fixes error keyboards not initializing (solution dcrespo3D)
 //#define FIX_PERIBOARD_NOT_INITING

 //Sound
 //#define use_lib_sound_ay8912
 #define use_lib_sound_digital
 #define use_lib_sound_dac

 //#define use_lib_fix_double_precision

 #define use_lib_tinybitluni_fast

 //Video mode selection VGA
 //Select one mode and deselect other modes
 //400x300 800x600 35.1Khz 56.2
 #define use_lib_vga400x300_bitluni
 //320x350 640x350 31.5Khz 70 Hz
 //#define use_lib_vga320x350_bitluni
 //#define use_lib_vga320x240x60hz_bitluni
 //320x200 720x400 31.4 Khz 70.0 Hz freq:25175000 pixel_clock:12587500
 //QVGA 320x240 60Hz freq:12600000 funciona
 //#define use_lib_vga320x240x60hz_fabgl  


 #ifdef use_lib_vga400x300_bitluni
  #define use_lib_vga400x300
 #else
  #ifdef use_lib_vga320x350_bitluni
   #define use_lib_vga320x350
  #else
   #ifdef use_lib_vga320x240x60hz_bitluni
    #define use_lib_vga320x240
   #else
    #ifdef use_lib_vga320x240x60hz_fabgl
     #define use_lib_vga320x240
     #ifndef use_lib_fix_double_precision
      #define use_lib_fix_double_precision
     #endif
    #else
    #endif
   #endif
  #endif
 #endif

 //Use terminal keyboard, putty
 //#define use_lib_keyboard_uart
 
 //timeout read millis
 #define use_lib_keyboard_uart_timeout 0

 //log keyboard remote uart 
 //#define use_lib_log_keyboard_uart



 #define use_lib_log_serial

 
 //Read keyboard every x milliseconds
 #define gb_ms_keyboard 10
 //VGA 16 ms 60 FPS refresh 
 #define gb_ms_vga 40

 
#endif
