#include "gpio.h"

//extern volatile unsigned int mbox[36];

#define MBOX_BASE  (MMIO_BASE+0x0000B880)
#define MBOX_READ       ((volatile unsigned int*)(MBOX_BASE+0x0))
#define MBOX_STATUS     ((volatile unsigned int*)(MBOX_BASE+0x18))
#define MBOX_WRITE      ((volatile unsigned int*)(MBOX_BASE+0x20))
#define MBOX_RESPONSE   0x80000000
#define MBOX_FULL       0x80000000
#define MBOX_EMPTY      0x40000000

#define REQUEST_CODE        0x00000000
#define REQUEST_SUCCEED     0x80000000
#define REQUEST_FAILED      0x80000001
#define TAG_REQUEST_CODE    0x00000000
#define END_TAG             0x00000000

#define GET_BOARD_REVISION  0x00010002
#define GET_ARM_MEMORY		0x00010005

/* channels */
#define MBOX_CH_POWER   0
#define MBOX_CH_FB      1
#define MBOX_CH_VUART   2
#define MBOX_CH_VCHIQ   3
#define MBOX_CH_LEDS    4
#define MBOX_CH_BTNS    5
#define MBOX_CH_TOUCH   6
#define MBOX_CH_COUNT   7
#define MBOX_CH_PROP    8

/* tags */
#define MBOX_TAG_GETSERIAL      0x10004
#define MBOX_TAG_LAST           0

void get_board_revision();
void get_arm_memory();
int mmbox_call(unsigned char ch);
