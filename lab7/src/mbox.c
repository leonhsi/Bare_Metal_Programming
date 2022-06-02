#include "mbox.h"
#include "uart.h"
#include "syscall.h"

/* mailbox message buffer */
volatile unsigned int  __attribute__((aligned(16))) mbox[36];

/**
 * Make a mailbox call. Returns 0 on failure, non-zero on success
 */
int mmbox_call(unsigned char ch)
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
	//volatile unsigned int  __attribute__((aligned(16))) mbox[36];
	mbox[0] = 7 * 4;					//buffer size in bytes
	mbox[1] = REQUEST_CODE;
	
	//tags begin
	mbox[2] = GET_BOARD_REVISION;		//tag identifier
	mbox[3] = 4;						//max size of request and response value buffer
	mbox[4] = TAG_REQUEST_CODE;		
	mbox[5] = 0;						//value buffer

	//tags end
	mbox[6] = END_TAG;

	mbox_call(MBOX_CH_PROP, mbox);
	printf("\nBoard revision : \t\t0x%x\n", mbox[5]);
}

void get_arm_memory()
{
	//volatile unsigned int  __attribute__((aligned(16))) mbox[36];
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

	mbox_call(MBOX_CH_PROP, mbox);
	//mmbox_call(MBOX_CH_PROP);
	printf("\nARM memory base address : \t0x%x\n", mbox[5]);
	printf("ARM memory size : \t\t0x%x\n", mbox[6]);
}
