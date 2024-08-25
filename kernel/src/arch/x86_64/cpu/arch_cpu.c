//
// Created by dustyn on 7/2/24.
//
#include "include/types.h"
#include "include/cpu.h"
#include "include/uart.h"
#include "include/arch/arch_local_interrupt_controller.h"

int8 panicked = 0;
local_cpu local_cpu_info[16];
#ifdef __x86_64__

void arch_panic(const char *str){
    write_string_serial("\nPanic! ");
    write_string_serial(str);
    panicked = 1;
    asm("cli");
    for(;;){
      asm("hlt");
      }
}

uint64 arch_mycpu(){
    return lapic_get_id();
}
#endif