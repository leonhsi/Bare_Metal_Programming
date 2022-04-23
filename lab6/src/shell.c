#include "uart.h"
#include "utils.h"
#include "mbox.h"
#include "reboot.h"
#include "loader.h"
#include "initrd.h"
#include "dtb.h"
#include "excp.h"
#include "time.h"
#include "buddy.h"
#include "thread.h"
#include "syscall.h"
#include "shell.h"

extern void _user();
extern void _from_el1_to_el0();
extern long long _get_current_context();

void hello()
{
	printf("\nhello world\n");
}

void help()
{
	printf("\n\nhelp :\t\tprint help menu\n");
	printf("hello :\t\tprint hello world\n");
	printf("reboot :\treboot device\n");
	printf("board : \tprint board revision\n");
	printf("mem : \t\tprint arm memory base address and size\n");
	printf("lk : \t\tload new kernel\n");
	printf("ls : \t\tparse cpio archive\n");
	printf("cat : \t\tshow cpio file content\n");
	printf("cel : \t\tshow current exception level, cannot work at el0 since system register\n");
	printf("sel0 : \t\tswitch from el1 to el0, and set timeout 2 sec later\n");
	printf("user : \t\tsystem call to print spsr_el1, elr_el1, esr_el1 and far_el1 for five times\n");
	printf("time : \t\ttime <MESG> <SEC>, set time out after <SEC> and print <MESG>\n");
	printf("page : \t\tpage <PAGES>, allocates <PAGES> pages\n");
	printf("free : \t\tfree <ADDR>, free allocated pages at <ADDR>\n");
	printf("dma : \t\tdma <BYTES>, Dynamic Memory Allocate <BYTES> on page\n");
	printf("fdma : \t\tfdma <ADDR>, free allocated dynamic memory chunk on <ADDR>\n");
	printf("\n");
}

void reboot()
{
	printf("\nrebooting ... \n");
	reset(0);

	//use a while loop to do nothing or it will keep reading commands
	while(1){}
}

void set_timer_interface(char * cmd){
	//printf("\n%s\n", cmd);

	int index = 5;
	char *message = "";
	strclear(message);
	//printf("%s\n", message);

	while(cmd[index] != ' '){
		message[index-5] = cmd[index];
		//printf("%c %c\n", message[index-5], cmd[index]);
		++index;
	}
	message[index] = '\0';

	int sec = 0;
	++index;
	while(cmd[index] != '\0'){
		sec = sec*10 + (cmd[index] - '0');
		//printf("%d\n", cmd[index] - '0');
		++index;
	}

	//printf("message : %s\n", message);
	//printf("sec : %d \n", sec);

	setTimeout(message, sec);
}

void alloc_page_interface(char *cmd){
	int index = 5;
	
	int size = 0;
	while(cmd[index] != '\0'){
		size = size * 10 + (cmd[index] - '0');
		index++;
	}

	printf("\n\nrequested page number : [%d]\n", size);

	//decide number of pages to be allocated
	int alloc_size = (size == 1) ? 1 : 2;
	int tmp_size = size;
	while(tmp_size > 1){
		if(alloc_size == size) break;
		tmp_size >>= 1;
		alloc_size <<= 1;
	}

	char *page_ptr = "";
	strclear(page_ptr);
	page_ptr = alloc_page(alloc_size);
	printf("\nreceive page point at %x\n\n", (char*)page_ptr);
}

void free_page_interface(char *cmd){
	int index = 5;

	int len = index;
	while(cmd[len] != '\0'){
		len++;
	}
	len -= index;

	char *free_addr = simple_malloc(len+1);
	
	while(cmd[index] != '\0'){
		free_addr[index-5] = cmd[index];
		index++;
	}
	free_addr[index] = '\0';

	printf("free addr : %s\n", free_addr);

	printf("\naddr : %s\n", free_addr);
	free_pages(free_addr);
}

void dynamic_memory_alloc_interface(char *cmd){
	int index = 4;
	
	int bytes = 0;
	while(cmd[index] != '\0'){
		bytes = bytes*10 + (cmd[index] - '0');
		index++;
	}

	//round up bytes to multiple of 16
	int round_bytes = 16;

	while(round_bytes < bytes){
		round_bytes *= 2;
	}

	char *page_ptr = allocate_dynamic_memory(round_bytes);

	printf("\nreceived page address : %x\n\n", page_ptr);
}

void free_dynamic_memory_interface(char *cmd){
	int index = 5;
	
	int len = index;
	while(cmd[len] != '\0'){
		len++;
	}
	len -= index;

	char *free_addr = simple_malloc(len+1);
	
	while(cmd[index] != '\0'){
		free_addr[index-5] = cmd[index];
		index++;
	}
	free_addr[index] = '\0';

	free_dynamic_memory(free_addr);	
}

void create_thread_interface(){
	for(int i=0; i<5; i++){
		create_thread(foo);
	}
	idle();
}

void system_call_interface(char *cmd){
	int index = 4;
	int sys_no = cmd[index] - '0';

	switch(sys_no){
		case 0:
			asm volatile("svc #0");
			break;
		case 1:
			//printf("\n111\n");
			asm volatile("svc #1");
			break;
		case 2:
			asm volatile("svc #2");
			break;
		default:
			printf("\nNo.%d system call not implemented\n", sys_no);
			break;
	}
}

