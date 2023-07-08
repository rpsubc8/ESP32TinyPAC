#include "gbConfig.h"
#include "gbGlobals.h"
#include "pac.h"
#include "gbrompacman6e.h"
#include "gbrompacman6f.h"
#include "gbrompacman6h.h"
#include "gbrompacman6j.h"
#include "gbrom82s1237f.h"
#include "gbrom82s1264a.h"
#include "gbrompacman5e.h"
#include "gbrompacman5f.h"
#include "gbrom82s1261m.h"
#include <Arduino.h>

static uint8_t rb(void* userdata, uint16_t addr) 
{
  pac* const p = (pac*) userdata;

  // according to https://www.csh.rit.edu/~jerry/arcade/pacman/daves/
  // the highest bit of the address is unused
  addr &= 0x7fff;

  if (addr < 0x4000) 
  {
   unsigned char id= (addr>>12); //DIV 4096
   return gb_ptr_id_rom[id][(addr & 0x0FFF)]; //MOD 4096
   //JJ return p->rom[addr];
  } else if (addr < 0x5000) {
    return p->ram[addr - 0x4000];
  } else if (addr <= 0x50ff) { // io
    if (addr == 0x5003) {
      return p->flip_screen;
    } else if (addr == 0x5004 || addr == 0x5005) {
      // lamps, not used in pacman
      return 0;
    } else if (addr == 0x5006) {
      // coin lockout, not used in pacman
      return 0;
    } else if (addr == 0x5007) {
      // coin counter
    } else if (addr >= 0x5000 && addr <= 0x503f) { // in 0
      return (!p->p1_up << 0) | (!p->p1_left << 1) | (!p->p1_right << 2) |
             (!p->p1_down << 3) | (!p->rack_advance << 4) | (!p->coin_s1 << 5) |
             (!p->coin_s2 << 6) | (!p->credits_btn << 7);
    } else if (addr >= 0x5040 && addr <= 0x507f) { // in 1
      return (!p->p1_up << 0) | (!p->p1_left << 1) | (!p->p1_right << 2) |
             (!p->p1_down << 3) | (!p->board_test << 4) | (!p->p1_start << 5) |
             (!p->p2_start << 6) | (1 << 7); // cabinet mode: 1=upright 0=table
    } else if (addr >= 0x5080 && addr <= 0x50bf) { // dip switch
      // bit 0-1: 1 Coin 1 Credit
      // bit 2-3: 3 Pacman Per Game
      // bit 4-5: Bonus Player @ 10000 Pts
      // bit 6: Difficulty (normal=1, hard=0)
      // bit 7: Alternate ghost names
      //JJ return 0b11001001;
      return 0xC9; //0b11001001
    }
  } else {
    Serial.printf("ERR: read at %04x\r\n", addr);
    return 0;
  }

  return 0xff;
}

static void wb(void* userdata, uint16_t addr, uint8_t val) 
{
  pac* const p = (pac*) userdata;

  // according to https://www.csh.rit.edu/~jerry/arcade/pacman/daves/
  // the highest bit of the address is unused
  addr &= 0x7fff;

  if (addr < 0x4000) {
    // cannot write to rom
  } else if (addr < 0x5000) {
    p->ram[addr - 0x4000] = val;
  } else if (addr <= 0x50ff) { // io
    if (addr == 0x5000) {
      p->vblank_enabled = val & 1;
    } else if (addr == 0x5001) {
      p->sound_enabled = val & 1;
    } else if (addr == 0x5002) {
      // aux board?
    } else if (addr == 0x5003) {
      p->flip_screen = val & 1;
    } else if (addr == 0x5004 || addr == 0x5005) {
      // lamps, not used in pacman
    } else if (addr == 0x5006) {
      // coin lockout, not used in pacman
    } else if (addr == 0x5007) {
      // coin counter
    } else if (addr >= 0x5040 && addr <= 0x505f) { // audio
       wsg_write(&p->sound_chip, addr - 0x5040, val); //revisar sonido
    } else if (addr >= 0x5060 && addr <= 0x506f) {
      p->sprite_pos[addr - 0x5060] = val;
    } else if (addr >= 0x50c0 && addr <= 0x50ff) {
      // watchdog: no action is needed here, because watchdog is not
      // implemented on the emu.
    }
  } else {
    Serial.printf("ERR: write %02x at %04x\r\n", val, addr);
  }
}

inline unsigned char port_in(z80* const z, unsigned char port) 
{
  return 0;
}

inline void port_out(z80* const z, unsigned char port, unsigned char val) 
{
  pac* const p = (pac*) z->userdata;

  // setting the interrupt vector
  if (port == 0) {
    p->int_vector = val;
  }
}

