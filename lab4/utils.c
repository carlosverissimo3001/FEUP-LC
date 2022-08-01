#include <lcom/lcf.h>
#include "i8254.h"

#include <stdint.h>

int(util_get_LSB)(uint16_t val, uint8_t *lsb) {
  *lsb = val & EXTRACT_LSB;
  return 0;
}

int(util_get_MSB)(uint16_t val, uint8_t *msb) {
  *msb = (val & EXTRACT_MSB) >> 8;                 
  return 0;
}

int (util_sys_inb)(int port, uint8_t *value) {
  uint32_t lst;
  if (sys_inb(port, &lst) != OK){
    printf("Error in util_sys_inb()\n");
    return 1;
  }
  else {
    *value = lst;
    return 0;
  }
}
