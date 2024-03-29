#ifndef SYSCALL_H
#define SYSCALL_H

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

void __get_pid(int sp);
void __uartwrite(long long buf_addr, long long size, long long cur_ksp);
void __uartread(long long buf_addr, long long size, long long cur_ksp);
int sys_exec(char *filename);
void __exec(int cur_ksp, long long img_name_addr);
void __fork(int cur_ksp, int svc_lr);
void __exit(int svc_lr);
void __mailbox_call(long long cur_ksp, long long ch, int* mbox);
void __kill(int pid, long long SIGTYPE);
void __signal(long long SIGTYPE, void (*register_handler)());
void __sigreturn();

void copy_context();
void system_call_handler(int esr_el1, int cur_ksp, int svc_lr);

#endif