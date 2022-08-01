#ifndef __GRAPHICS_H
#define __GRAPHICS_H

#include <lcom/lcf.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * @param mode desired graphics mode
 * @return Returns 1 if the mode was successfulyy set up, 0 otherwise
 */
int (set_graphics_mode)(uint16_t mode);

/**
 * @brief Draws a pixel on the screen in the specified coordinates
 * @param x x coordinate
 * @param y y coordinate
 * @param color color of the pixel
 * @return Return 1 if an error occurred, 0 otherwise
 */
int(vg_draw_pixel)(uint16_t x, uint16_t y, uint32_t color);

int (map_vram)(uint8_t mode);

/**
 * @brief Maps the VRAM and sets the graphic mode passed in the argument
 * @param mode desired graphics mode
 * @return 1 if the mode has been successfully set, 0 otherwise
 */
int (vgcard_init)(uint16_t mode);


/**
 * @brief 
 * @param x x coordinate
 * @param y y coordinate
 * @param img
 */
int (draw_pixmap)(uint16_t x, uint16_t y, xpm_image_t *img);

#endif /* __GRAPHICS_H */
