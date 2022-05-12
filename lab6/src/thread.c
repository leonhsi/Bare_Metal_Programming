#include "uart.h"
#include "utils.h"
#include "queue.h"
#include "buddy.h"
#include "thread.h"
#include "syscall.h"
#include "shell.h"
#include "mm.h"

Run_Queue *run_queue;

extern void _switch_to();
extern void _switch_to_bottom();
extern size_t _get_current_context();
extern void _ret_shell();
extern void _core_timer_enable();
extern void _switch_to_top();

void init_run_queue(){
    run_queue = (Run_Queue *)simple_malloc(sizeof(Run_Queue));
    run_queue->cur_tid = 0;

    // run_queue->init_thread.tid = 100;
    // run_queue->init_thread.status = IDLE;
    // run_queue->init_thread.task = (shell);
    // run_queue->init_thread.ustack = alloc_page(1);
    // run_queue->init_thread.kstack = alloc_page(1);

    // run_queue->init_thread.context.sp = (size_t)run_queue->init_thread.kstack;
    // run_queue->init_thread.context.lr = (size_t)run_queue->init_thread.task;

    INIT_LIST_HEAD(&(run_queue->h_list));

    run_queue->init_thread = create_thread(0);

    asm volatile("msr tpidr_el1, %0" : : "r"(&run_queue->init_thread->context));

    printf("initial thread context : %x\n", &run_queue->init_thread->context);

    //set sp
    //asm volatile("mov sp, %0" : : "r"(run_queue->init_thread->context.sp));
    run_queue->init_thread->kstack = (char *)0xFFFF000000080000;
    run_queue->init_thread->context.sp = (size_t)run_queue->init_thread->kstack;
    size_t initial_sp;
    asm volatile("mov %0, sp" : "=r"(initial_sp));
    printf("initial sp : %x\n", initial_sp);

    printf("tid : %d\n", run_queue->init_thread->tid);
    //int pid = get_pid();
    //printf("pid : %d\n", pid);
}

Thread* create_thread(void (*task)()){
    Thread *t = (Thread *)alloc_page(1);
    run_queue->cur_tid++;

    t->tid = run_queue->cur_tid;
    t->status = RUN;
    t->task = task;
    t->ustack = alloc_page(1) + 0x1000;
    t->kstack = alloc_page(1) + 0x1000;
    //printf("\ncreate thread id : %d, user stack : %x, kernel stack : %x\n", t->tid, t->ustack, t->kstack);

    INIT_LIST_HEAD(&(t->t_list));

    t->context.sp = (size_t)t->kstack;   //used only when context switch, always in el1
    t->context.lr = (size_t)task;

    t->pagetable = (uint64_t)KA2PA(alloc_page(1));
    
    //map user stack
    t->mm = init_mm();
    vma_struct *vma = container_of(t->mm->vma_head.next, vma_struct, vma_list);     //first vma
    vma->start = 0x0000ffffffffb000;
    vma->end = 0x0000fffffffff000;
    mappages(PA2KA((void *)t->pagetable), 0x0000ffffffffe000, 4096, (uint64_t)t->ustack - 0x1000, PT_AF | PT_USER | PT_MEM | PT_RW, t);

    //map mail box
    mappages(PA2KA((void *)t->pagetable), 0x3c100000, 0x200000, PA2KA(0x3c100000), PT_AF | PT_USER | PT_MEM | PT_RW, t);

    //add new thread to run queue
    list_add_tail(&(t->t_list), &(run_queue->h_list));

    //set signal flag to 0
    // for(int i=0; i<10; i++){
    //     t->sig_types[i] = 0;
    // }

    // //signal handler
    // t->default_sighands[SIGKILL] = default_handler_kill;

    //printf("thread created\n");
    return t;
}

void print_list(){
    printf("[id(0), status(0)] -> ");

    struct list_head *pos;
    list_for_each(pos, &run_queue->h_list){
        Thread *t = container_of(pos, Thread, t_list);
        printf("[id(%d), status(%d)] -> ", t->tid, t->status);
    }
    printf("null\n");
}

void schedule(size_t init_lr){

    if(list_empty(&run_queue->h_list)){
        printf("\nRun queue empty, return to shell!\n");
        printf("\r# ");
        run_queue->init_thread->context.lr = init_lr;
        asm volatile("mov %0, sp" : "=r"(run_queue->init_thread->context.sp));

        // size_t pre_context = _get_current_context();
        // size_t next_context = (size_t)&run_queue->init_thread.context;
        // if(pre_context == next_context){
        //     printf("same!!");
        //     _switch_to_bottom(next_context);
        // }

        _switch_to(_get_current_context(), &run_queue->init_thread->context);
    }

    size_t pre_context = _get_current_context(); 
    size_t next_context = (size_t)&(run_queue->init_thread->context);
    //printf("\n====schedule : \n");

    size_t pgd_addr;
    struct list_head *pos; 
    list_for_each(pos, &run_queue->h_list){
        Thread *t = container_of(pos, Thread, t_list);

        //printf("lr : %x\n", t->context.lr);
        if(t->status == RUN){
            uint64_t usp;
            asm volatile("mov %0, sp" : "=r"(usp));
            printf("tid %d is runnable, switch to this thread with virtual sp : %x, physical sp : %x\n", t->tid, usp, KA2PA(t->ustack));
            //printf("addr : %x\n", t);
            list_move_tail(&t->t_list, &run_queue->h_list);
            next_context = (size_t)(&(t->context));
            pgd_addr = t->pagetable;
            // if(pre_context == next_context){
            //     //printf("same!!");
            //     asm volatile("mov lr, %0" : : "r"(t->context.lr));
            //     asm volatile("mov sp, %0" : : "r"(t->context.sp));
            //     asm volatile("ret");
            // }
            break;
        }
        // else if(t->status == DEAD){
        //     //printf("tid %d died, switch to idle(init) thread\n", t->tid);
        //     // let idle thread kill zombie
        //     run_queue->init_thread.task = (idle);
        //     run_queue->init_thread.context.lr = (size_t)run_queue->init_thread.task;
        //     next_context = (size_t)&run_queue->init_thread.context;
        //     //idle();
        //     //return;
        //     // run_queue->init_thread.context.lr = init_lr;
        //     // asm volatile("mov %0, sp" : "=r"(run_queue->init_thread.context.sp));
        //     break;
        // }
    }
    
    //print_list();
    //printf("====\n");
    //printf("\ncur pid 1: %d\n", get_pid());

    _switch_to(pre_context, next_context, PA2KA(pgd_addr)); 
    //printf("\ncur pid 2: %d\n", get_pid());
    //check_signal();
}

