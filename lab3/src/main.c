#include "uart.h"
#include "shell.h"
#include "dtb.h"
#include "initrd.h"
#include "excp.h"
#include "utils.h"
#include "queue.h"
#include "time.h"
#include "irq.h"

int main(int addr, int time)
{
	uart_init();
	uart_flush();

	// printf("\nWelcom to Raspberry pi 3\n");

	// uart_enable_ir_init();

	// uart_puts_async("qqqqqqqqqqq\n");

	// uart_puts_async("\r# ");

	// char c;
	// while( (c = uart_getc_async()) != '\n'){
	// 	if(c != '\0'){
	// 		uart_send_async(c);
	// 	}
	// }

	set_dtb_addr(addr);
	parse_dtb(get_cpio_addr);
 
	get_boot_time(time);
	init_timer_queue();
	init_task_queue();

	shell();

	return 0;
}
