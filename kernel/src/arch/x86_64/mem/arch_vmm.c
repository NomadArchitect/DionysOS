//
// Created by dustyn on 6/25/24.
//

/*
 * Taking inspo from xv6 to keep things as readable and simple as possible here.
 */

#include "include/types.h"
#include "include/mem.h"
#include "include/arch_paging.h"
#include "include/pmm.h"
#include "include/kalloc.h"
#include "include/arch_asm_functions.h"
#include "include/mem_bounds.h"
#include "include/uart.h"
#include "include/cpu.h"
#include "include/arch_vmm.h"
#include "include/arch_asm_functions.h"

p4d_t* global_pg_dir = 0;
struct virt_map* kernel_pg_map;

void switch_page_table(p4d_t* page_dir){
    lcr3((uint64)(page_dir));
}

void arch_init_vmm(){
    kernel_pg_map = kalloc(PAGE_SIZE);
    kernel_pg_map->top_level = (uint64*)rcr3();
    kernel_pg_map->vm_region_head = kalloc(PAGE_SIZE);
    kernel_pg_map->vm_region_head->next = kernel_pg_map->vm_region_head;
    kernel_pg_map->vm_region_head->prev = kernel_pg_map->vm_region_head;

    uint64* boot_table = (uint64*)rcr3();
    serial_printf("Boot table located at %x.64\n", boot_table);

    /*
     * Map symbols in the linker script
     */

    if (map_pages(kernel_pg_map->top_level, (uint64)(text_start - kernel_min) + kernel_phys_min, (uint64*)text_start, 0,
                  text_end - text_start) == -1){
        panic("Mapping text!");
    }
    serial_printf("Kernel Text Mapped\n");


    if (map_pages(kernel_pg_map->top_level, (uint64)(rodata_start - kernel_min) + kernel_phys_min,
                  (uint64*)rodata_start,
                  PTE_NX, rodata_end - rodata_start) == -1){
        panic("Mapping rodata!");
    }
    serial_printf("Kernel Rodata Mapped\n");


    if (map_pages(kernel_pg_map->top_level, (uint64)(data_start - kernel_min) + kernel_phys_min, (uint64*)data_start,
                  PTE_NX | PTE_RW, data_end - data_start) == -1){
        panic("Mapping data!");
    }
    serial_printf("Kernel Data Mapped\n");
    /*
     * Map the first 4gb to the higher va space for the kernel, for the hhdm mapping
     */
    if (map_pages(kernel_pg_map->top_level, 0, 0 + (uint64*)hhdm_offset, PTE_RW | PTE_NX, 0x100000000) == -1){
        panic("Mapping first 4gb!");
    }

    serial_printf("Kernel HHDM Mapped\n");

    struct limine_memmap_response* memmap = memmap_request.response;
    struct limine_memmap_entry** entries = memmap->entries;

    for (uint64 i = 0; i < memmap->entry_count; i++){
        uint64 base = PGROUNDDOWN(entries[i]->base);
        uint64 top = PGROUNDUP(entries[i]->base + entries[i]->length);
        if (top < 0x100000000){
            continue;
        }
        for (uint64 j = base; j < top; j += PAGE_SIZE){
            if (j < (uint64)0x100000000){
                continue;
            }
            if (map_pages(kernel_pg_map->top_level, j, (uint64*)j + hhdm_offset, PTE_NX | PTE_RW, PAGE_SIZE) == -1){
                panic("hhdm mapping");
            }
        }
    }
    serial_printf("Mapped limine memory map into kernel page table\n");
    serial_printf("Kernel page table built in table located at %x.64\n", kernel_pg_map->top_level);
    lcr3((uint64)kernel_pg_map->top_level);
    serial_printf("VMM mapped and initialized");
    panic("Done");
}


/*
 * Walk a page directory to find a physical address, or allocate all the way down if the alloc flag is set
 */

static pte_t* walkpgdir(p4d_t* pgdir, const void* va, int alloc){
    if (!pgdir){
        return 0;
    }

    p4d_t p4d_idx = P4DX(va);
    pud_t pud_idx = PUDX(va);
    pmd_t pmd_idx = PMDX(va);
    pte_t pte_idx = PTX(va);

    pud_t* pud = P2V(pgdir);
    pud += p4d_idx;

    if (!(*pud & PTE_P)){
        if (alloc){
            serial_printf("pud\n");
            *pud = (pud_t)phys_alloc(1);
            *pud |= PTE_P | PTE_RW | PTE_U;
        }
        else{
            return 0;
        }
    }

    pmd_t* pmd = P2V(*pud);
    pmd += pud_idx;

    if (!(*pmd & PTE_P)){
        if (alloc){
            serial_printf("pmd\n");
            *pmd = (pmd_t)phys_alloc(1);
            *pmd |= PTE_P | PTE_RW | PTE_U;
        }
        else{
            return 0;
        }
    }

    pte_t* pte = P2V(*pmd);
    pte += pmd_idx;


    if (!(*pte & PTE_P)){
        if (alloc){
            serial_printf("pte\n");
            *pte = (pte_t)phys_alloc(1);
            *pte |= PTE_P | PTE_RW | PTE_U;
        }
        else{
            return 0;
        }
    }

    pte_t* entry = P2V(*pte);
    entry += pte_idx;

    return entry;
}

/*
 * Maps pages from VA/PA to size in page size increments.
 */
int map_pages(p4d_t* pgdir, uint64 physaddr, uint64* va, uint64 perms, uint64 size){
    pte_t* pte;
    uint64 address = PGROUNDDOWN((uint64) va);
    uint64 last = PGROUNDUP(((uint64) va) + size - 1);

    for (;;){
        if ((pte = walkpgdir(pgdir, (void*)address, 1)) == 0){
            return -1;
        }

        if ((perms & PTE_U) && *pte & PTE_P){
            panic("remap");
        }

        *pte = physaddr | perms | PTE_P;

        if (address == last){
            break;
        }

        address += PAGE_SIZE;
        physaddr += PAGE_SIZE;
    }

    return 0;
}



uint64 dealloc_va(p4d_t* pgdir, uint64 address){
    uint64 aligned_address = PGROUNDDOWN(address);
    pte_t* entry = walkpgdir(pgdir, (void*)aligned_address, 0);

    if (!entry){
        return 0;
    }

    if (*entry & PTE_P){
        phys_dealloc((void *)PTE_ADDR(*entry),1);
        *entry = 0;
        native_flush_tlb_single(aligned_address);
        return 1;

    }
    return 0;
}

void dealloc_va_range(p4d_t* pgdir, uint64 address, uint64 size){
    uint64 aligned_size = PGROUNDUP(size);

    for (uint64 i = 0; i > aligned_size; i += PAGE_SIZE){
        dealloc_va(pgdir, address + i);
    }
}

struct vm_region* create_region(){
}
