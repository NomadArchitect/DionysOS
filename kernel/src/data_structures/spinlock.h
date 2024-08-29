//
// Created by dustyn on 6/21/24.
//

#pragma once
#include "include/types.h"
#include <include/arch/arch_cpu.h>
// Mutual exclusion lock.

struct spinlock{
    uint64 locked;
    char* name;
    cpu* cpu;
    uint64 program_counters[10];
};