// appends two NULL-terminated strings, creating a new string in the process.
// The pointer returned is owned by the user.
/*
static inline char* append_path(const char* s1, const char* s2)
{
  const int buf_size = strlen(s1) + strlen(s2) + 1;
  char* path = (char *) calloc(buf_size, sizeof(char));
  if (path == NULL) {
    return NULL;
  }
  snprintf(path, buf_size, "%s%s", s1, s2);
  return path;
}
*/

// copies "nb_bytes" bytes from a file into memory
/*
static inline int load_file(
    const char* filename, uint8_t* memory, size_t nb_bytes) {
  FILE* f = fopen(filename, "rb");
  if (f == NULL) {
    fprintf(stderr, "error: can't open file '%s'.\n", filename);
    return 1;
  }

  size_t result = fread(memory, sizeof(uint8_t), nb_bytes, f);
  if (result != nb_bytes) {
    fprintf(stderr, "error: while reading file '%s'\n", filename);
    return 1;
  }

  fclose(f);
  return 0;
}
*/

// MARK: graphics

// the color palette is stored in color_rom (82s123.7f). Each byte corresponds
// to one color and is composed of three components red, green and blue
// following that pattern: 0bBBGGGRRR.
// Each color component corresponds to a color intensity.
// @TODO: add comment on how to get from color intensity to RGB color.
static inline void get_color(pac* const p, uint8_t color_no, uint8_t* r, uint8_t* g, uint8_t* b) 
{
  const uint8_t data = p->color_rom[color_no];
  *r = ((data >> 0) & 1) * 0x21 + ((data >> 1) & 1) * 0x47 +
       ((data >> 2) & 1) * 0x97;
  *g = ((data >> 3) & 1) * 0x21 + ((data >> 4) & 1) * 0x47 +
       ((data >> 5) & 1) * 0x97;
  *b = ((data >> 6) & 1) * 0x51 + ((data >> 7) & 1) * 0xae;
}


inline unsigned char jj_get_color(pac* const p,unsigned char idColor)
{
// unsigned char data;
// unsigned char r,g,b;
 //unsigned char toReturn=0;
 
 //if (idColor>3)
 //{
 // printf("id:%d\n",idColor);
 // fflush(stdout);
 //}

// data = p->color_rom[idColor];
 //r = ((data >> 0) & 1) * 0x21 + ((data >> 1) & 1) * 0x47 + ((data >> 2) & 1) * 0x97;
 //g = ((data >> 3) & 1) * 0x21 + ((data >> 4) & 1) * 0x47 + ((data >> 5) & 1) * 0x97;
 //b = ((data >> 6) & 1) * 0x51 + ((data >> 7) & 1) * 0xae;
 //return (((r>>6)<<4)|((g>>6)<<2)|((b>>6)&0x03));
 
 //r= (r>>6);
 //g= (g>>6);
 //b= (b>>6);
 //toReturn= (r<<4)|(g<<2)|(b&0x03);
 //return toReturn;
// r= (data>>1) & 0x03;
// g= (data>>4) & 0x03;
// b= (data>>6) & 0x03;
 //return ((r<<4)|(g<<2)|(b&0x03));
 
 return (gb_jj_color[idColor]);
}

// Color palettes are defined in palette_rom (82s126.4a): each palette contains
// four colors (one byte for each color).
static inline void get_palette(pac* const p, uint8_t pal_no, uint8_t* pal) {
  pal_no &= 0x3f;
  pal[0] = p->palette_rom[pal_no * 4 + 0];
  pal[1] = p->palette_rom[pal_no * 4 + 1];
  pal[2] = p->palette_rom[pal_no * 4 + 2];
  pal[3] = p->palette_rom[pal_no * 4 + 3];
}

// decodes a strip from pacman tile/sprite roms to a bitmap output where each
// byte represents one pixel.
static inline void decode_strip(pac* const p, uint8_t* const input, uint8_t* const output, int bx, int by, int img_width) 
{
  const int base_i = by * img_width + bx;
  for (int x = 0; x < 8; x++) {
    uint8_t strip = *(input + x);

    for (int y = 0; y < 4; y++) {
      // bitmaps are stored mirrored in memory, so we need to read it
      // starting from the bottom right:
      int i = (3 - y) * img_width + 7 - x;
      output[base_i + i] = (strip >> (y % 4)) & 1;
      output[base_i + i] |= ((strip >> (y % 4 + 4)) & 1) << 1;
    }
  }
}