void fork_test_interface(){
	//run_queue->init_thread.task = (fork_test);
	fork_test();
}

void test_uart(){
	char *wr = "helloooooooooo\n";
	uartwrite(wr, 5);

	char *read = "";
	uartread(read, 3);
	printf("\n====== %s\n", read);
}

void exec_interface(){

	if(fork() == 0){
		exec("syscall.img");
	}
}

void test_signal(){
	if(fork() == 0){
		while(1){

		}
	}
	else{
		kill(0, SIGKILL);
	}
}

void command_not_found(char *cmd)
{
	printf("\nCommand not found : %s\n", cmd);
}

void shell_input(char *cmd) {
    uart_puts("\r# ");
    char c;
    int idx = 0, end = 0;
    cmd[0] = '\0';
    while((c = uart_getc()) != '\n') {
        if(c == 8 || c == 127) {
            if (idx > 0) {
                idx--;
                for (int i = idx; i < end; i++) {
                    cmd[i] = cmd[i + 1];
                }
                cmd[--end] = '\0';
            }
        }
        else if (c == 3) {
            cmd[0] = '\0';
			break;
		}
		else if(c == 27) {
			c = uart_getc();
			c = uart_getc();
			switch (c) {
				case 'C' :
					if (idx < end) idx++;
					break;
				case 'D' :
					if (idx > 0) idx--;
					break;
				default : uart_flush();
			}
		}
		else {
            if (idx < end) {
                for (int i = end; i > idx; i--) {
                    cmd[i] = cmd[i - 1];
                }
            }
            cmd[idx++] = c;
            cmd[++end] = '\0';
        }
        printf("\r# %s \r\e[%dC", cmd, idx + 2);
        //printf("%d %d\n", idx, end);
        //printf("\r# %s \r", cmd);
    }
}

void shell_input_async(char *cmd) {
	// uart_puts_async("\r# ");
    // char c;
    // int idx = 0, end = 0;
    // cmd[0] = '\0';
    // while( (c = uart_getc_async()) != '\n' ) {
	// 	if(c != '\0'){
	// 		if (idx < end) {
    //             for (int i = end; i > idx; i--) {
    //                 cmd[i] = cmd[i - 1];
    //             }
    //         }
	// 		cmd[idx++] = c;
	// 		cmd[++end] = '\0';
	// 		uart_send_async(c);
	// 	}
    // }

	uart_puts_async("\r# ");
    char c;
    int idx = 0, end = 0;
    cmd[0] = '\0';
    while((c = uart_getc_async()) != '\n') {
        if(c == 8 || c == 127) {
            if (idx > 0) {
                idx--;
                for (int i = idx; i < end; i++) {
                    cmd[i] = cmd[i + 1];
                }
                cmd[--end] = '\0';
            }
        }
        else if (c == 3) {
            cmd[0] = '\0';
			break;
		}
		else {
            if (idx < end) {
                for (int i = end; i > idx; i--) {
                    cmd[i] = cmd[i - 1];
                }
            }
            cmd[idx++] = c;
            cmd[++end] = '\0';
        }

		uart_send_async(c);
    }
}

void shell()
{
	char cmd[100];

	//char *string = simple_malloc(8);
	//char *string2 = simple_malloc(10);

	//string = "abasdf";
	//string2 = "1234500";

	//printf("%s\n", string);
	//printf("%s\n", string2);

	while(1)
	{
		shell_input(cmd);
		//uart_enable_ir_init();
		//shell_input_async(cmd);

		if( strcmp(cmd, "hello") == 0)			hello();
		else if( strcmp(cmd, "help") == 0)		help();
		else if( strcmp(cmd, "reboot") == 0)	reboot();
		else if( strcmp(cmd, "board") == 0)		get_board_revision();
		else if( strcmp(cmd, "mem") == 0)		get_arm_memory();
		else if( strcmp(cmd, "lk") == 0)		copy_old_kernel();
		else if( strcmp(cmd, "ls") == 0)		parse_cpio_name();
		else if( strcmp(cmd, "cat") == 0)		parse_cpio_file();
		else if( strcmp(cmd, "cel") == 0)		get_current_el();
		else if( strcmp(cmd, "sel0") == 0 )		_from_el1_to_el0();
		else if( strcmp(cmd, "user") == 0 )		_user();
		else if( strncmp(cmd, "time", 4) == 0)	set_timer_interface(cmd);
		else if( strncmp(cmd, "page", 4) == 0)	alloc_page_interface(cmd);
		else if( strncmp(cmd, "free", 4) == 0)	free_page_interface(cmd);
		else if( strncmp(cmd, "dma", 3) == 0)	dynamic_memory_alloc_interface(cmd);
		else if( strncmp(cmd, "fdma", 4) == 0)	free_dynamic_memory_interface(cmd);
		else if( strcmp(cmd, "thread") == 0)	create_thread_interface();
		else if( strncmp(cmd, "svc", 3) == 0)	system_call_interface(cmd);
		else if( strcmp(cmd, "fork") == 0)		fork_test_interface();
		else if( strcmp(cmd, "exec") == 0)		exec_interface();
		else if( strcmp(cmd, "ttt") == 0)		test_uart();
		else if( strcmp(cmd, "signal") == 0)	test_signal();
		else if( strcmp(cmd, "pid") == 0)		printf("\n%d\n", get_pid());
		else									command_not_found(cmd);
	}
}