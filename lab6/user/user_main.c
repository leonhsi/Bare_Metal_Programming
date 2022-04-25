#include "user_uart.h"
#include "user_shell.h"
#include "user_utils.h"

int main(){
    uart_init();
    uart_flush();

    shell();

    return 0;
}