// preloads sprites and tiles
static inline void preload_images(pac* const p) {
  // sprites and tiles are images that are stored in sprite/tile rom.
  // in memory, those images are represented using vertical "strips"
  // of 8*4px, each strip being 8 bytes long (each pixel is stored on two
  // bits)
  const int LEN_STRIP_BYTES = 8;

  // tiles are 8*8px images. in memory, they are composed of two strips.
  const int NB_PIXELS_PER_TILE = 8 * 8;
  const int TILE_WIDTH = 8;
  const int NB_TILES = 256;
  memset(p->tiles, 0, NB_TILES * NB_PIXELS_PER_TILE); //Ahorro 16 KB ya no lo necesito
  for (int i = 0; i < NB_TILES; i++) 
  {
    uint8_t* const tile = &p->tiles[i * NB_PIXELS_PER_TILE];
    uint8_t* const rom = &p->tile_rom[i * (LEN_STRIP_BYTES * 2)];

    decode_strip(p, rom + 0, tile, 0, 4, TILE_WIDTH);
    decode_strip(p, rom + 8, tile, 0, 0, TILE_WIDTH);
  }

  // sprites are 16*16px images. in memory, they are composed of 8 strips.
  const int NB_PIXELS_PER_SPRITE = 16 * 16;
  const int SPRITE_WIDTH = 16;
  const int NB_SPRITES = 64;
  memset(p->sprites, 0, NB_SPRITES * NB_PIXELS_PER_SPRITE);
  for (int i = 0; i < NB_SPRITES; i++) {
    uint8_t* const sprite = &p->sprites[i * NB_PIXELS_PER_SPRITE];
    unsigned char* const rom = &p->sprite_rom[i * (LEN_STRIP_BYTES * 8)];

    decode_strip(p, rom + 0 * 8, sprite, 8, 12, SPRITE_WIDTH);
    decode_strip(p, rom + 1 * 8, sprite, 8, 0, SPRITE_WIDTH);
    decode_strip(p, rom + 2 * 8, sprite, 8, 4, SPRITE_WIDTH);
    decode_strip(p, rom + 3 * 8, sprite, 8, 8, SPRITE_WIDTH);

    decode_strip(p, rom + 4 * 8, sprite, 0, 12, SPRITE_WIDTH);
    decode_strip(p, rom + 5 * 8, sprite, 0, 0, SPRITE_WIDTH);
    decode_strip(p, rom + 6 * 8, sprite, 0, 4, SPRITE_WIDTH);
    decode_strip(p, rom + 7 * 8, sprite, 0, 8, SPRITE_WIDTH);
  }
}

//JJ static inline void draw_tile(pac* const p, uint8_t tile_no, uint8_t* pal, uint16_t x, uint16_t y) 
static inline void draw_tile(pac* const p, unsigned char tile_no, uint8_t* pal, unsigned short int x, unsigned short int y) 
{       
 //short int px, py;
 unsigned char color;
 //unsigned short int screenbuf_pos; 
 unsigned short int ofsTile;
  
 if (x >= PAC_SCREEN_WIDTH)
 {//Se excede de 224 varias veces, 224, 232, 65528
  //printf("draw_tile x>=%d\n",x);
  return;
 }
 
 ofsTile= (tile_no<<6); //tile_no x 64 
 for (unsigned char j=0;j<8;j++)
 {
  //screenbuf_pos= (224 * (y+j)) + x;
  for (unsigned char i=0;i<8;i++)
  {
   color = p->tiles[ofsTile++];
   //gb_jj_screen[screenbuf_pos++]= gb_jj_color[(pal[color])]; //Ya no se necesita gb_jj_screen

   //gb_buffer_vga[(y+j)][(x+i)^2]= gb_color_vga[ gb_jj_color[(pal[color])] ];
   gb_224x288[(y+j)][(x+i)]= gb_jj_color[(pal[color])]; //revisar
  }  
 }
 
// return;
/* 

 //JJ if (x < 0 || x >= PAC_SCREEN_WIDTH) 
 //JJ {
 //JJ  return;
 //JJ }

 ofsTile= (tile_no<<6); //tile_no x 64
 //for (unsigned char i = 0; i < 8 * 8; i++) 
 for (unsigned char i = 0; i < 64; i++) 
 {
  px = i % 8;
  py = i >> 3; //JJ int py = i / 8;

  //JJ color = p->tiles[tile_no * 64 + i];
  color = p->tiles[ofsTile++];
  screenbuf_pos = (y + py) * PAC_SCREEN_WIDTH + (x + px);

//Ya no lo necesito
//JJ    get_color(p, pal[color], &p->screen_buffer[screenbuf_pos * 3 + 0],
//JJ        &p->screen_buffer[screenbuf_pos * 3 + 1],
//JJ        &p->screen_buffer[screenbuf_pos * 3 + 2]);
        
    //color= jj_get_color(p,pal[color]);
    //color= gb_jj_color[(pal[color])];
    //gb_jj_screen[screenbuf_pos]= color;
  gb_jj_screen[screenbuf_pos]= gb_jj_color[(pal[color])];
    //printf("%d %d\n",screenbuf_pos,color);
 }
 */
}

