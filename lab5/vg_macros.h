#ifndef _LCOM_VG_MACROS_H_
#define _LCOM_VG_MACROS_H_

#include <lcom/lcf.h>

/** @defgroup vg_macros.h
 * @{
 *
 * Constants for programming the graphics card.
 */

#define BIOS_ACESS_GRAPHICS 0x10     /**< @brief BIOS service interrupt vector for graphics card> */

/* VBE MODES */
#define INDEXED_MODE    0x105 /**< @brief 1024*768, 8 bytes per pixel */
#define DIRECT_MODE_15  0x110 /**< @brief 640*480, 15 bytes per pixel ((1:)5:5:5) */
#define DIRECT_MODE_24  0x115 /**< @brief 800x600, 24 bytes per pixel (8:8:8) */
#define DIRECT_MODE_16  0x11A /**< @brief 1280x1024, 16 bytes per pixel (5:6:5) */
#define DIRECT_MODE_32  0x14C /**< @brief 1152x864, 32 bytes per pixel ((8:)8:8:8) */

/* Values passed toreg86_t registers */

#define GET_CONTROLLER_INFO 0X4F00     /**< @brief VBE call, function 00 -- Return Controller Info */
#define GET_VBE_MODE        0x4F01     /**< @brief VBE call, function 01 -- Return VBE Mode Info */
#define SET_VBE_MODE        0x4F02     /**< @brief VBE call, function 02 -- set VBE mode */

#define LINEAR_BUFFER       BIT(14)    /**< @brief bit 14: linear framebuffer */

/**@}*/

#endif /* _LCOM_VG_MACROS_H */
