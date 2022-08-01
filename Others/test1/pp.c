#include <lcom/lcf.h>
#include "pp.h"


//#define BIT(n)          (1 << (n))
#define L3223_CTRL      0x23 /**< @brief Control register */
#define TIMER_0         0x20 /**< @brief Timer 0 count register */
#define TIMER_1         0x21 /**< @brief Timer 1 count register */
#define TIMER_2         0x22 /**< @brief Timer 2 count register */

#define IRQ_LINE        0x10

/* Writing from control register */
#define TIMER_SEL(n)    (n << 6)
#define TIMER_ALARM     BIT(4)

/* Reading from control register */
#define TIMER_INT(n)    BIT(n) /**< @brief Indicates a pending interruption from timer n*/

int hook_id = IRQ_LINE;
int alarm = 0;


int main(){
    /* Podia ser second ou milisecond */
    enum l3223_time_units units = l3223_microsecond;

    pp_test_alarm(0, 10, units);
}


int (pp_test_alarm)(int timer, int interval, enum l3223_time_units units){
    uint8_t controlWord = TIMER_SEL(timer) | TIMER_ALARM | units;

    if (sys_outb(L3223_CTRL, controlWord) != OK){
        printf("Error while writing to the control register\n");
        return 1;
    }

    uint8_t msb, lsb;

    util_get_LSB(interval, &lsb);
    util_get_MSB(interval, &msb);

    if (sys_outb(TIMER_0 + timer, lsb) != OK){
        printf("Error while writing to the timer %d\n", timer);
        return 1;
    }

    if (sys_outb(TIMER_0 + timer, msb) != OK){
        printf("Error while writing to the timer %d\n", timer);
        return 1;
    }

    uint8_t bit_no;
    uint32_t irq_set;
    int end = false;
    int ipc_status, r;
    message msg;

    pp_subscribe_int(&bit_no);

    irq_set = BIT(bit_no);

    while(!end) { /* You may want to use a different condition */
    /* Get a request message. */
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) { /* received notification */
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE: /* hardware interrupt notification */
          if (msg.m_notify.interrupts & irq_set) { /* subscribed interrupt */
            pp_ih(timer);
            if (alarm){
              pp_print_alarm(timer, interval, units);
              end = true;
            }
          }
          break;
        default:
          break;  /* no other notifications expected: do nothing */
      }
    } else { /* received a standard message, not a notification */
       /* no standard messages expected: do nothing */
    }
  }

  pp_unsubscribe_int();
  return 0;

}

int (pp_subscribe_int)(uint8_t *bit_no){
    *bit_no = hook_id;
    if (sys_irqsetpolicy(IRQ_LINE, IRQ_REENABLE, &hook_id) != OK){
        printf("Error while subscribing interruptions\n");
        return 1;
    }
    return 0;
}

int (pp_unsubscribe_int)(){
    if (sys_irqrmpolicy(&hook_id) != OK){
        printf("Error while unsubscribing interruptions\n");
        return 1;
    }
    return 0;
}

void (pp_ih)(int timer){
    /* Read from 0x23*/
    uint8_t st;

    if (util_sys_inb(L3223_CTRL, &st) != OK){
        printf("Error while reading from the control register\n");
        return;
    }

    if (st & TIMER_INT(timer))
        alarm = 1;

    return;
}

int (util_get_LSB)(uint16_t val, uint8_t *lsb){
    *lsb = val & 0x00FF;
    // 1000 0101 1011 1111
    //&0000 0000 1111 1111
    //&0000 0000 1011 1111
    return 0;
}

int (util_get_MSB)(uint16_t val, uint8_t *msb){
    *msb = (val & 0xFF00) >> 8;
    // 1000 0101 1011 1111
    //&1111 1111 0000 0000
    //&1011 1111 0000 0000 >> 8 -> 0000 0000 1011 1111
    return 0;
}

int (util_sys_inb)(int port, uint8_t *value){
  uint32_t st;

  if (sys_inb(port, &st) != OK){
    printf("Error while reading from port: %d\n", port);
    return !OK;
  }

  else{
    *value = st;
    return OK;
  }

  return !OK;
}