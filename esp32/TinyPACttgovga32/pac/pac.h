#ifndef PAC_PAC_H
#define PAC_PAC_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "z80.h"
#include "wsg.h"

#define PAC_CLOCK_SPEED 3072000 // 3.072 MHz (= number of cycles per second)
#define PAC_FPS 60
#define PAC_CYCLES_PER_FRAME (PAC_CLOCK_SPEED / PAC_FPS)
#define PAC_SCREEN_WIDTH 224
#define PAC_SCREEN_HEIGHT 288

typedef struct pac pac;
struct pac 
{
  z80 cpu;
  //JJ uint8_t rom[0x10000]; // 0x0000-0x4000
//JJ  uint8_t rom[0x4000]; // 0x0000-0x4000 //Limito la rom a 16384 bytes 0x4000
//JJ  uint8_t ram[0x1000]; // 0x4000-0x5000
  uint8_t sprite_pos[0x10]; // 0x5060-0x506f

//  uint8_t* rom; // 0x0000-0x4000 //Limito la rom a 16384 bytes 0x4000
//  uint8_t* ram; // 0x4000-0x5000  //Ya no lo necesito esta en gb_pac_ptr_ram

  //JJ uint8_t color_rom[32];
  //JJ uint8_t palette_rom[0x100];
//JJ  uint8_t tile_rom[0x1000];
  //uint8_t* tile_rom; //No lo necesito lo tengo en gb_pac_ptr_tile_rom
//JJ  uint8_t sprite_rom[0x1000];
  //uint8_t *sprite_rom; //No lo necesito lo tengo en gb_pac_ptr_sprite_rom
  //uint8_t sound_rom1[0x100]; //No lo necesito, genero frecuencias con oscilador
  //uint8_t sound_rom2[0x100]; //No lo necesito
  const unsigned char *color_rom; 
  //const unsigned char *palette_rom; //Ya no lo necesito tengo gb_pac_ptr_palette_rom
  //const unsigned char *tile_rom;
  //const unsigned char *sprite_rom;
  //const unsigned char *sound_rom1;
  //const unsigned char *sound_rom2;  

//JJ  uint8_t tiles[256 * 8 * 8]; // to store predecoded tiles 16384 bytes
//JJ  uint8_t sprites[64 * 16 * 16]; // to store predecoded sprites 16384 bytes
  //uint8_t* tiles; // to store predecoded tiles 16384 bytes Esta en gb_pac_ptr_tiles
  //uint8_t* sprites; // to store predecoded sprites 16384 bytes  Esta en gb_pac_ptr_sprites

  uint8_t int_vector;
  bool vblank_enabled;
  bool sound_enabled;
  bool flip_screen;

  // in 0 port
  unsigned char p1_up, p1_left, p1_right, p1_down, coin_s1, coin_s2, rack_advance, credits_btn;

  // in 1 port
  unsigned char board_test, p1_start, p2_start;

  // ppu
//JJ  uint8_t screen_buffer[PAC_SCREEN_HEIGHT * PAC_SCREEN_WIDTH * 3]; //288x244x3=193536 bytes Ya no lo necesito
  //void (*update_screen)(pac* const n);

  // audio
   wsg sound_chip; //revisar sonido
  //JJ int audio_buffer_len; //revisar sonido
  //JJ int16_t* audio_buffer; //revisar sonido
  //JJ int sample_rate; //revisar sonido
  //JJ unsigned char mute_audio; //revisar sonido
  //void (*push_sample)(pac* const n, int16_t);
};

//JJ int pac_init(pac* const p);
void pac_init(pac* const p);
//JJ void pac_quit(pac* const p);
void pac_update(pac* const p, unsigned int ms);

//void pac_cheat_invincibility(pac* const p);

//void pac_assign_ptr(pac* const p);

//#ifdef use_lib_sound_ay8912
// void sound_cycleFabgl(void);
//#endif

#endif // PAC_PAC_H
