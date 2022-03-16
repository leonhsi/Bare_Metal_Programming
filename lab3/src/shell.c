#include "uart.h"
#include "utils.h"
#include "mbox.h"
#include "reboot.h"
#include "loader.h"
#include "initrd.h"
#include "dtb.h"
#include "excp.h"
#include "time.h"

extern void _user();
extern void _from_el1_to_el0();

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
	printf("svc : \t\tsystem call to print spsr_el1, elr_el1, esr_el1 and far_el1\n");
	printf("time : \t\ttime <MESG> <SEC>, set time out after <SEC> and print <MESG>");
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
		else if( strcmp(cmd, "svc") == 0 )		_user();
		else if( strncmp(cmd, "time", 4) == 0)	set_timer_interface(cmd);
		else									command_not_found(cmd);
	}
}