//JJ static inline void draw_sprite(pac* const p, uint8_t sprite_no, uint8_t* pal, int16_t x, int16_t y, bool flip_x, bool flip_y) 
static inline void draw_sprite(pac* const p, unsigned char sprite_no, uint8_t* pal, unsigned short int x, unsigned short int y, bool flip_x, bool flip_y) 
{
  //JJ if (x <= -16 || x > PAC_SCREEN_WIDTH)
  //JJ {
  //JJ   return;
  //JJ }
  
 //unsigned char color;
 //unsigned short int offSprite;
 //unsigned short int screenbuf_pos;
 //unsigned short int base_i;
  
 if (x > PAC_SCREEN_WIDTH)
 {
  return;
 }
 
 /*
 offSprite= (sprite_no<<8);
 base_i = y * 224 + x;
 for (unsigned char j=0;j<16;j++)
 {     
  screenbuf_pos= (flip_x) ? (224 * (y+j)) + x + 16 : (224 * (y+j)) + x;
  for (unsigned char i=0;i<16;i++)
  {
   color = p->sprites[offSprite++];
   if (pal[color] == 0) 
   {
    if (flip_x)
    {
     screenbuf_pos--;     
    }
    else
    {
     screenbuf_pos++;
    }
    continue;
   }
 
   if (flip_x)
   {
    gb_jj_screen[screenbuf_pos--]= gb_jj_color[(pal[color])];              
   }   
   else
   {
    gb_jj_screen[screenbuf_pos++]= gb_jj_color[(pal[color])];
   }
  }
 }
 */
 
 
  

  //const int base_i = y * PAC_SCREEN_WIDTH + x;

  //JJ for (int i = 0; i < 16 * 16; i++) 
  for (unsigned short int i = 0; i < 256; i++) 
  {
    unsigned char px = i % 16;
    unsigned char py = i>>4; //JJ int py = i / 16;

    //JJ unsigned char color = p->sprites[sprite_no * 256 + i];
    unsigned char color = p->sprites[((unsigned short int)sprite_no << 8) + i];

    // color 0 is transparent
    if (pal[color] == 0) {
      continue;
    }

    unsigned char x_pos = flip_x ? 15 - px : px;
    unsigned char y_pos = flip_y ? 15 - py : py;
    //screenbuf_pos = base_i + y_pos * PAC_SCREEN_WIDTH + x_pos;

    if (x + x_pos < 0 || x + x_pos >= PAC_SCREEN_WIDTH) {
      continue;
    }

//Ya no lo necesito
//JJ    get_color(p, pal[color], &p->screen_buffer[screenbuf_pos * 3 + 0],
//JJ        &p->screen_buffer[screenbuf_pos * 3 + 1],
//JJ        &p->screen_buffer[screenbuf_pos * 3 + 2]);

    //color= jj_get_color(p,pal[color]);
    //color= gb_jj_color[(pal[color])];

//    gb_jj_screen[screenbuf_pos]= gb_jj_color[(pal[color])]; //Ya no se necesita gb_jj_screen

    //gb_buffer_vga[(y+y_pos)][(x+x_pos)^2]= gb_color_vga[ gb_jj_color[(pal[color])] ];
    gb_224x288[(y+y_pos)][(x+x_pos)]= gb_jj_color[(pal[color])]; //revisar
  }
  
}

