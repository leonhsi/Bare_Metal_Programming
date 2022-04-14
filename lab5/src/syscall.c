#include "uart.h"
#include "utils.h"
#include "thread.h"
#include "buddy.h"
#include "shell.h"
#include "initrd.h"
#include "mbox.h"
#include "syscall.h"

extern int _get_current_context();
extern void _switch_to_bottom();

//user
int get_pid(){
    asm volatile("mov x8, #0");
    asm volatile("svc #0");

    int pid;
    asm volatile("mov %0, x0" : "=r"(pid));
    return pid;
}

long long uartread(char buf[], long long size){
    asm volatile("mov x9, %0" : : "r"(buf));
    asm volatile("mov x10, %0" : : "r"(size));
    asm volatile("mov x8, #1");
    asm volatile("svc #0");

    long long ret;
    asm volatile("mov %0, x0" : "=r"(ret));
    return ret;
}

long long uartwrite(const char buf[], long long size){
    asm volatile("mov x9, %0" : : "r"(buf));
    asm volatile("mov x10, %0" : : "r"(size));
    asm volatile("mov x8, #2");
    asm volatile("svc #0");

    long long ret;
    asm volatile("mov %0, x0" : "=r"(ret));
    return ret;
}

//int exec(char *name, char *const argv[]){
int exec(char *name){

    //printf("exec img addr : %x\n", img_addr);

    // asm volatile("mov x9, %0" : : "r"(img_addr));
    // asm volatile("mov x10, %0" : : "r"(img_len));
    asm volatile("mov x0, %0" : : "r"(name));
    asm volatile("mov x8, #3");
    asm volatile("svc #0");
 
    return 0;
}

int fork(){
    asm volatile("mov x8, #4");
    asm volatile("svc #0");

    //only need to read return value from x0
    int ret;
    asm volatile("mov %0, x0" : "=r"(ret));
    //printf("fork ret : %d\n", ret);
    return ret;
}

void exit(){
    asm volatile("mov x8, #5");
    asm volatile("svc #0");
}

int mbox_call(unsigned char ch, volatile unsigned int *mbox){
    asm volatile("mov x9, %0" : : "r"(ch));
    asm volatile("mov x10, %0" : : "r"(mbox));
    asm volatile("mov x8, #6");
    asm volatile("svc #0");

    long long ret;
    asm volatile("mov %0, x0" : "=r"(ret));
    return ret;
}

void kill(int pid, int SIGTYPE){
    printf("killing \n");
    asm volatile("mov x1, %0" : : "r"(SIGTYPE));
    asm volatile("mov x0, %0" : : "r"(pid));
    asm volatile("mov x8, #7");
    asm volatile("svc #0");
}

void signal(long long SIGTYPE, void *register_handler){
    asm volatile("mov x1, %0" : : "r"(register_handler));
    asm volatile("mov x0, %0" : : "r"(SIGTYPE));
    asm volatile("mov x8, #8");
    asm volatile("svc #0");
}

void sigreturn(){
    asm volatile("mov x8, #9");
    asm volatile("svc #0");
}

//kernel
void __get_pid(int sp){
    long long cur_context = _get_current_context();
    Thread *cur_thread = (Thread *)cur_context;
    //printf("tid : %d\n", cur_thread->tid);

    //write trap frame
    //int sp = cur_thread->context.sp - 34*8;
    asm volatile("mov x2, %0" : :"r"(sp));  //sp, can't use x0
    asm volatile("mov x1, %0" : :"r"(cur_thread->tid)); //return value
    asm volatile("str x1, [x2, 16 * 0]");
}

void __uartread(long long buf_addr, long long size, long long cur_ksp){

    char *buf = (char *)buf_addr;

    asm volatile("msr DAIFClr, 0xf");
    for(int i=0; i<size; i++){
        *buf = uart_getc();
        buf++;
    }
    asm volatile("msr DAIFSet, 0xf");

    //write trap frame
    asm volatile("mov x2, %0" : : "r"(cur_ksp));
    asm volatile("mov x1, %0" : : "r"(size));
    asm volatile("str x1, [x2, 16 * 0]");
}

