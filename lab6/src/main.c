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
extern long long _get_current_context();

int main(int addr, int time)
{
	printf("hihihi\n");

	long long cur_sp;
	asm volatile("mov %0, sp" : "=r"(cur_sp));
	printf("\ncur sp : %x\n", cur_sp);

	initialize(addr, time);

	init_run_queue();

	asm volatile("mov sp, %0" : : "r"(run_queue->init_thread->context.sp));
	asm volatile("msr sp_el0, %0" : : "r"((long long)run_queue->init_thread->ustack));
	printf("\nuser stack : %x\n", (long long)run_queue->init_thread->ustack);

    _core_timer_enable();

	long long cur_context;
	asm volatile("mrs %0, tpidr_el1" : "=r"(cur_context));
	printf("\nmain context : %lx\n", cur_context);

	cur_context = _get_current_context();
	printf("\ntpidr context : %lx\n", cur_context);

	sys_exec("syscall_lfb.img", 0);

	printf("\nwait a min...\n");
	//_from_el1_to_el0();

	//shell();

	return 0;
}
