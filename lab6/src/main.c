#include "uart.h"
#include "shell.h"
#include "dtb.h"
#include "initrd.h"
#include "excp.h"
#include "utils.h"
#include "queue.h"
#include "time.h"
#include "irq.h"
#include "buddy.h"
#include "thread.h"
#include "syscall.h"
#include "mm.h"

void initialize(int addr, int time){
	
	uart_init();
	uart_flush();

	printf("\nWelcom to Raspberry pi 3\n");

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
	parse_dtb(get_initrd_start, "chosen");

	get_boot_time(time);
	init_timer_queue();
	init_task_queue();

	init_page_array();
	init_frame_list();
	init_dynamic_memory();
	//startup_allocator();
}

extern void _core_timer_enable();
extern void _from_el1_to_el0();
extern size_t _get_current_context();

int main(int addr, int time)
{
	initialize(addr, time);
	init_run_queue();

	// size_t cur_context;
	// asm volatile("mrs %0, tpidr_el1" : "=r"(cur_context));
	// printf("\nmain context : %x\n", cur_context);

	// cur_context = _get_current_context();
	// printf("\ntpidr context : %x\n", cur_context);

	_core_timer_enable();

	sys_exec("vm.img", 0);

	printf("\nwait a min...\n");
	//_from_el1_to_el0();

	//shell();

	return 0;
}