void __uartwrite(long long buf_addr, long long size, long long cur_ksp){
    char *buf = (char *)buf_addr;
    
    asm volatile("msr DAIFClr, 0xf");
    uart_write(buf, size);
    asm volatile("msr DAIFSet, 0xf");

    //write trap frame
    asm volatile("mov x2, %0" : : "r"(cur_ksp));
    asm volatile("mov x1, %0" : : "r"(size));
    asm volatile("str x1, [x2, 16 * 0]");
}

void __exec(int cur_ksp, long long img_name_addr){
    char *filename = (char *)img_name_addr;
    char *img_cpio_addr = get_cpio_file(filename);
    long long img_len = get_cpio_file_len(filename);

    printf("\nfile %s\n", filename);

    char *img_addr = alloc_page(img_len / Page_Size + 1);
    char *img_addr_tmp = img_addr;
    char *img_cpio_tmp = (char *)img_cpio_addr;

    for(int i=0; i<img_len; i++){
        *img_addr_tmp = *img_cpio_tmp;
        img_addr_tmp++;
        img_cpio_tmp++;
    }

    //write trap frame, set elr to img addr
    asm volatile("mov x1, %0" : : "r"(cur_ksp));
    asm volatile("mrs x2, spsr_el1");
    asm volatile("mov x3, %0" : : "r"((long long)img_addr));    //elr
    asm volatile("stp x2, x3, [x1, 16 * 16]");

    //return;
}

void copy_context(Context *child, Context *parent){
    child->x19 = parent->x19;
    child->x20 = parent->x20;
    child->x21 = parent->x21;
    child->x22 = parent->x22;
    child->x23 = parent->x23;
    child->x24 = parent->x24;
    child->x25 = parent->x25;
    child->x26 = parent->x26;
    child->x27 = parent->x27;
    child->x28 = parent->x28;
    child->fp = parent->fp;
    child->lr = parent->lr;
    child->sp = parent->sp;
}

void __fork(int cur_ksp, int svc_lr){
    //create child thread
    Thread *child_thread = create_thread(0);

    //copy parent's user and kernel stack
    long long parent_context = _get_current_context();
    Thread *parent_thread = (Thread *)parent_context;

    char *tmp_child_usp = child_thread->ustack;
    char *tmp_parent_usp = parent_thread->ustack;
    char *tmp_child_ksp = child_thread->kstack;
    char *tmp_parent_ksp = parent_thread->kstack;
    int n = Page_Size;
    while(n--){
        *tmp_child_usp = *tmp_parent_usp;
        tmp_child_usp--;
        tmp_parent_usp--;
        *tmp_child_ksp = *tmp_parent_ksp;
        tmp_child_ksp--;
        tmp_parent_ksp--;
    }

    //copy context
    copy_context(&child_thread->context, &parent_thread->context);

    //set child context's sp as parent's
    long long offset = (long long)parent_thread->kstack - cur_ksp;
    child_thread->context.sp = (long long)child_thread->kstack - offset;

    //set lr
    parent_thread->context.lr = svc_lr;
    child_thread->context.lr = svc_lr;

    //write return value to parent's trap frame
    asm volatile("mov x1, %0" : : "r"(cur_ksp));
    asm volatile("mov x2, %0" : : "r"(child_thread->tid));
    asm volatile("str x2, [x1, 16 * 0]");

    //write return value and sp_el0 to child's trap frame
    asm volatile("mov x1, %0" : : "r"(child_thread->context.sp));
    asm volatile("mov x2, #0");
    asm volatile("str x2, [x1, 16 * 0]");

    long long cur_usp, cur_child_usp;
    asm volatile("mov x1, %0" : : "r"(cur_ksp));
    asm volatile("ldr %0, [x1, 16 * 17]" : "=r"(cur_usp));
    offset = (long long)parent_thread->ustack - cur_usp;
    cur_child_usp = (long long)child_thread->ustack - offset;
    asm volatile("mov x1, %0" : : "r"(child_thread->context.sp));
    asm volatile("mov x2, %0" : : "r"(cur_child_usp));
    asm volatile("str x2, [x1, 16 * 17]");

    //schedule
    schedule(shell);
    //printf("omgomgomg\n");
}

