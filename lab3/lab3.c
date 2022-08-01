#include <lcom/lab3.h>
#include <lcom/lcf.h>

#include "i8042.h"
#include "keyboard.h"

#include <stdbool.h>
#include <stdint.h>

extern int count; // no of timer interruptions
extern int sys_inb_calls;
extern int kbc_done;

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab3/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  // lcf_log_output("/home/lcom/labs/lab3/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(kbd_test_scan)() {
  int ipc_status, r, esc = false;
  message msg;

  uint8_t bit_no;

  if ((kbc_subscribe_int(&bit_no)) != OK)
    return 1;

  uint32_t irq_set = BIT(bit_no);

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
            if (is_error()) {
              printf("Keyboard error after handling interruption\n");
              continue;
            }
            /* Increment size if we are dealing with a 2 byte scancode */
            if (two_byte_code())
              set_scancode_size(2);

            /* print code otherwise */
            else {
              kbd_print_scancode(is_make_code(), get_scancode_size(), get_scancode());
              set_scancode_size(1);
            }
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

  if ((kbc_unsubscribe_int) () != OK)
    return 1;

  kbd_print_no_sysinb(sys_inb_calls);
  sys_inb_calls = 0;

  return 0;
}

int(kbd_test_poll)() {
  bool esc = false;
  // bool next_brk = 0;      // if next = 1, a break code is coming
  uint8_t cmdByte;
  int c  = 0;

  

  while (!esc && c < 2000000) {
    if (is_error()) {
      printf("Keyboard error after handling interruption\n");
      continue;
    }

    if (kbc_polling()){
      ++c;
      //printf("I'm stuck: %d\n", c);
      continue;
    }
    if (two_byte_code()) {
      set_scancode_size(2);
    }
    else {
      kbd_print_scancode(is_make_code(), get_scancode_size(), get_scancode());
      set_scancode_size(1);
    }
    if (get_scancode()[0] == ESC_BRK_CODE)
      esc = true;
  }

  // Should read the command byte before to restore it later
  kbc_get_command_byte(&cmdByte);

  // enable interrupts by writing command byte before exiting
  cmdByte |= KBC_ENA_INT_KBD;
  kbc_set_command_byte(cmdByte);
  kbd_print_no_sysinb(sys_inb_calls);

  return 0;
}

int(kbd_test_timed_scan)(uint8_t n) {
  int ipc_status, r, terminate = false;
  message msg;

  uint8_t timer_bit_no, kbc_bit_no;

  if ((timer_subscribe_int(&timer_bit_no)) != OK)
    return 1;

  if ((kbc_subscribe_int(&kbc_bit_no)) != OK)
    return 1;

  uint32_t timer_irq_set = BIT(timer_bit_no);
  uint32_t kbc_irq_set = BIT(kbc_bit_no);

  while (!terminate) {
    /* Get a request message. */
    if ((r = driver_receive(ANY, &msg, &ipc_status))) {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) { /* Received a notification */
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:                                 /* Hardware interrupt notification */
          if (msg.m_notify.interrupts & kbc_irq_set) { /* Interrupt from the keyboard */
            count = 0;
            kbc_ih();
            /* If an error has occurred, don't print scancode */
            if (is_error()) {
              printf("Keyboard error after handling interruption\n");
              continue;
            }
            /* Increment size if we are dealing with a 2 byte scancode */
            if (two_byte_code())
              set_scancode_size(2);

            /* print code otherwise */
            else {
              kbd_print_scancode(is_make_code(), get_scancode_size(), get_scancode());
              set_scancode_size(1);
            }
            /* If the ESC key was released, exit loop */
            if (get_scancode()[0] == ESC_BRK_CODE)
              terminate = true;
          }

          if (msg.m_notify.interrupts & timer_irq_set) { /* Interrupt from the timer */
            timer_int_handler();
          }
          break;
        default:
          break; /* no other notifications expected: do nothing */
      }
    }
    else { /* received a standard message, not a notification */
           /* no standard messages expected: do nothing */
    }

    if (n * 60 <= count)
      terminate = true;
  }

  if (timer_unsubscribe_int() != OK)
    return 1;

  if ((kbc_unsubscribe_int) () != OK)
    return 1;

  return 0;
}
