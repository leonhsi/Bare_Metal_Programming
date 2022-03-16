#include "gpio.h"
#include "sprintf.h"
#include "uart.h"
#include "queue.h"

extern volatile unsigned char _end;

Queue* user_w_queue;
Queue* user_r_queue;

void uart_enable_ir_init(){
    *IR_EN_REG |= (1<<29);
    uart_enable_rx_ir();
}

void uart_enable_ir()
{
    *AUX_MU_IER |= 0x3;     //enable interrupt       
}

void uart_disable_ir(){
    *AUX_MU_IER &= 0xc;     //1100
}

void uart_enable_tx_ir(){
    *AUX_MU_IER |= 0xfe;    //1111 1110
}

void uart_enable_rx_ir(){
    *AUX_MU_IER |= 0xfd;    //1111 1101
}

void uart_disable_tx_ir(){
    *AUX_MU_IER &= 0xd;     //1101
}

void uart_disable_rx_ir(){
    *AUX_MU_IER &= 0xe;     //1110
}

void uart_send_async(char c){
    enqueue(user_w_queue, c);
    uart_enable_tx_ir();
}

void uart_puts_async(char *s){
    while(*s){
        if(*s == '\n') {
            enqueue(user_w_queue, *s);
        }
        enqueue(user_w_queue, *s++);
    }

    uart_enable_tx_ir();
}

char uart_getc_async(){
    
    return dequeue(user_r_queue);
}

/**
 * Set baud rate and characteristics (115200 8N1) and map to GPIO
 */
void uart_init()
{
    //init uart queue
    user_r_queue = queue_init();
    user_w_queue = queue_init();

    register unsigned int r;

    /* initialize UART */
    *AUX_ENABLE |=1;       // enable UART1, AUX mini uart
    *AUX_MU_CNTL = 0;
    *AUX_MU_LCR = 3;       // 8 bits
    *AUX_MU_MCR = 0;
    *AUX_MU_IER = 0;       
    *AUX_MU_IIR = 0xc6;    // disable interrupts
    *AUX_MU_BAUD = 270;    // 115200 baud
    /* map UART1 to GPIO pins */
    r=*GPFSEL1;
    r&=~((7<<12)|(7<<15)); // gpio14, gpio15
    r|=(2<<12)|(2<<15);    // alt5
    *GPFSEL1 = r;
    *GPPUD = 0;            // enable pins 14 and 15
    r=150; while(r--) { asm volatile("nop"); }
    *GPPUDCLK0 = (1<<14)|(1<<15);
    r=150; while(r--) { asm volatile("nop"); }
    *GPPUDCLK0 = 0;        // flush GPIO setup
    *AUX_MU_CNTL = 3;      // enable Tx, Rx
}

/**
 * Send a character
 */
void uart_send(unsigned int c) {
    /* wait until we can send */
    do{asm volatile("nop");}while(!(*AUX_MU_LSR&0x20));
    /* write the character to the buffer */
    *AUX_MU_IO=c;
}

/**
 * Receive a character
 */
char uart_getc() {
    char r;
    /* wait until something is in the buffer */
    do{asm volatile("nop");}while(!(*AUX_MU_LSR&0x01));
    /* read it and return */
    r=(char)(*AUX_MU_IO);
    /* convert carrige return to newline */
    return r=='\r'?'\n':r;
}

/**
 * Display a string
 */
void uart_puts(char *s) {
    while(*s) {
        /* convert newline to carrige return + newline */
        if(*s=='\n')
            uart_send('\r');
        uart_send(*s++);
    }
}

void uart_flush()
{
	char tmp = '1';
	tmp++;
	do{tmp = (char)(*AUX_MU_IO);}while(*AUX_MU_LSR&0x01);
}

void uart_hex(unsigned int d) {
    unsigned int n;
    int c;
    for(c=28;c>=0;c-=4) {
        // get highest tetrad
        n=(d>>c)&0xF;
        // 0-9 => '0'-'9', 10-15 => 'A'-'F'
        n+=n>9?0x37:0x30;
        uart_send(n);
    }
}

void printf(char *fmt, ...) {
    __builtin_va_list args;
    __builtin_va_start(args, fmt);
    // we don't have memory allocation yet, so we
    // simply place our string after our code
    char *s = (char*)&_end;
    // use sprintf to format our string
    vsprintf(s,fmt,args);
    // print out as usual
    while(*s) {
        /* convert newline to carrige return + newline */
        if(*s=='\n')
            uart_send('\r');
        uart_send(*s++);
    }
}

void uart_write(char *str, int count){
	for(int i=0; i<count; i++){
		if(*str == '\n') uart_send('\r');
		uart_send(*str++);
	}
}
