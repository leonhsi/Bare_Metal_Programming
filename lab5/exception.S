#define CORE0_TIMER_IRQ_CTRL 0x40000040

.macro save_all
  sub   sp, sp, 16 * 18
  stp   x0, x1, [sp ,16 * 0]
  stp   x2, x3, [sp ,16 * 1]
  stp   x4, x5, [sp ,16 * 2]
  stp   x6, x7, [sp ,16 * 3]
  stp   x8, x9, [sp ,16 * 4]
  stp   x10, x11, [sp ,16 * 5]
  stp   x12, x13, [sp ,16 * 6]
  stp   x14, x15, [sp ,16 * 7]
  stp   x16, x17, [sp ,16 * 8]
  stp   x18, x19, [sp ,16 * 9]
  stp   x20, x21, [sp ,16 * 10]
  stp   x22, x23, [sp ,16 * 11]
  stp   x24, x25, [sp ,16 * 12]
  stp   x26, x27, [sp ,16 * 13]
  stp   x28, x29, [sp ,16 * 14]
  str   x30, [sp, 16 * 15]
  mrs   x2, spsr_el1
  mrs   x3, elr_el1
  stp   x2, x3, [sp, 16 * 16]
  mrs   x2, sp_el0
  str   x2, [sp, 16 * 17]
.endm

// load general registers from stack
.macro load_all
  ldr   x2, [sp, 16 * 17]
  msr   sp_el0, x2
  ldp   x2, x3, [sp, 16 * 16]
  msr   spsr_el1, x2
  msr   elr_el1, x3
  ldp   x0, x1, [sp ,16 * 0]
  ldp   x2, x3, [sp ,16 * 1]
  ldp   x4, x5, [sp ,16 * 2]
  ldp   x6, x7, [sp ,16 * 3]
  ldp   x8, x9, [sp ,16 * 4]
  ldp   x10, x11, [sp ,16 * 5]
  ldp   x12, x13, [sp ,16 * 6]
  ldp   x14, x15, [sp ,16 * 7]
  ldp   x16, x17, [sp ,16 * 8]
  ldp   x18, x19, [sp ,16 * 9]
  ldp   x20, x21, [sp ,16 * 10]
  ldp   x22, x23, [sp ,16 * 11]
  ldp   x24, x25, [sp ,16 * 12]
  ldp   x26, x27, [sp ,16 * 13]
  ldp   x28, x29, [sp ,16 * 14]
  ldr   x30, [sp, 16 * 15]
  add   sp, sp, 16 * 18;
.endm

1:bl  shell
	b   1b

.global _from_el1_to_el0
_from_el1_to_el0:
	//process state, return exception level is 0
  mov 	x0, 0x0       //I = 0, enable interrupt
  msr 	spsr_el1, x0
	//return address
  adr 	x2, 1b
  msr 	elr_el1, x2
	//stack pointer
  //ldr 	x1, =0x60000
  //msr 	sp_el0, x1
  eret

.align 11 
.global exception_vector_table
exception_vector_table: 
	// synchronous
  mov   x0, 0
  b     exception_handler
  .align  7
  // IRQ
  mov   x0, 1
  b     exception_handler
  .align  7
  // FIQ
  mov   x0, 2
  b     exception_handler
  .align  7
  // SError
  mov   x0, 3
  b     exception_handler
  .align  7
	
	//Synchronous
  //mov   x0, 0
	//b     exception_handler
  b     _system_call_saver
  .align 7
	//IRQ
  b     _irq_saver
  .align 7
	//FIQ
  mov   x0, 2
	b     exception_handler
  .align 7
	//SError
  mov   x0, 3
  b     exception_handler
  .align 7

	//Synchronous
  //mov   x0, 0
  //b     exception_handler
  b     _system_call_saver
  .align 7
  //IRQ
  //mov   x0, 1
  b     _irq_saver
  .align 7
  //FIQ
  mov   x0, 2
  b     exception_handler
  .align 7
  //SError
  mov   x0, 3
  b     exception_handler
  .align 7

  //Synchronous
  mov   x0, 0
  b     exception_handler
  .align 7
  //IRQ
  mov   x0, 1
  b     exception_handler
  .align 7
  //FIQ
  mov   x0, 2
  b     exception_handler
  .align 7
  //SError
  mov   x0, 3
  b     exception_handler
  .align 7

exception_handler:
  save_all
  mrs   x1, esr_el1
  mrs   x2, elr_el1
  mrs   x3, spsr_el1
  mrs   x4, far_el1
  bl    exception_log
  load_all
  eret

_system_call_saver:
  save_all
  mrs   x0, esr_el1
  mov   x1, sp        //cur_sp, sp_el1
  adr   x2, 2f        //svc_lr
  bl system_call_handler
2:  load_all
  eret

_irq_saver:
  save_all
  adr   x0, 3f
  bl irq_handler
3:  load_all
  eret

.global _uart_irq
_uart_irq:
  mov   x0, 1
  mrs   x1, esr_el1
  mrs   x2, elr_el1
  mrs   x3, spsr_el1
  mrs   x4, far_el1
  //bl    exception_log
  bl    uart_irq_handler
  ret

.global _timeout_message_handler
_timeout_message_handler:
  //disable
  mov   x2, 0
  msr   cntp_ctl_el0, x2
  //store lr to return to vector table
  mov   x6, lr
  bl    timeout
  mov   lr, x6
  //configure eret return address to shell
  adr   x2, 1b
  msr   elr_el1, x2
  ret

.section ".text"
.global _user
_user:
	mov 	x1, #0
4:
	add 	x1, x1, #1
	svc		#0
	cmp 	x1, #5
	blt		4b

  bl    shell

.global _set_core_timer
_set_core_timer:
  mrs   x1, cntfrq_el0
  msr   cntp_tval_el0, x1
  ret

.global _core_timer_enable
_core_timer_enable:
  //x0 : timeout sec
  //set expired time
  mrs   x1, cntfrq_el0
  //mul   x1, x1, x0    //x1 = freq * sec
  msr   cntp_tval_el0, x1
  //enable
  mov   x2, 1
  msr   cntp_ctl_el0, x2
  //unmask timer interrupt 
  mov   x3, 2
  ldr   x1, =CORE0_TIMER_IRQ_CTRL
  str   w3, [x1]
  ret