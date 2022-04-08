#include "list.h"

enum T_STATUS {IDLE, RUN, DEAD};

typedef struct{
    long long x19, x20, x21, x22, x23, x24, x25, x26, x27, x28;
    long long fp, lr;
    long long sp;
}Context;

typedef struct{
    int sp_usr;
    int lr_usr;
    int sp_svc;
    int lr_svc;
    int spsr;
}Trap_Frame;

typedef struct{
    Context context;
    int tid;
    enum T_STATUS status;
    void (*task)();
    char *ustack;
    char *kstack;
    struct list_head t_list;
}Thread;

typedef struct{
    int cur_tid;
    Thread init_thread;
    struct list_head h_list;
}Run_Queue;

Run_Queue *run_queue;

void init_run_queue();
void print_list();
Thread* create_thread(void (*task)());
void schedule();
void idle();
void foo();
void fork_test();