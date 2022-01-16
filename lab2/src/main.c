#include "uart.h"
#include "shell.h"

int main()
{
	uart_init();
	uart_flush();
	printf("\nWelcom to Raspberry pi 3\n");
	printf("type help to see commands\n");
	shell();

	return 0;
}
