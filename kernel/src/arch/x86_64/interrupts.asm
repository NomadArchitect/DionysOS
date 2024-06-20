;generated by interrupts.pl - do not edit
;isr_routine_table table
extern divide_by_zero
extern debug_exception
extern nmi_interrupt
extern breakpoint
extern overflow
extern bounds_check
extern illegal_opcode
extern device_not_available
extern double_fault
extern isr_routine_9
extern invalid_tss
extern segment_not_present
extern stack_exception
extern general_protection_fault
extern page_fault
extern isr_routine_15
extern floating_point_error
extern alignment_check
extern machine_check
extern simd_floating_point_error

%macro pushaq 0
    push rax      ;save current rax
    push rbx      ;save current rbx
    push rcx      ;save current rcx
    push rdx      ;save current rdx
    push rbp      ;save current rbp
    push rdi       ;save current rdi
    push rsi       ;save current rsi
    push r8        ;save current r8
    push r9        ;save current r9
    push r10      ;save current r10
    push r11      ;save current r11
    push r12      ;save current r12
    push r13      ;save current r13
    push r14      ;save current r14
    push r15      ;save current r15
%endmacro

%macro popaq 0
    pop r15      ;save current r15
    pop r14      ;save current r14
    pop r13      ;save current r13
    pop r12      ;save current r12
    pop r11      ;save current r11
    pop r10      ;save current r10
    pop r9        ;save current r9
    pop r8        ;save current r8
    pop rsi       ;save current rsi
    pop rdi       ;save current rdi
    pop rbp      ;save current rbp
    pop rdx      ;save current rdx
    pop rcx      ;save current rcx
    pop rbx      ;save current rbx
    pop rax      ;save current rax

%endmacro

global isr_wrapper_0
isr_wrapper_0:
    pushaq
    call divide_by_zero
    popaq
    iretq
global isr_wrapper_1
isr_wrapper_1:
    pushaq
    call debug_exception
    popaq
    iretq

global isr_wrapper_2
isr_wrapper_2:
    pushaq
    call nmi_interrupt
    popaq
    iretq

global isr_wrapper_3
isr_wrapper_3:
  pushaq
  call breakpoint
  popaq
  iretq

global isr_wrapper_4
isr_wrapper_4:
  pushaq
  call overflow
  popaq
  iretq

global isr_wrapper_5
isr_wrapper_5:
  pushaq
  call bounds_check
  popaq
  iretq

global isr_wrapper_6
isr_wrapper_6:
  pushaq
  call illegal_opcode
  popaq
  iretq

global isr_wrapper_7
isr_wrapper_7:
  pushaq
  call device_not_available
  popaq
  iretq

global isr_wrapper_8
isr_wrapper_8:
  pushaq
  call double_fault
  popaq
  iretq

global isr_wrapper_10
isr_wrapper_10:
  pushaq
  call invalid_tss
  popaq
  iretq

global isr_wrapper_11
isr_wrapper_11:
  pushaq
  call segment_not_present
  popaq
  iretq

global isr_wrapper_12
isr_wrapper_12:
  pushaq
  call stack_exception
  popaq
  iretq

global isr_wrapper_13
isr_wrapper_13:
  pushaq
  call general_protection_fault
  popaq
  iretq

global isr_wrapper_14
isr_wrapper_14:
  pushaq
  call page_fault
  popaq
  iretq

global isr_wrapper_16
isr_wrapper_16:
  pushaq
  call floating_point_error
  popaq
  iretq

global isr_wrapper_17
isr_wrapper_17:
  pushaq
  call alignment_check
  popaq
  iretq

global isr_wrapper_18
isr_wrapper_18:
  pushaq
  call machine_check
  popaq
  iretq

global isr_wrapper_19
isr_wrapper_19:
  pushaq
  call simd_floating_point_error
  popaq
  iretq
