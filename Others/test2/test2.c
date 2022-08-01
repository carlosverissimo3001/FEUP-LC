#include <lcom/lcf.h>

typedef unsigned long phys_addr_t;
#define IRQ_LINE 12
#define BIT(n) (1 << (n))

int hook_id = IRQ_LINE;

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

enum lpv_dir_t {
  lpv_hor, // horizontal line
  lpv_vert // vertical line
};

lpv_mode_info_t *lmi_p;
uint8_t *video_mem;

int lpv_get_mode_info(uint16_t mode, lpv_mode_info_t *lmi_p);
int lpv_set_mode(uint16_t mode);

int draw_pixel(uint16_t x, uint16_t y, uint32_t color) {
  if (x < 0 || y < 0 || x > lmi_p->x_res || y > lmi_p->y_res)
    return 1;

  uint8_t *ptr = video_mem;
  uint8_t bytes_per_pixel = ceil(lmi_p->bpp / 8.0);
  uint8_t i;

  ptr += (lmi_p->x_res * y + x) * bytes_per_pixel;

  for (i = 0; i < bytes_per_pixel; i++) {
    *ptr = (color >> i * 8);
    ptr++;
  }
  return 0;
}

int draw_line(uint16_t x, uint16_t y, uint16_t len, uint32_t color, enum lpv_dir_t dir){
  if (dir == lpv_hor){
    for (int i = x; i < (x + len); i++) {
        draw_pixel(i, y, color);
    }
  }
  else if (dir == lpv_vert){
    for (int j = y; j < (y + len); j++){
      draw_pixel(x, j, color);
    }
  }
}

int pp_test_line(uint8_t mode, enum lpv_dir_t dir, uint16_t x, uint16_t y,
                 uint16_t len, uint32_t color, uint32_t delay) {

  if (lpv_get_mode_info(mode, &lmi_p) != OK) {
    printf("Error getting lpv_mode_info_t\n");
    exit(EXIT_FAILURE);
  }

  size_t vram_size = lmi_p->x_res * lmi_p->y_res * ceil(lmi_p->bpp / 8.0);

  /* Map video memory */
  video_mem = video_map_phys(lmi_p->phys_addr, vram_size);

  /* Set the mode */
  if (lpv_set_mode(mode) != OK) {
    printf("Error setting the graphics mode\n");
    exit(EXIT_FAILURE);
  }

  draw_line(x, y, len, color, dir);

  sleep(delay);

  vg_exit();
}









