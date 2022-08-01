#include <lcom/lcf.h>

int (pp_subscribe_int)(uint8_t *bit_no);

int (pp_unsubscribe_int)();

void (pp_ih)(int timer);

enum l3223_time_units {
    l3223_microsecond,
    l3223_milisecond,
    l3223_second
};