void __exit(int svc_lr){
    long long cur_context = _get_current_context();
    Thread *cur_thread = (Thread *)cur_context;

    //printf("\ntid %d exit\n", cur_thread->tid);

    cur_thread->status = DEAD;

    //schedule();
}

void __mbox_call(long long cur_ksp, long long ch, int* mbox){
    long long ret = 0;

    unsigned int r = (((unsigned int)((unsigned long)mbox)&~0xF) | (ch&0xF));
    //printf("\nr :%d\n", r);

    do{asm volatile("nop");}while(*MBOX_STATUS & MBOX_FULL);
    *MBOX_WRITE = r;
    while(1) {
        /* is there a response? */
        do{asm volatile("nop");}while(*MBOX_STATUS & MBOX_EMPTY);
        /* is it a response to our message? */
        if(r == *MBOX_READ){
            /* is it a valid successful response? */
            ret = (mbox[1]==MBOX_RESPONSE);
            break;
        }
    }

    //write trap frame
    asm volatile("mov x1, %0" : : "r"(cur_ksp));
    asm volatile("mov x2, %0" : : "r"(ret));
    asm volatile("str x2, [x1, 16 * 0]");
}

void __kill(int pid, long long SIGTYPE){
    printf("\npid : %d will be killed, SIGTYPE : %d\n", pid, SIGTYPE);
    struct list_head *pos;
    list_for_each(pos, &run_queue->h_list){
        Thread *t = container_of(pos, Thread, t_list);
        if(t->tid == pid){
            printf("die bitch\n");
            t->sig_types[SIGTYPE] = 1;
            break;
        }
    }
}

void __signal(long long SIGTYPE, void (*register_handler)()){
    printf("\nSIGTYPE : %d\n", SIGTYPE);
    (register_handler)();
    long long cur_context = _get_current_context();
    Thread *cur_thread = (Thread *)cur_context;
    cur_thread->register_sighands[SIGTYPE] = register_handler;
    cur_thread->sig_types[SIGTYPE] = 2;
}

void __sigreturn(long long cur_ksp){
    printf("signal returnnnnnnnnnnnnnnnnnnnnnnnnnn\n");

    //require cur kernel stack pointer
    //get cur context (should be in kernel )
    long long cur_context = _get_current_context();
    _switch_to_bottom(cur_context);

    //schedule();
}

void system_call_handler(int esr_el1, int cur_ksp, int svc_lr){

    int svc_no = esr_el1 & 0xF;
    int sys_no;
    asm volatile("mov %0, x8" : "=r"(sys_no));

    long long x0, x1;
    asm volatile("mov x1, %0" : : "r"(cur_ksp));
    asm volatile("ldp %0, %1, [x1, 16 * 0]" : "=r"(x0), "=r"(x1));

    if( ((esr_el1 >> 26) & (0b111111)) != (0b010101)){
        printf("fuck you it's real exception\n");
        return;
    }

    if(svc_no == 0){
        switch(sys_no){
            case 0:
                __get_pid(cur_ksp);
                break;
            case 1:
                __uartread(x0, x1, cur_ksp);
                break;
            case 2:
                __uartwrite(x0, x1, cur_ksp);
                break;
            case 3:
                __exec(cur_ksp, x0);
                break;
            case 4:
                __fork(cur_ksp, svc_lr);
                break;
            case 5:
                __exit(svc_lr);
                break;
            case 6:
                __mbox_call(cur_ksp, x0, (int *)x1);
                break;
            case 7:
                __kill(x0, x1);
                break;
            case 8:
                __signal(x0, (void *)x1);
                break;
            case 9:
                __sigreturn(cur_ksp);
                break;
            default:
                printf("\nException!!! SVC %d\n", svc_no);
                break;
        }
    }
    else{
        printf("\nException!!! SVC %d\n", svc_no);
    }

    return;
}