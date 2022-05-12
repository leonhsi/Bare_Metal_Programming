#include "mm.h"
#include "buddy.h"
#include "utils.h"
#include "uart.h"
#include "thread.h"

extern size_t _get_current_context();

pte_t *walk(pagetable_t pagetable, uint64_t va, int alloc)
{
    for (int level = 3; level > 0; level--)
    {
        pte_t *pte = &pagetable[PX(level, va)];             //use va's index (9 bits) to find entry in pagetable
        if (*pte & PT_PAGE || *pte & PT_BLOCK)              //entry exist
        {   
            pagetable = (pagetable_t)PA2KA(PTE2PA(*pte));   //find pagetable base address according to entry value(pte)
        }
        else
        {
            if (!alloc || (pagetable = (pagetable_t)alloc_page(1)) == 0) return 0;
            memset(pagetable, 0, PGSIZE);
            *pte = KA2PA((uint64_t)pagetable) | PT_PAGE;    //set pte 
        }
    }
    return &pagetable[PX(0, va)];
}

void mappages(pagetable_t pagetable, uint64_t va, uint64_t size, uint64_t pa, int perm, Thread *t)
{
    add_mapped_page(t, (char *)va, (char *)pa, size, perm);

    uint64_t a, last;
    pte_t *pte;
    a = PGROUNDDOWN(va);
    last = PGROUNDDOWN(va + size - 1);
    for (; a != last + PGSIZE; a += PGSIZE, pa += PGSIZE){
        pte = walk(pagetable, a, 1);
        *pte = KA2PA(pa) | perm | PT_PAGE;
    }
}

void set_pte_RO(pagetable_t pagetable, int level){
    if(level == 3){
        for(int i=0; i<PGENTRY; i++){
            pte_t *last_pte = &pagetable[i];
            if(*last_pte & (PT_RO)){
                printf("already RO\n");
            }
            *last_pte |= PT_RO;   //set to readonly
        }
        return;
    }

    for(int i=0; i<PGENTRY; i++){
        pte_t *pte = &pagetable[i];
        if(*pte != 0){
            pagetable_t next_pagetable = (pagetable_t)PA2KA(PTE2PA(*pte));
            int next_level = level + 1;
            set_pte_RO(next_pagetable, next_level);
        }
        //*pte |= PT_RO;
    }
}

vma_struct* init_vma(usize_t start, usize_t end, int prot){
    vma_struct *vma = (vma_struct *)simple_malloc(sizeof(vma_struct));
    vma->start = start;
    vma->end = end;
    INIT_LIST_HEAD(&vma->vma_list);
    vma->prot = PROT_EXEC | PROT_READ | PROT_WRITE;

    return vma;
}

mapped_pg * init_mapped_pg(char *va, char *pa, uint64_t size, int perm){
    mapped_pg *pg = (mapped_pg *)simple_malloc(sizeof(mapped_pg));
    pg->va = va;
    pg->pa = pa;
    pg->size = size;
    pg->perm = perm;
    INIT_LIST_HEAD(&pg->mapped_pg_list);

    return pg;
}

mm_struct* init_mm(){
    mm_struct *mm = (mm_struct *)simple_malloc(sizeof(mm_struct));
    INIT_LIST_HEAD(&mm->vma_head);
    INIT_LIST_HEAD(&mm->mapped_pg_head);

    vma_struct *vma = init_vma(0, 0, 0);
    list_add_tail(&vma->vma_list, &mm->vma_head);

    return mm;
}

void add_mapped_page(Thread *t, char *va, char *pa, uint64_t size, int perm){
    mapped_pg *pg = init_mapped_pg(va, pa, size, perm);
    list_add_tail(&pg->mapped_pg_list, &t->mm->mapped_pg_head);
}

void copy_pagetable(Thread *src_thread, Thread *dst_thread){
    //BUG
    //wrong physical address
    
    //TODO
    //walk through parent's pagetable, find PTE's pte
    //walk through child's pagetable, write PTE's pte as parent's

    struct list_head *tmp;
    list_for_each(tmp, &src_thread->mm->mapped_pg_head){
        mapped_pg *src_pg = container_of(tmp, mapped_pg, mapped_pg_list);
        if((uint64_t)src_pg->va != 0x3c100000){     //mail box has already mapped during thread creating
            //mappages((pagetable_t)PA2KA(dst_thread->pagetable), (uint64_t)src_pg->va, src_pg->size, (uint64_t)src_pg->pa, src_pg->perm, dst_thread);
            add_mapped_page(dst_thread, (char *)src_pg->va, (char *)src_pg->pa, src_pg->size, src_pg->perm);

            uint64_t a, last;
            pte_t *src_pte, *dst_pte;
            a = PGROUNDDOWN((uint64_t)src_pg->va);
            last = PGROUNDDOWN((uint64_t)src_pg->va + src_pg->size - 1);
            for (; a != last + PGSIZE; a += PGSIZE, src_pg->pa += PGSIZE){
                src_pte = walk((pagetable_t)PA2KA(src_thread->pagetable), a, 1);
                dst_pte = walk((pagetable_t)PA2KA(dst_thread->pagetable), a, 1);
                //printf("copy parent pte : %x(%x), child pte : %x(%x)\n", src_pte, *src_pte, dst_pte, *dst_pte);
                *dst_pte = *src_pte;
            }
        }
    }
}

