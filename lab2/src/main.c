#include "uart.h"
#include "shell.h"
#include "dtb.h"
#include <initrd.h>

int main(int addr)
{
	uart_init();
	uart_flush();
	printf("\nWelcom to Raspberry pi 3\n");
	printf("Type help to see commands\n");

	set_dtb_addr(addr);
	parse_dtb(get_cpio_addr);

	shell();

	return 0;
}
