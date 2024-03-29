.section ".text.boot"

.global _start

_start:
    // read cpu id, stop slave cores
    mov 	x3, x0          //store dtb address
    mrs     x1, mpidr_el1
    and     x1, x1, #3
    cbz     x1, 2f

    // cpu id > 0, stop
1:  wfe
    b       1b

2:  // cpu id == 0

    // set top of stack just before our code (stack grows to a lower address per AAPCS64)
    ldr     x1, =_start

	//running at el2?
    mrs 	x0, CurrentEL
    and 	x0, x0, #12
    cmp 	x0, #8          
    bne 	5f

    msr 	sp_el1, x1      //stack pointer

    // enable AArch64 in EL1
    mov 	x0, (1 << 31)   // EL1 uses aarch64
    msr 	hcr_el2, x0

    // make el0, el1 can use Floating point and Advanced SIMD, so can use printf
    mrs 	x0, cpacr_el1
    orr		x0, x0, #(3 << 20)
    msr 	cpacr_el1, x0

    // change execution level to EL1
    mov     x0, 0x5         // EL1h  with interrupt enable (DAIF = 0)
    //mov 	x0, 0x3c5       // EL1h (SPSel = 1) with interrupt disabled (DAIF = 1)
    msr 	spsr_el2, x0
    adr 	x2, 5f
    msr 	elr_el2, x2

	//setup excption handler
	adr 	x0, exception_vector_table
	msr		vbar_el1, x0

    //return to EL1
    eret                

5:  mov     sp, x1

    // clear bss
    ldr     x1, =__bss_start
    ldr     w2, =__bss_size
3:  cbz     w2, 4f
    str     xzr, [x1], #8
    sub     w2, w2, #1
    cbnz    w2, 3b

4:  //restore dtb address
	mov 	x0, x3     
    //get boot time
    mrs     x1, cntpct_el0
  	// jump to C code, should not return
	bl      main
    // for failsafe, halt this core too
    b       1b


