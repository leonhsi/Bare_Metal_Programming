void get_current_el();
void exception_log(unsigned long type, unsigned long esr, unsigned long elr, unsigned long spsr, unsigned long far);
void get_boot_time(unsigned int time);
void uart_irq_handler();
