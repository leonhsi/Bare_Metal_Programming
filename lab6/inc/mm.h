#ifndef MEM_H
#define MEM_H

#include "stdint.h"

typedef uint64_t pte_t;
typedef uint64_t *pagetable_t;

#define PA2KA(a) (a + 0xffff000000000000)
#define KA2PA(a) (a - 0xffff000000000000)

#define PGSIZE 4096 // bytes per page
#define PGSHIFT 12  // bits of offset within a page

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

void kvmmap(uint64_t va, uint64_t pa, uint64_t sz, int perm);
void mappages(pagetable_t pagetable, uint64_t va, uint64_t size, uint64_t pa,
              int perm);

#endif // !MEM_H