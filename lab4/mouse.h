#ifndef __MOUSE_H
#define __MOUSE_H


/**
 * Interrupt handler for the mouse
 */
void (mouse_ih)();

/**
 * @brief Subscribes and enables mouses interrupts
 *
 * @param bit_no address of memory to be initialized with the
 *         bit number to be set in the mask returned upon an interrupt
 * @return Return 0 upon success and non-zero otherwise
 */

int (mouse_subscribe_int)(uint8_t *bit_no);

int (mouse_unsubscribe_int)();

int (mouse_read_out_buf)(uint8_t *byte);

int (mouse_parse_packet)(struct packet *pp);

int (mouse_send_command)(uint8_t cmd);


#endif /* __MOUSE_H */
