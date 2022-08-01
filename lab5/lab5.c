#include <lcom/lab5.h>
#include <lcom/lcf.h>
#include <lcom/timer.h>

#include <stdint.h>
#include <stdio.h>

#include "graphics.h"
#include "i8042.h"
#include "keyboard.h"
#include "vg_macros.h"

extern int count;

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab5/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab5/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(video_test_init)(uint16_t mode, uint8_t delay) {
  if (set_graphics_mode(mode) != OK) {
    printf("Error while seting the graphics mode %d\n", mode);
    return !OK;
  }

  sleep(delay);

  vg_exit();

  return 0;
}

int(video_test_rectangle)(uint16_t mode, uint16_t x, uint16_t y,
                          uint16_t width, uint16_t height, uint32_t color) {
  int esc = false, irq_set, r, ipc_status;
  message msg;

  uint8_t bit_no;
  kbc_subscribe_int(&bit_no);
  irq_set = BIT(bit_no);

  /* Maps the VRAM and sets the graphics mode */
  if (vgcard_init(mode) != OK) {
    printf("Error in vg_init()\n");
    return 1;
  }

  vg_draw_rectangle(x, y, width, height, color);

  while (!esc) {
    if ((r = driver_receive(ANY, &msg, &ipc_status))) {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) { /* received notification */
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:                             /* hardware interrupt notification */
          if (msg.m_notify.interrupts & irq_set) { /* Interrupt from the keyboard */
            kbc_ih();
            /* If an error has occurred, don't print scancode */
            if (is_error())
              continue;

            /* Increment size if we are dealing with a 2 byte scancode */
            if (two_byte_code())
              set_scancode_size(2);

            /* print code otherwise */
            else
              set_scancode_size(1);

            /* If the ESC key was released, exit loop */
            if (get_scancode()[0] == ESC_BRK_CODE)
              esc = true;
          }
          break;
        default:
          break; /* no other notifications expected: do nothing */
      }
    }
    else { /* received a standard message, not a notification */
           /* no standard messages expected: do nothing */
    }
  }

  kbc_unsubscribe_int();
  vg_exit();

  return 0;
}

int(video_test_pattern)(uint16_t mode, uint8_t no_rectangles, uint32_t first, uint8_t step) {
  /* To be completed */
  printf("%s(0x%03x, %u, 0x%08x, %d): under construction\n", __func__,
         mode, no_rectangles, first, step);

  return 1;
}

int(video_test_xpm)(xpm_map_t xpm, uint16_t x, uint16_t y) {
  /* Maps the VRAM and sets the graphics mode */
  if (vgcard_init(INDEXED_MODE) != OK) {
    printf("Error in vg_init()\n");
    return 1;
  }

  int esc = false, irq_set, r, ipc_status;
  message msg;

  uint8_t bit_no;
  kbc_subscribe_int(&bit_no);
  irq_set = BIT(bit_no);

  xpm_image_t img;
  uint8_t *map;

  map = xpm_load(xpm, XPM_INDEXED, &img);

  if (map == NULL) {
    printf("Error while reading from pixmap\n");
    return 1;
  }

  draw_pixmap(x, y, &img);



  while (!esc) {
    if ((r = driver_receive(ANY, &msg, &ipc_status))) {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) { /* received notification */
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:                             /* hardware interrupt notification */
          if (msg.m_notify.interrupts & irq_set) { /* Interrupt from the keyboard */
            kbc_ih();
            /* If an error has occurred, don't print scancode */
            if (is_error())
              continue;

            /* Increment size if we are dealing with a 2 byte scancode */
            if (two_byte_code())
              set_scancode_size(2);

            /* print code otherwise */
            else
              set_scancode_size(1);

            /* If the ESC key was released, exit loop */
            if (get_scancode()[0] == ESC_BRK_CODE)
              esc = true;
          }
          break;
        default:
          break; /* no other notifications expected: do nothing */
      }
    }
    else { /* received a standard message, not a notification */
           /* no standard messages expected: do nothing */
    }
  }

  kbc_unsubscribe_int();
  vg_exit();

  return 0;
}

int(video_test_move)(xpm_map_t xpm, uint16_t xi, uint16_t yi, uint16_t xf, uint16_t yf,
                     int16_t speed, uint8_t fr_rate) {

  return 0;
}
