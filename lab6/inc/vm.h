struct vm_area_struct{
    unsigned long vm_start;
    unsigned long vm_end;

    struct vm_area_struct *vm_prev;
    struct vm_area_struct *vm_next;

    struct mm_struct *vm_mm;

    int vma_page_prot;
};

struct mm_struct{
    struct vm_area_struct *mmap;    //head of vma list

    unsigned long mm_base;

    unsigned long start_code, end_code, start_data, end_data;
    unsigned long start_brk, brk, start_stack;

    int mm_count;

    char *pgd;
};

struct task_struct{
    struct mm_struct mm;
};