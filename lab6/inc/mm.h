#ifndef MEM__H
#define MEM__H

#include "stdint.h"
#include "thread.h"

typedef uint64_t pte_t;
typedef uint64_t *pagetable_t;

#define PA2KA(a) (a + 0xffff000000000000)
#define KA2PA(a) (a - 0xffff000000000000)

#define PGSIZE 4096 // bytes per page
#define PGSHIFT 12  // bits of offset within a page
#define PGENTRY 512

#define PGROUNDUP(sz) (((sz) + PGSIZE - 1) & ~(PGSIZE - 1))
#define PGROUNDDOWN(a) (((a)) & ~(PGSIZE - 1))

#define PA2PTE(pa) ((((uint64_t)pa) >> 12) << 12)
#define PTE2PA(pte) (pte & 0xfffffffffffff000)

// extract the three 9-bit page table indices from a virtual address.
#define PXMASK 0x1FF // 9 bits
#define PXSHIFT(level) (PGSHIFT + (9 * (level)))
#define PX(level, va) ((((uint64_t)(va)) >> PXSHIFT(level)) & PXMASK)

// granularity
#define PT_PAGE 0b11  // 4k granule
#define PT_BLOCK 0b01 // 2M granule
// accessibility
#define PT_KERNEL (0 << 6) // privileged, supervisor EL1 access only
#define PT_USER (1 << 6)   // unprivileged, EL0 access allowed
#define PT_RW (0 << 7)     // read-write
#define PT_RO (1 << 7)     // read-only
#define PT_AF (1 << 10)    // accessed flag
#define PT_NX (1UL << 54)  // no execute
// defined in MAIR register
#define PT_MEM (0 << 2) // normal memory
#define PT_DEV (1 << 2) // device MMIO
#define PT_NC (2 << 2)  // non-cachable
// prot
#define PROT_NONE 0
#define PROT_READ 1
#define PROT_WRITE 2
#define PROT_EXEC 4


//void kvmmap(uint64_t va, uint64_t pa, uint64_t sz, int perm);
pte_t *walk(pagetable_t pagetable, uint64_t va, int alloc);
void mappages(pagetable_t pagetable, uint64_t va, uint64_t size, uint64_t pa, int perm, Thread *t);

void set_pte_RO(pagetable_t pagetable, int level);

vma_struct* init_vma(usize_t start, usize_t end, int prot);
mapped_pg * init_mapped_pg(char *va, char *pa, uint64_t size, int perm);
mm_struct* init_mm();

void add_mapped_page(Thread *t, char *va, char *pa, uint64_t size, int perm);
void copy_pagetable(Thread *src_thread, Thread *dst_thread);

int can_write_vma(vma_struct *vma);
int is_legal_vma(mm_struct *mm, size_t addr);

void page_fault_handler();
void cow_fault_handler();

void print_pagetable(pagetable_t pagetable, int tid);
void print_mapped_pg_list(Thread *t);

#endif // !MEM_H