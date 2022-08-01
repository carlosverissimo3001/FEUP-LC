#ifndef _LCOM_I8042_H_
#define _LCOM_I8042_H_

#include <lcom/lcf.h>

/** @defgroup i8042 i8042
 * @{
 *
 * Constants for programming the keyboard.
 */

#define KBC_IRQ 1                 /**< @brief Keyboard controller IRQ line */
#define KBC_DELAY 20000           /**< @brief Delay> */
#define MAX_TRIES 30          

/* The KBC registers */

#define KBC_IN_BUF      0x60      /**< @brief Keyboard controller input buffer */
#define KBC_OUT_BUF     0x60      /**< @brief Keyboard controller output buffer */
#define KBC_ST_REG      0x64      /**< @brief KBC status register */

/* Status register bits */

#define KBC_PAR_ERR     BIT(7)    /**< @brief Parity error - invalid data */
#define KBC_TO_ERR      BIT(6)    /**< @brief Timeout error - invalid data */
#define KBC_MOUSE_DATA  BIT(5)    /**< @brief Mouse data */
#define KBC_IBFULL      BIT(1)    /**< @brief Input buffer full - dont write commands or arguments*/
#define KBC_OBFULL      BIT(0)    /**< @brief Output buffer full - data avaliabe for reading' */

/* Keyboard-Related KBC Commands for PC-AT/PS2 - ALL WRITEN USING THE ADRESS 0X64*/

#define KBC_SEND_CMD   0x64       /**< @brief Issue a command to the KBC */
#define KBC_SEND_ARG   0x60       /**< @brief Arguments of the command must be passed here */
#define KBC_READ_RET   0x60       /**< @brief Return values must be read from here */

#define KBC_READ_CMD_BYTE  0x20   /**< @brief Read command byte - Return value: command byte */
#define KBC_WRITE_CMD_BYTE 0x60   /**< @brief Write command byte - Argument: command byte */
#define KBC_DIS_KBD_INT    0xAD   /**< @brief Disable KBD Interface */
#define KBC_ENA_KBD_INT    0xAE   /**< @brief Enable KBD Interface */

/* KBC Command byte bits */

#define KBC_DIS_MOUSE_INT     BIT(5) /**< @brief Disable Mouse interface */
#define KBC_DIS_KBD_INT_BIT   BIT(4) /**< @brief Disable KBD interface */
#define KBC_ENA_INT_MOUSE     BIT(1) /**< @brief Enable interrupt on OBF, from mouse */
#define KBC_ENA_INT_KBD       BIT(0) /**< @brief Enable interrupt on OBF, from keyboard */


/* Some useful codes */

#define ESC_BRK_CODE    0x81      /**< @brief ESC key break code */
#define TWO_BYTE_SCODE  0xE0      /**< @brief Two-byte scandode, have 0xE0 as their first byte */


/* Useful combination of macros */
#define KBC_CHECK_ERR   (KBC_PAR_ERR | KBC_TO_ERR)      /**< @brief Mask to check for both errors */

/**@}*/

#endif /* _LCOM_I8042_H */  
