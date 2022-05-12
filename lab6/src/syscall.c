#include "uart.h"
#include "utils.h"
#include "thread.h"
#include "buddy.h"
#include "shell.h"
#include "initrd.h"
#include "mbox.h"
#include "syscall.h"
#include "mm.h"

extern size_t _get_current_context();
extern void _switch_to_bottom();

//user
int get_pid(){
    asm volatile("mov x8, #0");
    asm volatile("svc #0");

    int pid;
    asm volatile("mov %0, x0" : "=r"(pid));
    return pid;
}

size_t uartread(char buf[], size_t size){
    asm volatile("mov x9, %0" : : "r"(buf));
    asm volatile("mov x10, %0" : : "r"(size));
    asm volatile("mov x8, #1");
    asm volatile("svc #0");

    size_t ret;
    asm volatile("mov %0, x0" : "=r"(ret));
    return ret;
}

size_t uartwrite(const char buf[], size_t size){
    asm volatile("mov x9, %0" : : "r"(buf));
    asm volatile("mov x10, %0" : : "r"(size));
    asm volatile("mov x8, #2");
    asm volatile("svc #0");

    size_t ret;
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

    size_t ret;
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

void signal(size_t SIGTYPE, void *register_handler){
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
void __get_pid(size_t sp){
    size_t cur_context = _get_current_context();
    Thread *cur_thread = (Thread *)cur_context;
    //printf("tid : %d\n", cur_thread->tid);

    //write trap frame
    //int sp = cur_thread->context.sp - 34*8;
    asm volatile("mov x2, %0" : :"r"(sp));  //sp, can't use x0
    asm volatile("mov x1, %0" : :"r"(cur_thread->tid)); //return value
    asm volatile("str x1, [x2, 16 * 0]");
}

void __uartread(size_t buf_addr, size_t size, size_t cur_ksp){

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

void __uartwrite(size_t buf_addr, size_t size, size_t cur_ksp){
    char *buf = (char *)buf_addr;

    //printf("uart write buf : %s, buf addr : %x\n", buf, buf_addr);
    
    asm volatile("msr DAIFClr, 0xf");
    uart_write(buf, size);
    asm volatile("msr DAIFSet, 0xf");

    //write trap frame
    asm volatile("mov x2, %0" : : "r"(cur_ksp));
    asm volatile("mov x1, %0" : : "r"(size));
    asm volatile("str x1, [x2, 16 * 0]");
}

int sys_exec(char *filename, char *const argv[]){
    char *img_cpio_addr = get_cpio_file(filename);
    size_t img_len = get_cpio_file_len(filename);

    size_t cur_context = _get_current_context();
    //printf("\ncontext : %lx\n", cur_context);
    Thread *task = (Thread *)cur_context;

    int order = 1;
    while( img_len > 0x1000 << order){
        order++;
    }

    task->code = alloc_page(order);
    task->codesize = img_len;
    print_pagetable(PA2KA((void *)task->pagetable), task->tid);

    //map code section
    mappages((pagetable_t)PA2KA(task->pagetable), 0, img_len, (uint64_t)task->code, PT_AF | PT_USER | PT_MEM | PT_RW, task);

    print_pagetable(PA2KA((void *)task->pagetable), task->tid);

    printf("\nfile %s\n", filename);

    memcpy(task->code, img_cpio_addr, img_len);

    to_el0(0, 0x0000fffffffff000, task->pagetable);

    return 1;
}


void __exec(size_t cur_ksp, size_t img_name_addr){
/*
    char *filename = (char *)img_name_addr;
    char *img_cpio_addr = get_cpio_file(filename);
    size_t img_len = get_cpio_file_len(filename);

    size_t cur_context = _get_current_context();
    Thread *task = (Thread *)cur_context;
    int order = 1;
    while( img_len > 0x1000 << order){
        order++;
    }
    task->code = alloc_page(order);
    task->codesize = img_len;

    mappages((pagetable_t)PA2KA(task->pagetable), 0, img_len,
           (uint64_t)task->code, PT_AF | PT_USER | PT_MEM | PT_RW);

    printf("\nfile %s\n", filename);

    memcpy(task->code, img_cpio_addr, img_len);

    printf("hi\n");

    to_el0(0, 0x0000fffffffff000, task->pagetable);

    //write trap frame, set elr to img addr
    // asm volatile("mov x1, %0" : : "r"(cur_ksp));
    // asm volatile("mrs x2, spsr_el1");
    // asm volatile("mov x3, %0" : : "r"((size_t)img_addr));    //elr
    // asm volatile("stp x2, x3, [x1, 16 * 16]");
*/
}


void __fork(size_t cur_ksp, size_t svc_lr){
    size_t parent_context = _get_current_context();
    Thread *parent_thread = (Thread *)parent_context;

    //create child thread
    Thread *child_thread = create_thread(0);

    //copy context
    memcpy(child_thread, parent_thread, sizeof(uint64_t) * 10);

    //copy user and kernel stack
    memcpy((void *)child_thread->ustack - 0x4000, (void *)parent_thread->ustack - 0x4000, 0x4000);
    memcpy((void *)child_thread->kstack - 0x1000, (void *)parent_thread->kstack - 0x1000, 0x1000);

    //copy on write
    //memcpy(PA2KA((void *)child_thread->pagetable), PA2KA((void *)parent_thread->pagetable), PGSIZE);
    // copy_pagetable(parent_thread, child_thread);
    // set_pte_RO(PA2KA((void *)parent_thread->pagetable), 0);
    // set_pte_RO(PA2KA((void *)child_thread->pagetable), 0);
    // print_pagetable(PA2KA((void *)child_thread->pagetable), child_thread->tid);

    print_mapped_pg_list(parent_thread);
    print_mapped_pg_list(child_thread);

    //use same code section
    child_thread->code = parent_thread->code;
    child_thread->codesize = parent_thread->codesize;
    //mappages((pagetable_t)PA2KA(child_thread->pagetable), 0, child_thread->codesize, (uint64_t)child_thread->code, PT_AF | PT_USER | PT_MEM | PT_RW, child_thread);

    //set child context's sp as parent's
    size_t offset = (size_t)parent_thread->kstack - cur_ksp;
    child_thread->context.sp = (size_t)child_thread->kstack - offset;

    //set lr
    parent_thread->context.lr = svc_lr;
    child_thread->context.lr = svc_lr;

    //write return value to parent's trap frame
    asm volatile("mov x1, %0" : : "r"(cur_ksp));
    asm volatile("mov x2, %0" : : "r"(child_thread->tid));
    asm volatile("str x2, [x1, 16 * 0]");

    //write return value to child's trap frame
    asm volatile("mov x1, %0" : : "r"(child_thread->context.sp));
    asm volatile("mov x2, #0");
    asm volatile("str x2, [x1, 16 * 0]");

    //no need to write sp_el0, casuse it's 0xfffffffff000, same as parent's, and it has already copy from parent's tf

    // size_t cur_usp;
    // asm volatile("mov x1, %0" : : "r"(cur_ksp));
    // asm volatile("ldr %0, [x1, 16 * 17]" : "=r"(cur_usp));

    // size_t cur_child_usp;
    // offset = (size_t)parent_thread->ustack - cur_usp;
    // cur_child_usp = (size_t)child_thread->ustack - offset;
    // asm volatile("mov x1, %0" : : "r"(child_thread->context.sp));
    // asm volatile("mov x2, %0" : : "r"(cur_child_usp));
    // asm volatile("str x2, [x1, 16 * 17]");

    //printf("\nfork : parent usp : %x, ksp : %x, child ups : %x, ksp : %x\n", parent_thread->ustack, parent_thread->kstack, child_thread->ustack, child_thread->kstack);
}

void __exit(size_t svc_lr){
    size_t cur_context = _get_current_context();
    Thread *cur_thread = (Thread *)cur_context;

    //printf("\ntid %d exit\n", cur_thread->tid);

    cur_thread->status = DEAD;

    //schedule();
}

void __mbox_call(size_t cur_ksp, size_t ch, int* mbox){
    size_t cur_context = _get_current_context();
    Thread *t = (Thread *)cur_context;

    unsigned int *new_mbox = (unsigned int *)((uint64_t)mbox - 0x0000ffffffffe000 + ((uint64_t)t->ustack - 0x1000));
    //printf("\nmbox : %x\n", mbox);
    //printf("new mbox : %x\n", new_mbox);

    int ret = mmbox_call(ch, new_mbox);

    //write trap frame
    asm volatile("mov x1, %0" : : "r"(cur_ksp));
    asm volatile("mov x2, %0" : : "r"(ret));
    asm volatile("str x2, [x1, 16 * 0]");
}

void __kill(int pid, size_t SIGTYPE){
    printf("\npid : %d will be killed, SIGTYPE : %d\n", pid, SIGTYPE);
    struct list_head *pos;
    list_for_each(pos, &run_queue->h_list){
        Thread *t = container_of(pos, Thread, t_list);
        if(t->tid == pid){
            //printf("die bitch\n");
            t->sig_types[SIGTYPE] = 1;
            break;
        }
    }
}

void __signal(size_t SIGTYPE, void (*register_handler)()){
    printf("\nSIGTYPE : %d\n", SIGTYPE);
    (register_handler)();
    size_t cur_context = _get_current_context();
    Thread *cur_thread = (Thread *)cur_context;
    cur_thread->register_sighands[SIGTYPE] = register_handler;
    cur_thread->sig_types[SIGTYPE] = 2;
}

void __sigreturn(size_t cur_ksp){
    printf("signal returnnnnnnnnnnnnnnnnnnnnnnnnnn\n");

    //require cur kernel stack pointer
    //get cur context (should be in kernel )
    size_t cur_context = _get_current_context();
    _switch_to_bottom(cur_context);
}

void system_call_handler(size_t esr_el1, size_t cur_ksp, size_t svc_lr){
    size_t sys_no, x0, x1;
    asm volatile("mov %0, x8" : "=r"(sys_no));
    asm volatile("mov x1, %0" : : "r"(cur_ksp));
    asm volatile("ldp %0, %1, [x1, 16 * 0]" : "=r"(x0), "=r"(x1));

    size_t Exception_Class = esr_el1 >> 26 & (0b111111);

    //page fault
    if(Exception_Class == INST_ABORT_L || Exception_Class == DATA_ABORT_L){

        if(Exception_Class == INST_ABORT_L)     printf("instruction abort\n");
        else                                    printf("data abort\n");

        size_t fault_check = esr_el1 & (0b111100);
        printf("fault check : %x\n", fault_check);

        if(fault_check == PERMI_FAULT){
            printf("[Permission fault]\n");
            cow_fault_handler();
        }
        else if(fault_check == TRANS_FAULT){
            printf("[Translation fault]\n");
            page_fault_handler();
        }
        else{
            printf("wtf not page fault\n");
            while(1);
        }
    }
    //system call
    else if(Exception_Class == SVC_INST){  
        size_t svc_no = esr_el1 & 0xF;
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
    }
    else{
        printf("fuck you it's real exception\n");
        while(1);
    }
    return;
}