int get_pid();
long long uartread(char buf[], long long size);
long long uartwrite(const char buf[], long long size);
int exec(char *name);
int fork();
void exit();
int mbox_call(unsigned char ch, unsigned int *mbox);

void __get_pid(int sp);
void __uartwrite(long long buf_addr, long long size, long long cur_ksp);
void __uartread(long long buf_addr, long long size, long long cur_ksp);
void __exec(int cur_ksp, long long img_cpio_addr, long long img_len);
void __fork(int cur_ksp, int svc_lr);
void __exit(int svc_lr);
void __mailbox_call(long long cur_ksp, long long ch, int* mbox);

void copy_context();
void system_call_handler(int esr_el1, int cur_ksp, int svc_lr);