static inline void pac_draw(pac* const p) 
{  
  unsigned int a0,a1,a2,a3,a32;
  unsigned short int addrIni;
  // 1. writing tiles according to VRAM

  const unsigned short int VRAM_SCREEN_BOT = 0x4000;
  //const unsigned short int VRAM_SCREEN_MID = 0x4000 + 64;
  const unsigned short int VRAM_SCREEN_TOP = 0x4000 + 64 + 0x380;

  short int x, y;
  unsigned short int i;
  unsigned char palette[4];

  // bottom of screen:
  x = 31;
  y = 34;
  i = VRAM_SCREEN_BOT;
  while (x != 31 || y != 36) 
  {
    const unsigned char tile_no = rb(p, i);
    const unsigned char palette_no = rb(p, i + 0x400);

    get_palette(p, palette_no, palette);
    draw_tile(p, tile_no, palette, ((x - 2) << 3), (y<<3));

    i += 1;
    if (x == 0) 
    {
      x = 31;
      y += 1;
    }
    else 
    {
      x -= 1;
    }
  }

  // middle of the screen:
  addrIni= 17312;
  for (y=2; y<34; y++)
  {
   i = addrIni;
   for (x=2; x<30; x++)
   {
    const unsigned char tile_no = rb(p, i);
    const unsigned char palette_no = rb(p, i + 0x400);

    get_palette(p, palette_no, palette);
    draw_tile(p, tile_no, palette, ((x - 2)<<3), (y<<3));
    i-= 32;
   }
   addrIni++;
  }  


  // top of the screen:
  x = 31;
  y = 0;
  i = VRAM_SCREEN_TOP;
  while (x != 31 || y != 2) {
    const unsigned char tile_no = rb(p, i);
    const unsigned char palette_no = rb(p, i + 0x400);

    get_palette(p, palette_no, palette);
    draw_tile(p, tile_no, palette, ((x - 2)<<3), (y<<3));

    i += 1;
    if (x == 0) 
    {
      x = 31;
      y += 1;
    } 
    else 
    {
      x -= 1;
    }
  }

  // 2. drawing the 8 sprites (in reverse order)
  const uint16_t VRAM_SPRITES_INFO = 0x4FF0;
  for (int s = 7; s >= 0; s--) {
    // the screen coordinates of a sprite start on the lower right corner
    // of the main screen:
    const short int x = PAC_SCREEN_WIDTH - p->sprite_pos[s * 2] + 15;
    const short int y = PAC_SCREEN_HEIGHT - p->sprite_pos[s * 2 + 1] - 16;

    const unsigned char sprite_info = rb(p, VRAM_SPRITES_INFO + s * 2);
    const unsigned char palette_no = rb(p, VRAM_SPRITES_INFO + s * 2 + 1);

    const bool flip_x = (sprite_info >> 1) & 1;
    const bool flip_y = (sprite_info >> 0) & 1;
    const unsigned char sprite_no = sprite_info >> 2;

    get_palette(p, palette_no, palette);
    draw_sprite(p, sprite_no, palette, x, y, flip_x, flip_y);
  }


  //Copio del doble buffer a vga scanline  
  //if ((gb_fps_cur & 0x01)==0)
  {
   unsigned char cont=0;
   unsigned int aux_ini= gb_ini_x;
   if (gb_vertical==0)
   {
    //unsigned int aux_ini= gb_ini_x>>2;    
    for (y=0;y<288;y++)
    {
     //memcpy(gb_buffer_vga[y],gb_224x288[y],224);
     cont=0;
     for (unsigned char x=0;x<56;x++)
     {
      a0= gb_224x288[y][cont++];
      a1= gb_224x288[y][cont++];
      a2= gb_224x288[y][cont++];
      a3= gb_224x288[y][cont++];
      a32= a2 | (a3<<8) | (a0<<16) | (a1<<24);
      gb_buffer_vga32[y+gb_ini_y][x+aux_ini]= a32;
     }   
    }
   }
   else
   {//vertical
    /*
    for (y=0;y<288;y++)
    {     
     cont=0;
     for (unsigned char x=0;x<224;x++)
     {
      //a0= gb_224x288[y][cont++];
      //a1= gb_224x288[y][cont++];
      //a2= gb_224x288[y][cont++];
      //a3= gb_224x288[y][cont++];
      //a32= a2 | (a3<<8) | (a0<<16) | (a1<<24);     
      gb_buffer_vga[x+gb_ini_y][((288-y)+gb_ini_x)^2]= gb_224x288[y][cont++];
     }   
    }
    */
    
    unsigned char destY;
    unsigned aux_ini_x= gb_ini_x;
    unsigned aux_ini_y= gb_ini_y;
    for (unsigned char x=0;x<224;x++)
    {
     destY=72;     
     for (unsigned short y=0;y<288;y+=4)
     {      
      a3= gb_224x288[y][x];
      a2= gb_224x288[y+1][x];
      a1= gb_224x288[y+2][x];
      a0= gb_224x288[y+3][x];
      a32= a2 | (a3<<8) | (a0<<16) | (a1<<24);
      //gb_buffer_vga32[x][72-(y>>2)]= a32; //288 DIV 4 - y DIV 4
//      gb_buffer_vga32[x][destY--]= a32; //288 DIV 4 - y DIV 4
      gb_buffer_vga32[x+aux_ini_y][(destY+gb_ini_x)]= a32; //288 DIV 4 - y DIV 4
      destY--;
     }
    }

   }
  }
}


