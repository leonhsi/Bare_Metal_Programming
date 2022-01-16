void uart_init();
void uart_send(unsigned int c);
char uart_getc();
void uart_puts(char *s);
void uart_flush();
void uart_hex(unsigned int d);
void printf(char *fmt, ...);
int strcmp(char *str1, char *str2);

