#ifndef IRQ__H
#define IRQ__H

//#define INTERRUPT_SOURCE (volatile unsigned int*)0x40000060
#define INTERRUPT_SOURCE (volatile unsigned int*)0xFFFF000040000060

typedef int bool;
#define false 0
#define true 1

void init_task_queue();
void enqueue_task();
void pop_task();
void process_task();
void irq_handler();
void uart_irq_handler();

#endif