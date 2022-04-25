
.section ".text.boot"

.global _start

_start:
1:
    // clear bss
    ldr     x1, =__bss_start
    ldr     w2, =__bss_size
    cbz     w2, 2f
    str     xzr, [x1], #8
    sub     w2, w2, #1
    cbnz    w2, 1b

2:
	bl      main
    // for failsafe, halt this core too
    b       1b