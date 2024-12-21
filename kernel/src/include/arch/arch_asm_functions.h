//
// Created by dustyn on 8/30/24.
//


#ifdef __x86_64__
#pragma once
#include <include/mem/kalloc.h>
#include "include/arch/x86_64/asm_functions.h"
#include "include/scheduling/process.h"

static inline void enable_interrupts() {
  cli();
}

static inline void disable_interrupts() {
  sti();
}

static inline uint64_t are_interrupts_enabled() {
  return interrupts_enabled();
}

static inline void nop(){
    asm("nop");
}


#endif