//#ifdef use_lib_sound_ay8912  
// void sound_cycleFabgl()
// {  
//  //AY8912
//  for (unsigned char i=0;i<3;i++)
//  {
//   /*
//   if (gbVolMixer_now[i] != gbVolMixer_before[i])
//   {
//    //gb_sineArray[i].setVolume((gbVolMixer_now[i]<<2));    
//    switch (gbShiftLeftVolumen)
//    {
//      case 0: gb_sineArray[i].setVolume((gbVolMixer_now[i]<<2)); break;
//      case 1: gb_sineArray[i].setVolume((gbVolMixer_now[i]<<1)); break;
//      case 2: gb_sineArray[i].setVolume((gbVolMixer_now[i])); break;
//      case 3: gb_sineArray[i].setVolume((gbVolMixer_now[i]>>1)); break;
//      case 4: gb_sineArray[i].setVolume((gbVolMixer_now[i]>>2)); break;
//      default: gb_sineArray[i].setVolume((gbVolMixer_now[i]<<2)); break;
//    }
//    
//    gbVolMixer_before[i] = gbVolMixer_now[i];
//   }
//   */
//   //if (gbFrecMixer_now[i] != gbFrecMixer_before[i])
//   {
//    //gb_sineArray[i].setVolume(gbVolMixer_now[i]<<4);
//    gb_sineArray[i].setVolume(127);
//
//    gb_sineArray[i].setFrequency(gbFrecMixer_now[i]);
//    gbFrecMixer_before[i] = gbFrecMixer_now[i];
//   }
//  }
// }
//#endif

// generates audio for one frame
static inline void sound_update(pac* const p) 
{
 //#ifdef use_lib_sound_ay8912  
 // for (unsigned char i=0;i<3;i++)      
 // {
 //  unsigned int frec= p->sound_chip.voices[i].frequency;         
 //  //frec= frec>>6; //Para seno
 //  gbFrecMixer_now[i]= frec>>6;
 //  gbVolMixer_now[i]= ((p->sound_chip.voices[i].frequency>0)&&(p->sound_chip.voices[i].volume>0)) ? 15 : 0;   
 //  //gb_ct_Pulse[i]= ((gbFrecMixer_now[i]>0)&&(gbVolMixer_now[i]>0)) ? (unsigned int)((44100 / gbFrecMixer_now[i]))>>1 : 0;   
 // }
 //
 // sound_cycleFabgl();
 //#endif 

 #ifdef use_lib_sound_digital
  for (unsigned char i=0;i<3;i++)      
  {
   unsigned int frec= p->sound_chip.voices[i].frequency;
   #ifdef use_lib_sound_dac
    frec= frec>>6; //Para DAC analogico
   #else 
    frec= frec>>4; //Para pulsos digitales   
   #endif   
   gbFrecMixer_now[i]= frec;
   gbVolMixer_now[i]= ((p->sound_chip.voices[i].frequency>0)&&(p->sound_chip.voices[i].volume>0)) ? 15 : 0;   
   //gb_ct_Pulse[i]= ((gbFrecMixer_now[i]>0)&&(gbVolMixer_now[i]>0)) ? (unsigned int)((10000 / gbFrecMixer_now[i]))>>1 : 0; //10000 Hz
   gb_ct_Pulse[i]= ((gbFrecMixer_now[i]>0)&&(gbVolMixer_now[i]>0)) ? (unsigned int)((8000 / gbFrecMixer_now[i]))>>1 : 0; //8000 Hz
  }
   
 #endif
  /*
  if (!p->sound_enabled || p->mute_audio) {
    return;
  }

  // update the WSG (filling the audio buffer)
  wsg_play(&p->sound_chip, p->audio_buffer, p->audio_buffer_len);

  // resampling the 96kHz audio stream from the WSG into a 44.1kHz one
  float d = (float) WSG_SAMPLE_RATE / (float) p->sample_rate;
  for (int i = 0; i < p->sample_rate / PAC_FPS; i++) {
    int pos = d * (float) i;
    p->push_sample(p, p->audio_buffer[pos]);
  }
  */
}

void pac_assign_ptr(pac* const p)
{
// p->rom= gb_pac_ptr_rom;  //Ya no lo necesito apunto a FLASH
 p->ram= gb_pac_ptr_ram;
 p->tiles= gb_pac_ptr_tiles;
 p->sprites= gb_pac_ptr_sprites;
 p->tile_rom= gb_pac_ptr_tile_rom;
 p->sprite_rom= gb_pac_ptr_sprite_rom;
}

