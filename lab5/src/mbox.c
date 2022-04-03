#include "gpio.h"
#include "mbox.h"
#include "uart.h"

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

/* mailbox message buffer */
volatile unsigned int  __attribute__((aligned(16))) mbox[36];

/**
 * Make a mailbox call. Returns 0 on failure, non-zero on success
 */
int mbox_call(unsigned char ch)
{
    unsigned int r = (((unsigned int)((unsigned long)&mbox)&~0xF) | (ch&0xF));
    /* wait until we can write to the mailbox */
    do{asm volatile("nop");}while(*MBOX_STATUS & MBOX_FULL);
    /* write the address of our message to the mailbox with channel identifier */
    *MBOX_WRITE = r;
    /* now wait for the response */
    while(1) {
        /* is there a response? */
        do{asm volatile("nop");}while(*MBOX_STATUS & MBOX_EMPTY);
        /* is it a response to our message? */
        if(r == *MBOX_READ)
            /* is it a valid successful response? */
            return mbox[1]==MBOX_RESPONSE;
    }
    return 0;
}

void get_board_revision()
{
	mbox[0] = 7 * 4;					//buffer size in bytes
	mbox[1] = REQUEST_CODE;
	
	//tags begin
	mbox[2] = GET_BOARD_REVISION;		//tag identifier
	mbox[3] = 4;						//max size of request and response value buffer
	mbox[4] = TAG_REQUEST_CODE;		
	mbox[5] = 0;						//value buffer

	//tags end
	mbox[6] = END_TAG;

	mbox_call(MBOX_CH_PROP);
	printf("\nBoard revision : \t\t0x%x\n", mbox[5]);
}

void get_arm_memory()
{
	mbox[0] = 8 * 4;					//buffer size in bytes
	mbox[1] = REQUEST_CODE;
	
	//tags begin
	mbox[2] = GET_ARM_MEMORY;			//tag identifier
	mbox[3] = 8;						//max size of request and response value buffer
	mbox[4] = TAG_REQUEST_CODE;		
	mbox[5] = 0;						//value buffer for base address
	mbox[6] = 0;						//value buffer for memory size

	//tags end
	mbox[7] = END_TAG;

	mbox_call(MBOX_CH_PROP);
	printf("\nARM memory base address : \t0x%x\n", mbox[5]);
	printf("ARM memory size : \t\t0x%x\n", mbox[6]);
}
