//
// Created by dustyn on 7/2/24.
//
#include <include/arch/arch_cpu.h>
#include <include/arch/arch_interrupts.h>
#include <include/arch/arch_memory_init.h>
#include <include/arch/x86_64/arch_asm_functions.h>
#include "include/types.h"
#include "include/uart.h"
#include "include/arch/arch_local_interrupt_controller.h"
#include "limine.h"

int8 panicked = 0;
cpu cpu_list[8];

#ifdef __x86_64__

void panic(const char* str) {
    cli();
    write_string_serial("\nPanic! ");
    write_string_serial(str);
    panicked = 1;
    for (;;) {
        asm("hlt");
        asm("nop");
    }
}

uint64 arch_mycpu() {
    return lapic_get_id();
}

void arch_initialise_cpu( struct limine_smp_info *smp_info) {
    for(;;) {
        asm("hlt");
    }

}
#endif