int can_write_vma(vma_struct *vma){
    if(vma->prot & PROT_WRITE){     //can write
        return 1;
    }
    return 0;
}

int is_legal_vma(mm_struct *mm, size_t addr){
    struct list_head *tmp;
    list_for_each(tmp ,&mm->vma_head){
        vma_struct *vma = container_of(tmp, vma_struct, vma_list);
        if(can_write_vma(vma) && (addr >= vma->start || addr <= vma->end) ){
            //printf("it's legal address in vma, start : %x, end : %x\n", vma->start, vma->end);
            return 1;
        }
    }
    return 0;
}

void page_fault_handler(){
    size_t cur_context = _get_current_context();
    Thread *cur_thread = (Thread *)cur_context;

    size_t far_el1;
    asm volatile("mrs x1, far_el1");
    asm volatile("mov %0, x1" : "=r"(far_el1));

    printf("page fault at %x, tid : %d, pagetable : %x\n", far_el1, cur_thread->tid, cur_thread->pagetable);   

    if(is_legal_vma(cur_thread->mm, far_el1)){
        char *pa = alloc_page(1);
        printf("allocate page at pa : %x\n", (uint64_t)(pa));
        mappages(PA2KA((void *)cur_thread->pagetable), far_el1 & (~0xFFF), 4096, (uint64_t)pa, PT_AF | PT_USER | PT_MEM | PT_RW, cur_thread);
        //print_pagetable(PA2KA((void *)cur_thread->pagetable), cur_thread->tid);
    }
    else{
        printf("[Segmentation fault]: Kill Process\n");
    }
}

void cow_fault_handler(){
    size_t cur_context = _get_current_context();
    Thread *cur_thread = (Thread *)cur_context;

    size_t far_el1;
    asm volatile("mrs x1, far_el1");
    asm volatile("mov %0, x1" : "=r"(far_el1));

    printf("cow fault at %x, tid : %d, pagetable : %x\n", far_el1, cur_thread->tid, cur_thread->pagetable); 

    //TODO
    //walk through pagetable to get pa 
    pte_t *pte = walk(PA2KA((void *)cur_thread->pagetable), far_el1 & (~0xFFF), 0);

    //copy page content to a new page
    char *pa = (char *)PA2KA((*pte & (~0xFFF)));
    char *new_pa = alloc_page(1);
    printf("readonly pa : %x, new rw pa : %x\n", (uint64_t)pa, (uint64_t)new_pa);
    memcpy(new_pa, pa, 0x1000);
    printf("finish memcpy\n");

    //map new page to pagetable and set permission to RW
    mappages(PA2KA((void *)cur_thread->pagetable), far_el1 & (~0xFFF), 0x1000, (uint64_t)new_pa, PT_AF | PT_USER | PT_MEM | PT_RW, cur_thread);
}

void print_pud(pagetable_t pagetable){
    for(int i=0; i<PGENTRY; i++){
        pte_t *pte = &pagetable[i];
        if(*pte !=0 && *pte != 0x80){
            printf("\t-entry : %d, value : %x\n", i, *pte);
        }
    }
}

void print_pagetable(pagetable_t pagetable, int tid){
    printf("\n===========\n");
    printf("tid : %d\n", tid);
    for(int i=0; i<PGENTRY; i++){
        pte_t *pte = &pagetable[i];
        if(*pte != 0 && *pte != 0x80){
        //if(*pte != 0){
            printf("entry : %d, value : %x\n", i, *pte);
            pagetable_t next_pagetable = (pagetable_t)PA2KA(PTE2PA(*pte));
            print_pud(next_pagetable);
        }
    }
    printf("===========\n");
}

void print_mapped_pg_list(Thread *t){
    printf("\ntid %d : ", t->tid);
    struct list_head *tmp;
    list_for_each(tmp, &t->mm->mapped_pg_head){
        mapped_pg *pg = container_of(tmp, mapped_pg, mapped_pg_list);
        printf("va(%x), pa(%x), size(%d) -> ", pg->va, pg->pa, pg->size);
    }
    printf("\n\n");
}