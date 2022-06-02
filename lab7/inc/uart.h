#ifndef UART_H
#define UART_H

/* Auxilary mini UART registers */
#define AUX_ENABLE      ((volatile unsigned int*)(MMIO_BASE+0x00215004))
#define AUX_MU_IO       ((volatile unsigned int*)(MMIO_BASE+0x00215040))
#define AUX_MU_IER      ((volatile unsigned int*)(MMIO_BASE+0x00215044))
#define AUX_MU_IIR      ((volatile unsigned int*)(MMIO_BASE+0x00215048))
#define AUX_MU_LCR      ((volatile unsigned int*)(MMIO_BASE+0x0021504C))
#define AUX_MU_MCR      ((volatile unsigned int*)(MMIO_BASE+0x00215050))
#define AUX_MU_LSR      ((volatile unsigned int*)(MMIO_BASE+0x00215054))
#define AUX_MU_MSR      ((volatile unsigned int*)(MMIO_BASE+0x00215058))
#define AUX_MU_SCRATCH  ((volatile unsigned int*)(MMIO_BASE+0x0021505C))
#define AUX_MU_CNTL     ((volatile unsigned int*)(MMIO_BASE+0x00215060))
#define AUX_MU_STAT     ((volatile unsigned int*)(MMIO_BASE+0x00215064))
#define AUX_MU_BAUD     ((volatile unsigned int*)(MMIO_BASE+0x00215068))

#define IR_EN_REG      ((volatile unsigned int*)(MMIO_BASE+0x0000b210))

void uart_init();

void uart_send(unsigned int c);
char uart_getc();
void uart_puts(char *s);
void uart_flush();
void uart_hex(unsigned int d);
void printf(char *fmt, ...);
void uart_write(char *str, int count);

//async
void uart_enable_ir();
void uart_enable_ir_init();
void uart_disable_ir();
void uart_enable_tx_ir();
void uart_enable_rx_ir();
void uart_disable_tx_ir();
void uart_disable_rx_ir();
char uart_getc_async();
void uart_send_async(char c);
void uart_puts_async(char *s);

#endif