//JJ int pac_init(pac* const p) 
void pac_init(pac* const p) 
{
 gb_ptr_id_rom[0]= gb_rom_pacman6e;
 gb_ptr_id_rom[1]= gb_rom_pacman6f;
 gb_ptr_id_rom[2]= gb_rom_pacman6h;
 gb_ptr_id_rom[3]= gb_rom_pacman6j;
 gb_ptr_rom_82s1237f= gb_rom_82s1237f;

  z80_init(&p->cpu);  
  p->cpu.userdata = p;
  p->cpu.read_byte = rb;
  p->cpu.write_byte = wb;
  p->cpu.port_in = port_in;
  p->cpu.port_out = port_out;

//JJ  memset(p->rom, 0, sizeof(p->rom));
//JJ   memset(p->rom, 0, 0x4000);  //Ya no inicializo, apunto directamente a FLASH
//JJ  memset(p->ram, 0, sizeof(p->ram));
  memset(p->ram, 0, 0x1000);
  memset(p->sprite_pos, 0, sizeof(p->sprite_pos));
//JJ  memset(p->screen_buffer, 0, sizeof(p->screen_buffer)); //Ya no lo necesito
  
  //memset(gb_jj_screen, 0, sizeof(gb_jj_screen)); //Ya no se necesita

  p->int_vector = 0;
  p->vblank_enabled = 0;
  p->sound_enabled = 0;
  p->flip_screen = 0;

  // in 0 port
  p->p1_up = 0;
  p->p1_left = 0;
  p->p1_right = 0;
  p->p1_down = 0;
  p->rack_advance = 0;
  p->coin_s1 = 0;
  p->coin_s2 = 0;
  p->credits_btn = 0;

  // in 1 port
  p->board_test = 0;
  p->p1_start = 0;
  p->p2_start = 0;

  // loading rom files  
  //JJ int r = 0;
  //Leer desde flash
//  memcpy(&p->rom[0x0000], gb_rom_pacman6e, 0x1000);
//  Serial.printf("gb_rom_pacman6e\r\n");  
//  gb_ptr_id_rom[0]= gb_rom_pacman6e;
//  memcpy(&p->rom[0x1000], gb_rom_pacman6f, 0x1000);
//  Serial.printf("gb_rom_pacman6f\r\n");
//  gb_ptr_id_rom[1]= gb_rom_pacman6f;
//  memcpy(&p->rom[0x2000], gb_rom_pacman6h, 0x1000);
//  Serial.printf("gb_rom_pacman6h\r\n");
//  gb_ptr_id_rom[2]= gb_rom_pacman6h;
//  memcpy(&p->rom[0x3000], gb_rom_pacman6j, 0x1000);
//  Serial.printf("gb_rom_pacman6j\r\n");
//  gb_ptr_id_rom[3]= gb_rom_pacman6j;
  //memcpy(p->color_rom, gb_rom_82s1237f, 32);
  p->color_rom= gb_rom_82s1237f;
  for (unsigned short int i=0;i<32;i++)
  {
   unsigned char data= p->color_rom[i];
   unsigned char r= (data>>1)&0x03;
   unsigned char g= (data>>4)&0x03;
   unsigned char b= (data>>6)&0x03;
   //gb_jj_color[i]= (r<<4)|(g<<2)|(b&0x03);
   //gb_jj_color[i]= (b<<4)|(g<<2)|(r&0x03); //invierto BBGGRR
   gb_jj_color[i]= (b<<4)|(g<<2)|(r&0x03)|gb_sync_bits; //invierto BBGGRR revisar
  }
  //memcpy(p->palette_rom, gb_rom_82s1264a, 0x100);
  p->palette_rom= gb_rom_82s1264a;
  memcpy(p->tile_rom, gb_rom_pacman5e, 0x1000);
  //p->tile_rom= gb_rom_pacman5e;
  memcpy(p->sprite_rom, gb_rom_pacman5f, 0x1000);
  //p->sprite_rom= gb_rom_pacman5f;
  memcpy(p->sound_rom1, gb_rom_82s1261m, 0x100);
  //p->sound_rom1= gb_rom_82s1261m;
  
  /*JJ Antes se leia desde ficheros
  char* file0 = append_path(rom_dir, "pacman.6e");
  r += load_file(file0, &p->rom[0x0000], 0x1000);
  char* file1 = append_path(rom_dir, "pacman.6f");
  r += load_file(file1, &p->rom[0x1000], 0x1000);
  char* file2 = append_path(rom_dir, "pacman.6h");
  r += load_file(file2, &p->rom[0x2000], 0x1000);
  char* file3 = append_path(rom_dir, "pacman.6j");
  r += load_file(file3, &p->rom[0x3000], 0x1000);
  char* file4 = append_path(rom_dir, "82s123.7f");
  r += load_file(file4, p->color_rom, 32);
  char* file5 = append_path(rom_dir, "82s126.4a");
  r += load_file(file5, p->palette_rom, 0x100);
  char* file6 = append_path(rom_dir, "pacman.5e");
  r += load_file(file6, p->tile_rom, 0x1000);
  char* file7 = append_path(rom_dir, "pacman.5f");
  r += load_file(file7, p->sprite_rom, 0x1000);
  char* file8 = append_path(rom_dir, "82s126.1m");
  r += load_file(file8, p->sound_rom1, 0x100);
  // char* file9 = append_path(rom_dir, "82s126.3m");
  // r += load_file(file9, p->sound_rom2, 0x100);  

  // @TODO: mspacman
  if (0) {
    char* file6 = append_path(rom_dir, "5e");
    r += load_file(file6, p->tile_rom, 0x1000);
    char* file7 = append_path(rom_dir, "5f");
    r += load_file(file7, p->sprite_rom, 0x1000);

    char* file10 = append_path(rom_dir, "u5");
    r += load_file(file10, &p->rom[0x8000], 2048);
    char* file11 = append_path(rom_dir, "u6");
    r += load_file(file11, &p->rom[0x9000], 4096);
    char* file12 = append_path(rom_dir, "u7");
    r += load_file(file12, &p->rom[0xB000], 4096);

    free(file10);
    free(file11);
    free(file12);
  }

  free(file0);
  free(file1);
  free(file2);
  free(file3);
  free(file4);
  free(file5);
  free(file6);
  free(file7);
  free(file8);
  // free(file9);
  */

  preload_images(p);
  //p->update_screen = NULL;

  // audio
  wsg_init(&p->sound_chip, p->sound_rom1); //revisar sonido
  //JJ p->audio_buffer_len = WSG_SAMPLE_RATE / PAC_FPS; //revisar sonido
  //JJ p->audio_buffer = (int16_t *) calloc(p->audio_buffer_len, sizeof(int16_t));
  //JJ p->sample_rate = 44100; //revisar sonido
  //JJ p->mute_audio= 0; //p->mute_audio = false; //revisar sonido
  //p->push_sample = NULL;

  //Serial.printf("r %d\r\n",r);  
//JJ  return r != 0;
}

