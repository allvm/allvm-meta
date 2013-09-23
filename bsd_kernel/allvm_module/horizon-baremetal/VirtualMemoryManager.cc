/* Horizon bytecode compiler
 * Copyright (C) 2010 James Molloy
 *
 * Horizon is open source software, released under the terms of the Non-Profit
 * Open Software License 3.0. You should have received a copy of the
 * licensing information along with the source code distribution. If you
 * have not received a copy of the license, please refer to the Horizon
 * project website.
 *
 * Please note that if you modify this file, the license requires you to
 * ADD your name to the list of contributors. This boilerplate is not the
 * license itself; please refer to the copy of the license you have received
 * for complete terms.
 */

/**@file   VirtualMemoryManager.cc
 * @author James Molloy <jamesm@osdev.org>
 * @date   Tue Jun  8 10:50:53 2010
 * @brief  Manages mapping of virtual to physical addresses.
 * @note   This file relies on the bootloader identity mapping the first GB of
           memory. Kiwi marks this memory as "reclaimable", but we never reclaim
           it - instead harvesting the current paging structures and using them as
           our own. We can do this as we only need one address space (SAS OS). */

#include <horizon/Baremetal/VirtualMemoryManager.h>
#include <horizon/Baremetal/PhysicalMemoryManager.h>
#include <horizon/Baremetal/DebugStream.h>
#include <horizon/Baremetal/cpu.h>
#include <horizon/Baremetal/assert.h>

#include <string.h>

/** Singleton instance. */
VirtualMemoryManager VirtualMemoryManager::m_instance;

/** Has the singleton been initialised? cannot be a member of the singleton as
    global constructors aren't run by the time this is needed. */
static bool initialised = false;

/** Index into a table. */
#define TABLE_IDX(t,i) ( ((uint64_t*)(t))[i] )

/** Get the PML4 index of a virtual address. */
#define PML4_IDX(v) ( (v>>39) & 0x1FF )
/** Get the PDPT index of a virtual address. */
#define PDPT_IDX(v) ( (v>>30) & 0x1FF )
/** Get the PDE index of a virtual address. */
#define PDE_IDX(v)  ( (v>>21) & 0x1FF )
/** Get the PTE index of a virtual address. */
#define PTE_IDX(v)  ( (v>>12) & 0x1FF )

VirtualMemoryManager::VirtualMemoryManager() {
}

void VirtualMemoryManager::Initialise() {
	/* Get the current PML4 location. */
	m_pml4 = ReadCR3();

	initialised = true;
}

void VirtualMemoryManager::Map(uint64_t v, uint64_t p, uint64_t flags) {
	if(!initialised) {
		Initialise();
	}

	EarlyAssert( (v&0xFFF) == 0 );

        /* For sanity */
	//bool large_page = flags & PAGE_2M;
	flags = (flags & (PAGE_WRITE|PAGE_USER)) | PAGE_PRESENT;

	/* Index into the PML4; see if an entry exists. */
	uint64_t pml4_entry = TABLE_IDX(m_pml4&PAGE_MASK, PML4_IDX(v));
	if( (pml4_entry & PAGE_PRESENT) == 0 ) {
		pml4_entry = NewTableEntry(m_pml4&PAGE_MASK, PML4_IDX(v));
	}

	EarlyAssert( (pml4_entry & PAGE_PRESENT) == PAGE_PRESENT );

	/* Index into the PDPT; see if an entry exists. */
	uint64_t pdpt_entry = TABLE_IDX(pml4_entry&PAGE_MASK, PDPT_IDX(v));
	if( (pdpt_entry & PAGE_PRESENT) == 0 ) {
		pdpt_entry = NewTableEntry(pml4_entry&PAGE_MASK, PDPT_IDX(v));
	}

	EarlyAssert( (pdpt_entry & PAGE_PRESENT) == PAGE_PRESENT );

	/* Index into the page directory; see if an entry exists. */
	uint64_t pde_entry = TABLE_IDX(pdpt_entry&PAGE_MASK, PDE_IDX(v));
	if( (pde_entry & PAGE_PRESENT) == 0 ) {
		pde_entry = NewTableEntry(pdpt_entry&PAGE_MASK, PDE_IDX(v));
	}

	EarlyAssert( (pde_entry & PAGE_PRESENT) == PAGE_PRESENT );

	/** @todo 2MB pages here. */

	/* Index into the page table; see if an entry exists. */
	uint64_t pte_entry = TABLE_IDX(pde_entry&PAGE_MASK, PTE_IDX(v));
	/** @todo TMP TMP, while we havne't implemented Unmap yet! */
	if((pte_entry & PAGE_PRESENT) != 0) {
		return;
	}
	EarlyAssert((pte_entry & PAGE_PRESENT) == 0 );

	TABLE_IDX(pde_entry&PAGE_MASK, PTE_IDX(v)) = (p & PAGE_MASK) | flags;

	pdpt_entry = TABLE_IDX(pml4_entry&PAGE_MASK, PDPT_IDX(v));
	EarlyAssert( (pdpt_entry & PAGE_PRESENT) == PAGE_PRESENT );
}

void VirtualMemoryManager::Unmap(uint64_t v) {
	/** @todo Implement */
}

void VirtualMemoryManager::UnmapAndFree(uint64_t v) {
	/** @todo Implement */
}

uint64_t VirtualMemoryManager::NewTableEntry(uint64_t t, uint64_t e) {
	uint64_t newpage = PhysicalMemoryManager::instance().Allocate();
	memset((void*)newpage, 0, PAGE_SIZE_4K);
	
	TABLE_IDX(t,e) = newpage | PAGE_WRITE | PAGE_PRESENT;
	return newpage | PAGE_WRITE | PAGE_PRESENT;
}
