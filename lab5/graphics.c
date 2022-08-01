#include <lcom/lab5.h>
#include <lcom/lcf.h>

#include "vg_macros.h"
#include <graphics.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

static uint8_t *video_mem; /* Process (virtual) address to which VRAM is mapped */
static vbe_mode_info_t vbe_info;

int(set_graphics_mode)(uint16_t mode) {
  /* Use the VESA Video Bios Extension (VBE) */
  reg86_t reg86;

  /* To avoid unexpected behavior, clear the ununsed registers */
  bzero(&reg86, sizeof reg86);

  reg86.ax = SET_VBE_MODE;
  reg86.bx = mode | LINEAR_BUFFER;
  reg86.intno = BIOS_ACESS_GRAPHICS;

  if (sys_int86(&reg86) != OK) {
    printf("set_vbe_mode(): sys_int86() failed \n");
    return 1;
  }

  /*  If the VBE function completed successfully, value 0x00 is returned in the AH */
  if (reg86.ah != 0x00) {
    printf("VBE function was NOT completed successfully\n");
    return 1;
  }

  return 0;
}

int(vg_draw_pixel)(uint16_t x, uint16_t y, uint32_t color) {
  uint8_t *ptr = video_mem;
  uint8_t i;

  unsigned int width = vbe_info.XResolution;
  unsigned int bytes_per_pixel = vbe_info.BytesPerScanLine / width;

  ptr += bytes_per_pixel * (y * width + x);

  for (i = 0; i < bytes_per_pixel; i++) {
    *ptr = (color >> 8 * i);
    ptr++;
  }

  return 0;
}

int(vg_draw_hline)(uint16_t x, uint16_t y, uint16_t len, uint32_t color) {
  // Draw pixels to form a line, from the left to the right of the screen
  unsigned int width = vbe_info.XResolution;

  for (uint16_t i = x; i < (x + len); i++) {
    if (i >= width)
      break;
    vg_draw_pixel(i, y, color);
  }
  return 0;
}

int(vg_draw_rectangle)(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color) {
  /* Draw horizontal lines from left to right, top to bottom*/

  unsigned int screen_height = vbe_info.YResolution;

  for (uint16_t i = y; i < (height + y); i++) {
    // if i is greater or equal than the vertical resolution, break
    if (i >= screen_height)
      break;
    // x is x, y is (y + delta), len is width
    vg_draw_hline(x, i, width, color);
  }
  return 0;
}

int(map_vram)(uint8_t mode) {
  int r;

  /* Allow memory mapping */
  struct minix_mem_range mr;

  mr.mr_base = (phys_bytes) vbe_info.PhysBasePtr;

  unsigned int width = vbe_info.XResolution;
  unsigned int height = vbe_info.YResolution;
  unsigned int bytes_per_pixel = vbe_info.BytesPerScanLine / width;
  unsigned int vram_size = width * height * bytes_per_pixel;

  mr.mr_limit = mr.mr_base + vram_size;

  if (OK != (r = sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mr)))
    panic("sys_privctl (ADD_MEM) failed: %d\n", r);

  /* Map memory */
  video_mem = vm_map_phys(SELF, (void *) mr.mr_base, vram_size);

  if (video_mem == MAP_FAILED)
    panic("couldn't map video memory");

  return 0;
}

int vgcard_init(uint16_t mode) {
  /* Obtain VBE mode information */

  bzero(&vbe_info, sizeof(vbe_info));


  if (vbe_get_mode_info(mode, &vbe_info) != OK) {
    printf("Error while getting VBE mode information\n");
    return 1;
  }

  /* Map the graphics mode VRAM into the process' address space */
  if (map_vram(mode) != OK) {
    printf("Error while mapping the VRAM into the process' adress space\n");
    return 1;
  }

  /* set the mode */
  if (set_graphics_mode(mode) != OK) {
    printf("Error setting the interface to the desired graphics mode\n");
    return 1;
  }

  return 0;
}

int(draw_pixmap)(uint16_t x, uint16_t y, xpm_image_t *img) {
  unsigned int screen_width = vbe_info.XResolution;
  unsigned int screen_height = vbe_info.YResolution;

  //unsigned int bytes_per_pixel = vbe_info.BytesPerScanLine / width;
  //printf("x: %d, y: %d\n", x, y);
  for (unsigned int i = 0; i < img->width; i++) {
    if ((x+i) >= screen_width)
      break;
    for (unsigned int j = 0; j < img->height; j++) {
      if ((j+y) >= screen_height)
        break;
      vg_draw_pixel((x + i), (y + j), *(img->bytes + j*img->width + i));
    }
  }

  return 0;
}