//void pac_quit(pac* const p) 
//{
////JJ  free(p->audio_buffer);
//}

// updates emulation for "ms" milliseconds.
void pac_update(pac* const p, unsigned int ms) 
{
  // machine executes exactly PAC_CLOCK_SPEED cycles every second,
  // so we need to execute "ms * PAC_CLOCK_SPEED / 1000"
  //JJ int count = 0;
  unsigned int count = 0;
  while (count < ms * PAC_CLOCK_SPEED / 1000)
  {
    //JJ int cyc = p->cpu.cyc;
    unsigned int cyc = p->cpu.cyc;
    z80_step(&p->cpu);
    //JJ int elapsed = p->cpu.cyc - cyc;
    unsigned int elapsed = p->cpu.cyc - cyc;
    count += elapsed;

    if (p->cpu.cyc >= PAC_CYCLES_PER_FRAME) {
      p->cpu.cyc -= PAC_CYCLES_PER_FRAME;

      // trigger vblank if enabled:
      if (p->vblank_enabled) {
        // p->vblank_enabled = 0;
        z80_gen_int(&p->cpu, p->int_vector);

        gb_vga_time_cur= millis();
        //if ((gb_fps_cur & 0x01) == 0)
        if ((gb_vga_time_last-gb_vga_time_cur) >= (gb_vga_cur_poll_ms-1))
        {//skip frame temporal
         gb_vga_time_last= gb_vga_time_cur;
         pac_draw(p);
        }
        //p->update_screen(p);
        sound_update(p);
        
        gb_fps_cur++;
      }
    }
  }
}

// invincibility patch (from http://cheat.retrogames.com)
/*
void pac_cheat_invincibility(pac* const p) 
{
  p->rom[0x1774 + 3] = 0x32;
  p->rom[0x1774 + 2] = 0x3c;
  p->rom[0x1774 + 1] = 0xe0;
  p->rom[0x1774 + 0] = 0xc3;

  p->rom[0x3cdf + 3] = 0x04;
  p->rom[0x3cdf + 2] = 0x20;
  p->rom[0x3cdf + 1] = 0xa7;
  p->rom[0x3cdf + 0] = 0x00;

  p->rom[0x3ce3 + 3] = 0x17;
  p->rom[0x3ce3 + 2] = 0x64;
  p->rom[0x3ce3 + 1] = 0xc3;
  p->rom[0x3ce3 + 0] = 0xaf;

  p->rom[0x3ce7 + 3] = 0x17;
  p->rom[0x3ce7 + 2] = 0x77;
  p->rom[0x3ce7 + 1] = 0xc3;
  p->rom[0x3ce7 + 0] = 0xaf;

  Serial.printf("applied invincibility patch\n");
}
*/