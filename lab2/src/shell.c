#include "uart.h"
#include "utils.h"
#include "mbox.h"
#include "reboot.h"
#include "loader.h"
#include "initrd.h"
#include "dtb.h"

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
	printf("\n");
}

void reboot()
{
	printf("\nrebooting ... \n");
	reset(0);

	//use a while loop to do nothing or it will keep reading commands
	while(1){}
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

		if( strcmp(cmd, "hello") == 0)			hello();
		else if( strcmp(cmd, "help") == 0)		help();
		else if( strcmp(cmd, "reboot") == 0)	reboot();
		else if( strcmp(cmd, "board") == 0)		get_board_revision();
		else if( strcmp(cmd, "mem") == 0)		get_arm_memory();
		else if( strcmp(cmd, "lk") == 0)		copy_old_kernel();
		else if( strcmp(cmd, "ls") == 0)		parse_cpio_name();
		else if( strcmp(cmd, "cat") == 0)		parse_cpio_file();
		else									command_not_found(cmd);
	}
}
