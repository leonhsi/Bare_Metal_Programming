.global to_el0
to_el0:  
    msr     elr_el1, x0
    msr     sp_el0, x1

    mov     x3, #0
    msr     spsr_el1, x3

    dsb ish // ensure write has completed
    msr ttbr0_el1, x2 // switch translation based address.
    tlbi vmalle1is // invalidate all TLB entries
    dsb ish // ensure completion of TLB invalidatation

    // set up exception handlers
    ldr     x2, =exception_vector_table
    msr     vbar_el1, x2

    isb // clear pipeline

    eret

.global _switch_to
_switch_to:
    stp x19, x20, [x0, 16 * 0]
    stp x21, x22, [x0, 16 * 1]
    stp x23, x24, [x0, 16 * 2]
    stp x25, x26, [x0, 16 * 3]
    stp x27, x28, [x0, 16 * 4]
    stp fp, lr, [x0, 16 * 5]
    mov x9, sp
    str x9, [x0, 16 * 6]

    ldp x19, x20, [x1, 16 * 0]
    ldp x21, x22, [x1, 16 * 1]
    ldp x23, x24, [x1, 16 * 2]
    ldp x25, x26, [x1, 16 * 3]
    ldp x27, x28, [x1, 16 * 4]
    ldp fp, lr, [x1, 16 * 5]
    ldp x9, x0, [x1, 16 * 6]
    mov sp,  x9
    msr tpidr_el1, x1

    dsb ish // ensure write has completed
    msr ttbr0_el1, x2 // switch translation based address.
    tlbi vmalle1is // invalidate all TLB entries
    dsb ish // ensure completion of TLB invalidatation
    
    ret //return to lr which store task address

.global _switch_to_top
_switch_to_top:
    stp x19, x20, [x0, 16 * 0]
    stp x21, x22, [x0, 16 * 1]
    stp x23, x24, [x0, 16 * 2]
    stp x25, x26, [x0, 16 * 3]
    stp x27, x28, [x0, 16 * 4]
    stp fp, lr, [x0, 16 * 5]
    mov x9, sp
    str x9, [x0, 16 * 6]
    ret

.global _switch_to_bottom
_switch_to_bottom:
    ldp x19, x20, [x0, 16 * 0]
    ldp x21, x22, [x0, 16 * 1]
    ldp x23, x24, [x0, 16 * 2]
    ldp x25, x26, [x0, 16 * 3]
    ldp x27, x28, [x0, 16 * 4]
    ldp fp, lr, [x0, 16 * 5]
    ldr x9, [x0, 16 * 6]
    mov sp,  x9
    msr tpidr_el1, x0
    ret //return to lr which store task address

.global _get_current_context
_get_current_context:
    mrs x0, tpidr_el1
    ret

1:
    bl shell
    b  1b

.global _ret_shell
_ret_shell:
    adr x0, 1b
    mov lr, x0
    ret