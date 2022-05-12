#ifndef THREAD__H
#define THREAD__H

#include "list.h"
#include "stdint.h"

#define SIG_INT 2
#define SIGKILL 9

enum T_STATUS {IDLE, RUN, DEAD};

typedef struct mm_struct{
    struct list_head vma_head;    //list of vma
    struct list_head mapped_pg_head;
}mm_struct;

typedef struct vma_struct{
    usize_t start;
    usize_t end;
    struct list_head vma_list;
    int prot;
}vma_struct;

typedef struct mapped_pg{
    char *va;
    char *pa;
    uint64_t size;
    int perm;
    struct list_head mapped_pg_list;
}mapped_pg;

typedef struct{
    long long x19, x20, x21, x22, x23, x24, x25, x26, x27, x28;
    long long fp, lr;
    long long sp;
}Context;

typedef struct{
    Context context;
    int tid;
    enum T_STATUS status;
    void (*task)();
    char *ustack;
    char *kstack;
    char *code;
    struct list_head t_list;    //for run queue
    long long sig_types[10];    //0 : no signal, 1 : default, 2 : register
    void (*default_sighands[10])();
    void (*register_sighands[10])();
    uint64_t codesize;
    uint64_t pagetable;
    struct mm_struct *mm;
}Thread;

typedef struct{
    int cur_tid;
    Thread *init_thread;
    struct list_head h_list;
}Run_Queue;

extern Run_Queue *run_queue;

extern void to_el0(void *addr, uint64_t usp, uint64_t pagetable);

void init_run_queue();
void print_list();
Thread* create_thread(void (*task)());
void schedule();
void idle();
void foo();
void fork_test();
void check_signal();
void default_handler_kill(Thread *this, long long SIGTYPE);
void register_handler_kill(Thread *this, long long SIGTYPE);
void say_hi();

#endif