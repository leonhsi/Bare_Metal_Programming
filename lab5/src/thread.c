#include "uart.h"
#include "utils.h"
#include "queue.h"
#include "buddy.h"
#include "thread.h"

extern void _switch_to();
extern int _get_current_context();
extern void _ret_shell();

void init_run_queue(){
    run_queue = (Run_Queue *)simple_malloc(sizeof(Run_Queue));
    run_queue->cur_tid = 0;

    run_queue->init_thread.tid = 0;
    run_queue->init_thread.status = IDLE;
    run_queue->init_thread.task = (idle);
    run_queue->init_thread.ustack = alloc_page(1);
    run_queue->init_thread.kstack = alloc_page(1);

    run_queue->init_thread.context.sp = (long long)run_queue->init_thread.ustack;
    run_queue->init_thread.context.lr = (long long)run_queue->init_thread.task;

    //asm volatile("msr tpidr_el1, %0" : : "r"(&run_queue->init_thread.context));
    
    INIT_LIST_HEAD(&(run_queue->h_list));
}

void create_thread(void (*task)()){
    Thread *t = (Thread *)simple_malloc(sizeof(Thread));
    run_queue->cur_tid++;

    t->tid = run_queue->cur_tid;
    t->status = RUN;
    t->task = task;
    t->ustack = alloc_page(1);
    t->kstack = alloc_page(1);
    INIT_LIST_HEAD(&(t->t_list));

    t->context.sp = (long long)t->ustack;
    t->context.lr = (long long)task;

    //add new thread to run queue
    list_add_tail(&(t->t_list), &(run_queue->h_list));

    //printf("\ncreate thread id : %d, ustack : %x, kstack : %x\n", t->tid, t->ustack, t->kstack);
}

void print_list(){
    printf("[0, 0] -> ");

    struct list_head *pos;
    list_for_each(pos, &run_queue->h_list){
        Thread *t = container_of(pos, Thread, t_list);
        printf("[%d, %d] -> ", t->tid, t->status);
    }
    printf("\n");
}

void schedule(){
    if(list_empty(&run_queue->h_list)){
        printf("\nRun queue empty, return to shell!\n");
        _ret_shell();
    }

    long long pre_context, next_context = 0;

    struct list_head *pos; 
    list_for_each(pos, &run_queue->h_list){
        Thread *t = container_of(pos, Thread, t_list);

        if(t->status == RUN){
            //printf("tid %d is runnable, switch to this thread\n", t->tid);
            list_move_tail(&t->t_list, &run_queue->h_list);
            next_context = (long long)&t->context;
            break;
        }
        else if(t->status == DEAD){
            //printf("has zombie no.%d, switch to idle thread\n", t->tid);
            next_context = (long long)&run_queue->init_thread.context;
            break;
        }
        printf("aa\n");
    }
    pre_context = _get_current_context();

    _switch_to(pre_context, next_context); 
}

void idle(){
    printf("===idle\n");
    int has_zombie = 0;

    struct list_head *pos;
    list_for_each(pos, &run_queue->h_list){
        Thread *t = container_of(pos, Thread, t_list);
        if(t->status == DEAD){
            has_zombie = 1;
            printf("kill tid %d\n", t->tid);
            pos = t->t_list.prev;
            list_del(&(t->t_list));
            //free_pages(t->ustack);
            //free_pages(t->kstack);
        }
    }
    if(!has_zombie) printf("no zombie to kill \n");
    printf("===\n");
    schedule();
}

void foo(){
    long long cur_context = _get_current_context();
    Thread *cur_thread = (Thread *)cur_context;
	for(int i=0; i<10; i++){
		printf("Thread id : %d ,i : %d, status : %d\n", cur_thread->tid, i, cur_thread->status);
        delay(100000000);
        if(i == 9){
            //printf("set tid %d to DEAD\n", cur_thread->tid);
            cur_thread->status = DEAD;
        }
        schedule();
	}
    //printf("\nhere\n");
}