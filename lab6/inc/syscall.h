#ifndef SYSCALL__H
#define SYSCALL__H

int get_pid();
long long uartread(char buf[], long long size);
long long uartwrite(const char buf[], long long size);
int exec(char *name);
int fork();
void exit();
int mbox_call(unsigned char ch, volatile unsigned int *mbox);
void kill(int pid, int SIGTYPE);
void signal(long long SIGTYPE, void *register_handler);
void sigreturn();

void __get_pid(long long sp);
void __uartwrite(long long buf_addr, long long size, long long cur_ksp);
void __uartread(long long buf_addr, long long size, long long cur_ksp);
void __exec(long long cur_ksp, long long img_name_addr);
int sys_exec(char *filename, char *const argv[]);
void __fork(long long cur_ksp, long long svc_lr);
void __exit(long long svc_lr);
void __mailbox_call(long long cur_ksp, long long ch, int* mbox);
void __kill(int pid, long long SIGTYPE);
void __signal(long long SIGTYPE, void (*register_handler)());
void __sigreturn();

void copy_context();
void system_call_handler(long long esr_el1, long long cur_ksp, long long svc_lr);

#endif