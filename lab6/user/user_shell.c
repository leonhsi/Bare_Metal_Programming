#include "user_uart.h"
#include "user_utils.h"
#include "user_syscall.h"

void help(){
    uart_puts("\n\nhelp : \t\tprint help menu\n");
    uart_puts("fork : \t\tcreate child process\n");
    uart_puts("exec : \t\texecute function\n");
}

void command_not_found(char *cmd){
    uart_puts("\ncommand not found : ");
    uart_puts(cmd);
    uart_puts(" \n");
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
        uart_puts("\r#");
        uart_puts(cmd);
    }
}

void shell(){
    char cmd[100];

    while(1){
        shell_input(cmd);

        if( strcmp(cmd, "help") == 0)       help();
        else if( strcmp(cmd, "fork") == 0)  fork();
        else if( strcmp(cmd, "exec") == 0)  exec(0);
        else                                command_not_found(cmd);
    }
}