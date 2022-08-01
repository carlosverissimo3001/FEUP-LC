#include <lcom/lcf.h>
#include <mouse.h>

#include "i8042.h"
#include <stdint.h>

int packet_counter = 1;
int mouse_hook_id = MOUSE_IRQ;
uint8_t packet[3];

void (mouse_ih)(){
  uint8_t byte;

  mouse_read_out_buf(&byte);

  /* Better safe than sorry */
  if (packet_counter == 1){
    if (!(byte & IS_FIRST_BYTE))
      return;
  }
  packet[++packet_counter - 1] = byte;
  return;

}

int(mouse_subscribe_int)(uint8_t *bit_no) {
  *bit_no = mouse_hook_id;

  if (sys_irqsetpolicy(MOUSE_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &mouse_hook_id) != OK) {
    printf("Error while subscribing the mouse interruptions.\n");
    return 1;
  }

  return 0;
}

int(mouse_unsubscribe_int)() {
  if (sys_irqrmpolicy(&mouse_hook_id) != OK) {
    printf("Error while unsubscribing the mouse interruptions.\n");
    return 1;
  }

  return 0;
}

int(mouse_read_out_buf)(uint8_t *byte) {
  if (util_sys_inb(KBC_OUT_BUF, byte) != OK) {
    printf("Error while reading from the output buffer\n");
    return 1;
  }
  return 0;
}

int (mouse_parse_packet)(struct packet *pp){
  memcpy(pp->bytes, packet, sizeof pp);

  pp->lb = packet[0] & LEFT_BUTTON;
  pp->mb = packet[0] & MIDDLE_BUTTON;
  pp->rb = packet[0] & RIGHT_BUTTON;

  pp->x_ov = packet[0] & X_OVF;
  pp->y_ov = packet[0] & Y_OVF;

  if (packet[0] & MSB_DELTA_X)   
    pp->delta_x = packet[1] | 0xFF00;

  else
    pp->delta_x = packet[1];


  if (packet[0] & MSB_DELTA_Y)
    pp->delta_y = packet[2] | 0xFF00;

  else
    pp->delta_y = packet[2];

  return 0;
}

int mouse_set_stream_mode(){
  return mouse_send_command(SET_STREAM_MODE);
}
int mouse_enable_data_rep(){
  return mouse_send_command(ENA_DATA_REPORT);
}
int mouse_disable_data_rep(){
  return mouse_send_command(DIS_DATA_REPORT);
}

int (mouse_send_command)(uint8_t cmd){
  sys_outb(WRITE_BYTE_TO_MOUSE, KBC_ST_REG);
  sys_outb(KBC_IN_BUF, cmd);

  return 0;
}
