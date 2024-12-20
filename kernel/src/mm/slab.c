//
// Created by dustyn on 6/22/24.
//
#include <stdint.h>
#include <stddef.h>
#include "include/mem/slab.h"

#include <include/arch/arch_cpu.h>
#include <include/arch/arch_paging.h>
#include <include/data_structures/hash_table.h>

#include "include/mem/pmm.h"
#include "include/mem/mem.h"
#include "include/drivers/serial/uart.h"

//Kernel heap
struct slab_t slabs[10];
/*
 * Create a slab of physical memory,
 */

struct hash_table slab_hash;
void heap_create_slab(struct slab_t *slab, uint64_t entry_size,uint64_t pages) {
    slab->first_free = P2V(phys_alloc(pages));
    slab->entry_size = entry_size;
    slab->start_address = slab->first_free;
    slab->end_address = (slab->first_free + (pages * (PAGE_SIZE)));

    serial_printf("Slab created at %x.64\n",slab->first_free);
    uint64_t header_offset = (sizeof(struct header) + (entry_size)) / entry_size * entry_size;
    uint64_t available_size = (pages * PAGE_SIZE) - header_offset;
    struct header *slab_pointer = (struct header *) slab->first_free;

    slab_pointer->slab = slab;
    slab->first_free = (void **) ((void *) slab->first_free + header_offset);

    void **array = (void **) slab->first_free;

    uint64_t max = available_size / entry_size - 1;
    uint64_t factor = entry_size / sizeof(void *);

    for (uint64_t i = 0; i < max; i++) {
        array[i * factor] = &array[(i + 1) * factor];
    }
    array[max * factor] = NULL;
}



void *heap_allocate_from_slab(struct slab_t *slab) {
    if (slab->first_free == NULL) {
        heap_create_slab(slab, slab->entry_size,PAGE_SIZE * 2);
    }
    void **old_free = slab->first_free;
    slab->first_free = *old_free;
    memset(old_free, 0, slab->entry_size);
    return old_free;
}




/*
 * Free part of a slab, setting it new head to the first_free field
 * This can be useful
 */

void heap_free_in_slab(struct slab_t *slab, void *address) {
    if (slab == NULL || (uint64_t) slab < 0x1000) {
        serial_printf("NULL Slab Found Aborting Free. Leaked Memory. Address : %x.64 Slab Address %x.16\n",address, slab);
        return;
    }
    if (address == NULL) {
        return;
    }

    //ensure address is in fact part of this slab
    if(address > slab->end_address || address < slab->start_address){
        return;
    }

    void **new_head = address;
    *new_head = slab->first_free;
    slab->first_free = new_head;
}