void check_signal(){
    struct list_head *pos;

    list_for_each(pos, &run_queue->h_list){
        Thread *t = container_of(pos, Thread, t_list);
        switch(t->sig_types[SIGKILL]){
            case 1:
                (t->default_sighands[SIGKILL])(t, SIGKILL);
                break;
            case 2:
                register_handler_kill(t, SIGKILL);
                break;
            default:
                break;
        }
    }
}

void idle(size_t init_lr){
    //printf("\n===idle\n");
    //int has_zombie = 0;

    struct list_head *pos;
    list_for_each(pos, &run_queue->h_list){
        Thread *t = container_of(pos, Thread, t_list);
        if(t->status == DEAD){
            //has_zombie = 1;
            printf("kill tid %d\n", t->tid);
            pos = t->t_list.prev;
            list_del(&(t->t_list));
            //free_pages(t->ustack);
            //free_pages(t->kstack);
        }
    }
    //if(!has_zombie) printf("no zombie to kill \n");
    //printf("===\n");

    schedule(init_lr);
}

void foo(){
    size_t cur_context = _get_current_context();
    Thread *cur_thread = (Thread *)cur_context;
	for(int i=0; i<10; i++){
		printf("Thread id : %d, get_pid : %d, i : %d, status : %d\n", cur_thread->tid, get_pid(), i, cur_thread->status);
        delay(10000000);
        if(i == 9){
            //printf("set tid %d to DEAD\n", cur_thread->tid);
            cur_thread->status = DEAD;
        }
        schedule((size_t)shell);
	}
    printf("\nhere\n");
}

void fork_test(){
    printf("\nFork Test, pid %d\n", get_pid());
    int cnt = 1;
    int ret = 0;
    size_t cur_sp;
    if ((ret = fork()) == 0) { // child
        asm volatile("mov %0, sp" : "=r"(cur_sp));
        printf("first child pid: %d, cnt: %d, ptr: %x, sp : %x\n", get_pid(), cnt, &cnt, cur_sp);
        ++cnt;
        
        if ((ret = fork()) != 0){
            printf("first child pid after fork: %d, cnt: %d, ptr: %x, sp : %x\n", get_pid(), cnt, &cnt, cur_sp);
        }
        else{
            while (cnt < 5) {
                asm volatile("mov %0, sp" : "=r"(cur_sp));
                printf("second child pid: %d, cnt: %d, ptr: %x, sp : %x\n", get_pid(), cnt, &cnt, cur_sp);
                delay(1000000);
                ++cnt;
            }
        }
        exit();
    } 
    else {
        asm volatile("mov %0, sp" : "=r"(cur_sp));
        printf("parent here, pid %d, child %d, sp : %x\n\n", get_pid(), ret, cur_sp);
    }
}

void default_handler_kill(Thread *this, size_t SIGTYPE){
    printf("\ndefualt handler kill pid : %d\n", this->tid);
    this->status = DEAD;
    this->sig_types[SIGTYPE] = 0;
    return;
}

void register_handler_kill(Thread *this, size_t SIGTYPE){
    //store lr to go back to check signal
    size_t cur_lr;
    asm volatile("mov %0, lr" : "=r"(cur_lr));

    printf("\nregister handler kill pid : %d\n", this->tid);
    this->sig_types[SIGTYPE] = 0;

    //acquire new user stack, set sp_el0
    char *new_usp = alloc_page(1);
    asm volatile("msr sp_el0, %0" : : "r"((size_t)new_usp));

    //set elr_el1 to register handler, spsr_el1 = 0
    asm volatile("msr elr_el1, %0" : : "r"((size_t)this->register_sighands[SIGTYPE]));
    asm volatile("mov x1, 0x0");
    asm volatile("msr spsr_el1, x1");

    //store cur context
    size_t cur_context = _get_current_context();
    _switch_to_top(cur_context);
    asm volatile("stp fp, %0, [%1, 16 * 5]" : : "r"(cur_lr), "r"(cur_context));

    //set lr to sigreturn()
    //asm volatile("mov lr, %0" : : "r"((size_t)sigreturn));

    //eret
    asm volatile("eret");

    printf("hohoho\n");
}

//register handler
void say_hi(){
    printf("\nhiiiiiiiiiiiiiiiiiiiiiiiiiiiii\n");
}