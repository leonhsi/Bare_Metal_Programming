#include "uart.h"
#include "utils.h"
#include "irq.h"
#include "queue.h"
#include "gpio.h"

#define Task_Capacity 10

extern void _timeout_handler();
extern void _uart_irq();

typedef struct{
    int type;   //0 : uart, 1 : timer
    int priority;
}Task;

typedef struct{
    int front; 
    int rear;
    int size;
    Task task_buffer[Task_Capacity];
}Task_Queue;

Task_Queue* task_queue;

void init_task_queue(){
    task_queue = (Task_Queue*)simple_malloc(sizeof(Task_Queue) + sizeof(Task) * Task_Capacity);
    task_queue->front = 0;
    task_queue->rear = Task_Capacity-1;
    task_queue->size = 0;
}

void enqueue_task(int type, int priority){
    task_queue->rear = (task_queue->rear+1) % Task_Capacity;
    task_queue->size++;

    task_queue->task_buffer[task_queue->rear].type = type;
    task_queue->task_buffer[task_queue->rear].priority = priority;

    //enable irq
    asm volatile ("msr daifclr, 0xf");

    if(task_queue->size == 1){
        process_task(false);
    }
    else if(task_queue->task_buffer[task_queue->front].priority < priority){
        //preempt
        process_task(true);
    }
}

void pop_task(){
    task_queue->front = (task_queue->front + 1) % Task_Capacity;
    task_queue->size--;
}

void process_task(bool preempt){
    Task task = task_queue->task_buffer[task_queue->front];

    if(preempt){
        if(task.type == 0){
            _uart_irq();
        }
        else if(task.type == 1){
            _timeout_handler();
        }
        pop_task();
    }
    else{
        while(task_queue->size){
            if(task.type == 0){
                _uart_irq();
            }
            else if(task.type == 1){
                _timeout_handler();
            }

            pop_task();         
        }
    }
} 

void irq_handler(){

    // //store return address to vector table
    // register unsigned int return_addr;
    // asm volatile("mov %0, lr\n" 
    //             : "=r"(return_addr) 
    //             : );

    //timer interrupt
    register unsigned int irq_source;
    irq_source = *INTERRUPT_SOURCE;

    if ( irq_source & (1<<1) ){     //physical timer interrupt
        _timeout_handler();
        //enqueue_task(1, 0);
    }
    else if( irq_source & (1<<8) ){ //uart (GPU) interrupt
        _uart_irq();
        //enqueue_task(0, 0);
    }

    // asm volatile("mov lr, %0" 
    //             : 
    //             : "r"(return_addr));
}

extern Queue *user_r_queue;
extern Queue *user_w_queue;

void uart_irq_handler(){

    uart_disable_ir();

    register unsigned int r;
    r = *AUX_MU_IIR;
    r &= 0x6;
    r >>= 1;

    switch(r){
        //TX interrupt handler
        case 0x01:  
        {
            //uart_puts("\ntx\n");
  
            while(user_w_queue->size){
                *AUX_MU_IO = dequeue(user_w_queue);
            }
            break;
        }
        //RX interrupt handler
        case 0x2:  
        {
            //uart_puts("\nrx\n");

            while((*AUX_MU_LSR&0x01)){
                char c = (char)(*AUX_MU_IO);
                enqueue(user_r_queue, c);
            }
            break;
        }
    }

    uart_enable_rx_ir();
}