#include "macros.h"
#include <lcom/lcf.h>

#define IRQ_LINE 12
#define MAX_TRIES 3

uint8_t scancode;
int hook_id = IRQ_LINE;

uint8_t packet_bytes[3];
struct packet pack;

int util_sys_inb(int port, uint8_t *value) {
  uint32_t lst;

  if (sys_inb(port, &lst) != OK) {
    printf("Error while reading from port %d\n", port);
    return !OK;
  }
  *value = lst;
  return OK;

}

int mouse_subscribe_int(uint8_t *bit_no) {
  *bit_no = IRQ_LINE;

  if (sys_irqsetpolicy(IRQ_LINE, IRQ_REENABLE | IRQ_EXCLUSIVE, &hook_id) != OK) {
    printf("Error while subscribing <device> interruptions\n");
    return !OK;
  }
  return OK;
}

int mouse_unsubscribe_int() { return sys_irqrmpolicy(&hook_id); }

bool(buffer_is_full)(uint8_t buffer) {
  uint8_t status;

  // Reads the status register to get the KBC status
  if (utils_sys_inb(KBC_ST_REG, &status) != OK)
    return false;

  // Checks for a full buffer (either the output or input buffer)
  if (status & buffer) {
    // Aborts if there's a parity or timeout error
    if ((status & KBC_CHECK_ERR) != OK)
      return false;

    else
      return true;
  }
  // If the buffer isn't even full, returns false
  return false;
}

void mouse_ih() {
  while (1) {
    if (buffer_is_full(KBC_OBFULL)) {
      if (util_sys_inb(KBC_OUT_BUF, &scancode) != OK) {
        printf("Error while reading from the output buffer\n");
        scancode = 0x00;
      }
      break;
    }
  }
}

void mouse_assmble_packet(struct packet *pack) {
  pack->lb = pack->bytes[0] & LEFT_BUTTON;
  pack->mb = pack->bytes[0] & MIDDLE_BUTTON;
  pack->rb = pack->bytes[0] & RIGHT_BUTTON;

  pack->x_ov = pack->bytes[0] & X_OVF;
  pack->y_ov = pack->bytes[0] & Y_OVF;

  if (pack->bytes[0] & MSB_DELTA_X) {
    pack->delta_x = pack->bytes[1] | 0xff00;
  }

  else
    pack->delta_x = pack->bytes[1];

  if (pack->bytes[0] & MSB_DELTA_Y) {
    pack->delta_y = pack->bytes[2] | 0xff00;
  }

  else
    pack->delta_y = pack->bytes[2];
}

int(mouse_enable_data_report)() {
  uint8_t response;

  for (int error = 0; error < MAX_TRIES;) {
    // TELL THE KBC THAT WE WANT TO WRITE A BYTE TO THE MOUSE
    if (sys_outb(KBC_CMD_REG, WRITE_BYTE_TO_MOUSE) != OK) {
      return 1;
    }

    // WRITE TO THE MOUSE
    if (mouse_write_command(ENA_DATA_REPORT, &response) != OK)
      return 1;

    if (response == ACK)
      return OK;

    else if (response == NACK)
      error++;

    else if (response == ERROR)
      return 1;
  }
  return 1;
}

int(mouse_disable_data_report)() {
  uint8_t response;

  for (int error = 0; error < MAX_TRIES;) {
    if (sys_outb(KBC_CMD_REG, WRITE_BYTE_TO_MOUSE) != OK)
      return 1;

    if (mouse_write_command(DIS_DATA_REPORT, &response) != OK)
      return 1;

    if (response == ACK)
      return OK;

    else if (response == NACK)
      error++;

    else if (response == ERROR)
      return 1;
  }
  return 1;
}

int(mouse_write_command)(uint32_t command, uint8_t *response) {
  int attempts = 0;

  while (attempts < MAX_TRIES) {
    // If the input buffer isn't full
    if (!buffer_is_full(KBC_IN_BUF)) {
      // Writes cmd on 0x60
      if (sys_outb(KBC_WRITE_CMD_BYTE, command) != OK)
        return 1;

      // Gets the response byte from the output buffer
      if (utils_sys_inb(KBC_OUT_BUF, response) != OK)
        return 1;

      return OK;
    }

    attempts++;
  }

  return 1;
}

int(mouse_test_packet)(uint32_t cnt) {
  uint8_t bit_no;
  int ipc_status, r, size = 1;
  message msg;

  if (mouse_enable_data_report() != OK)
    return 1;
  if (mouse_subscribe_int(&bit_no) != OK)
    return 1;

  uint32_t irq_set = BIT(bit_no);

  while (cnt > 0) {
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != OK) {
      printf("driver_receive failed with: %d", r);
      continue;
    }

    if (is_ipc_notify(ipc_status)){
      switch (_ENDPOINT_P(msg.m_source)){
      case HARDWARE:
        if (msg.m_notify.interrupts & irq_set){
          mouse_ih();

          packet_bytes[size - 1] = scancode;

          if (size == 1 && (scancode & IS_FIRST_BYTE)){
            size++;
          }
          else if (size == 2){
            size++;
          }
          else if (size == 3){
            for (int i = 0; i < 3; i++){
              pack.bytes[i] = packet_bytes[i];
            }

            mouse_assmble_packet(&pack);
            mouse_print_packet(&pack);

            size = 1;
            cnt--;
          }
        }
        break;

      default:
        break;
      }
    }
    mouse_unsubscribe_int();
    mouse_disable_data_report();
  }
}