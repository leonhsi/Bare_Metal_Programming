#include "uart.h"
#include "utils.h"
#include "queue.h"
#include "time.h"

#define timer_capacity 10
#define mesg_capacity 10

extern void _core_timer_enable();

typedef struct{
    int sec;
    char mesg[mesg_capacity];
}Timer;

typedef struct{
    int front;
    int rear;
    int size;
    Timer timer_array[timer_capacity]; 
}Timer_Queue;

Timer_Queue* timer_queue;

void init_timer_queue(){
    timer_queue = (Timer_Queue*)simple_malloc(sizeof(Timer_Queue) + sizeof(Timer) * timer_capacity);
    timer_queue->front = 0;
    timer_queue->size = 0;
    timer_queue->rear = timer_capacity-1;
}

void enqueue_timer(char *message, int sec){
    timer_queue->rear = (timer_queue->rear + 1) % timer_capacity;
    timer_queue->size++;
    timer_queue->timer_array[timer_queue->rear].sec = sec;

    strncpy(timer_queue->timer_array[timer_queue->rear].mesg, message, strlen(message));

    //printf("\nsize : %d\n", timer_queue->size);

    insert_timer();
    invoke_timer(); 
}

void pop_timer(){
    //Timer t = timer_q->timer[timer_q->front];
    timer_queue->front = (timer_queue->front + 1) % timer_capacity;
    timer_queue->size--;

    //return t;    
}

void insert_timer(){
    //one iteration insertion sort
    int size = timer_queue->size;

    if(size == 1) return;

    Timer temp = timer_queue->timer_array[timer_queue->rear];
    
    int i = size - 2;
    while(i >= 0 && timer_queue->timer_array[i].sec > temp.sec){
        timer_queue->timer_array[i+1] = timer_queue->timer_array[i];
        i--;
    }
    timer_queue->timer_array[i+1] = temp;
}

void invoke_timer(){
    //printf("\ninvoke timer\n");
    int sec = timer_queue->timer_array[timer_queue->front].sec;
    _core_timer_enable(sec);
}

void timeout(){
    printf("\n[time out] : [%s]\n", timer_queue->timer_array[timer_queue->front].mesg);

    pop_timer();

    if(timer_queue->size){
        invoke_timer();
    }
}

void add_timer(void (*call_back)(char *fmt, ...), char * message, int sec){
    enqueue_timer(message, sec);
    printf("\n");
}

void setTimeout(char *message, int sec){ 
    add_timer(printf, message, sec);
}