#define TCR_CONFIG_REGION_48bit (((64 - 48) << 0) | ((64 - 48) << 16))
#define TCR_CONFIG_4KB ((0b00 << 14) |  (0b10 << 30))   //TG1 and TG0 : 4KB
#define TCR_CONFIG_DEFAULT (TCR_CONFIG_REGION_48bit | TCR_CONFIG_4KB)

#define MAIR_DEVICE_nGnRnE 0b00000000
#define MAIR_NORMAL_NOCACHE 0b01000100
#define MAIR_IDX_DEVICE_nGnRnE 0
#define MAIR_IDX_NORMAL_NOCACHE 1
#define MAIR_ATTRIBUTE ( (MAIR_DEVICE_nGnRnE << (MAIR_IDX_DEVICE_nGnRnE * 8)) | \
                        (MAIR_NORMAL_NOCACHE << (MAIR_IDX_NORMAL_NOCACHE * 8)) )

#define PD_TABLE 0b11
#define PD_BLOCK 0b01
#define PD_ACCESS (1 << 10)
#define BOOT_PGD_ATTR PD_TABLE
#define BOOT_PUD_ATTR (PD_ACCESS | (MAIR_IDX_DEVICE_nGnRnE << 2) | PD_BLOCK)

.section ".text.boot"

.global _start

_start:
    // read cpu id, stop slave cores
    mov 	x8, x0          //store dtb address
    mrs     x1, mpidr_el1
    and     x1, x1, #3
    cbz     x1, 2f

    // cpu id > 0, stop
1:  wfe
    b       1b

2:  
    // cpu id == 0
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
5:          
    @ ldr x0, = TCR_CONFIG_DEFAULT
    @ msr tcr_el1, x0

    @ ldr x0, = MAIR_ATTRIBUTE
    @ msr mair_el1, x0

    @ mov x0, 0 // PGD's page frame at 0x0
    @ mov x1, 0x1000 // PUD's page frame at 0x1000

    @ ldr x2, = BOOT_PGD_ATTR
    @ orr x2, x1, x2 // combine the physical address of next level page with attribute.
    @ str x2, [x0]

    @ ldr x2, = BOOT_PUD_ATTR
    @ mov x3, 0x00000000
    @ orr x3, x2, x3
    @ str x3, [x1] // 1st 1GB mapped by the 1st entry of PUD
    @ mov x3, 0x40000000
    @ orr x3, x2, x3
    @ str x3, [x1, 8] // 2nd 1GB mapped by the 2nd entry of PUD

    @ msr ttbr0_el1, x0 // load PGD to the bottom translation based register.
    @ msr ttbr1_el1, x0 // also load PGD to the upper translation based register.
    @ isb

    @ mrs x2, sctlr_el1
    @ orr x2 , x2, 1
    @ //mov x2, #1
    @ msr sctlr_el1, x2 // set M bit and enable MMU. cache remains disabled
    @ isb

    @ ldr x2, = _boot_rest
    @ br  x2

_boot_rest:     
    //mov     sp, x1

    // clear bss
    ldr     x1, =__bss_start
    ldr     w2, =__bss_size
3:  cbz     w2, 4f
    str     xzr, [x1], #8
    sub     w2, w2, #1
    cbnz    w2, 3b


4:  //restore dtb address
	mov 	x0, x8     
    //get boot time
    mrs     x1, cntpct_el0
  	// jump to C code, should not return
	bl      main
    // for failsafe, halt this core too
    b       1b


