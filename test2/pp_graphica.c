#include <lcom/lcf.h>

#include "macros.h"

typedef unsigned long phys_addr_t;

typedef enum lpv_dir_t {
  horizontal,
  vertical
};

typedef struct {
  uint16_t x_res;        // horizontal resolution
  uint16_t y_res;        // vertical resolution
  uint8_t bpp;           // bits per pixel
  uint8_t r_sz;          // red component size
  uint8_t r_pos;         // red component LSB position
  uint8_t g_sz;          // green component size
  uint8_t g_pos;         // green component LSB position
  uint8_t b_sz;          // blue component size
  uint8_t b_pos;         // blue component LSB position
  phys_addr_t phys_addr; // video ram base phys address
} lpv_mode_info_t;


lpv_mode_info_t *lpv_t;
uint8_t *video_mem;
uint8_t bytes_per_pixel;

uint8_t *video_map_phys(uint32_t ph_addr, size_t len);
int lpv_get_mode_info(uint16_t mode, lpv_mode_info_t *lmi_p);

int lpv_set_mode(uint16_t mode){
  reg86_t reg86;

  reg86.intno = BIOS_ACESS_GRAPHICS;
  reg86.ax = SET_VBE_MODE;
  reg86.bx = mode | LINEAR_BUFFER;

  if (sys_int86(&reg86) != OK) {
    printf("set_vbe_mode(): sys_int86() failed \n");
    return 1;
  }

  /*  If the VBE function is supported, value 0x4f is returned in the AL */
  if (reg86.al != 0xF4) {
    printf("VBE function not supported\n");
    return 1;
  }

  /*  If the VBE function completed successfully, value 0x00 is returned in the AH */
  if (reg86.ah != 0x00) {
    printf("VBE function was not successful\n");
    printf("Cause: ");
    switch (reg86.ah) {
      case 0x01: printf("Function call failed\n"); break;
      case 0x02: printf("Function is not supported in current HW configuration\n"); break;
      case 0x03: printf("Function is invalid in current video mode"); break;
      default:
        break;
      }
    return 1;
  }

  return 0;
}


int (vg_draw_pixel)(uint16_t x, uint16_t y, uint32_t color){
  uint8_t *ptr;

  ptr = video_mem;

  ptr+= (y * lpv_t->x_res + x) * bytes_per_pixel;

  for (uint8_t i = 0; i < bytes_per_pixel; i++){
    *ptr = (color >> 8 * i);
    ptr++;
  }
}


int (vg_draw_line)(uint16_t x, uint16_t y, uint16_t len, uint32_t color, enum lpv_dir_t dir){
  if(dir == horizontal){
    for (int i = x; i < (x + len); i++){
      vg_draw_pixel(i, y, color);
    }
  }
  else if (dir == vertical){
    for (int j = y; j < (y + len); j++){
      vg_draw_pixel(x, j, color);
    }
  }
}

int (vg_test_square)(uint16_t x, uint16_t y, uint16_t len, uint32_t color){
  // Percorre o quadrado de cima para baixo (x fixo, y variavel)
  for (int j = y; j < (y + len); j++){
    // desenha linhas horizontais de x = x e y = j
    vg_draw_line(x, j, len, color, horizontal);
  }
}

int vg_test_line(uint8_t mode, enum lpv_dir_t dir, uint16_t x, uint16_t y, uint16_t len,
                 uint32_t color, uint32_t delay){

  if (lpv_get_mode_info(mode, &lpv_t) != OK){
    return 1;
  }

  bytes_per_pixel = ceil(lpv_t->bpp / 8);
  size_t vram_size = lpv_t->x_res * lpv_t->y_res * bytes_per_pixel;

  video_mem = video_map_phys(lpv_t->phys_addr, vram_size);

  if (lpv_set_mode(mode) != OK) return 1;

  vg_draw_line(x, y, len, color, dir);

  sleep(delay);

  vg_exit();

  return 1;

}