int fork(){
    asm volatile("mov x8, #4");
    asm volatile("svc #0");

    int ret;
    asm volatile("mov %0, x0" : "=r"(ret));

    return ret;
}

int exec(char *name){
    asm volatile("mov x0, %0" : : "r"(name));
    asm volatile("mov x8, #3");
    asm volatile("svc #0");

    return 0;
}