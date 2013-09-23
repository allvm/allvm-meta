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

/**@file   PhysicalMemoryManager.cc
 * @author James Molloy <jamesm@osdev.org>
 * @date   Mon Jun  7 23:48:29 2010
 * @brief  Manages physical memory. */

#include <horizon/Baremetal/PhysicalMemoryManager.h>
#include <horizon/Baremetal/VirtualMemoryManager.h>
#include <horizon/Baremetal/DebugStream.h>
#include <horizon/Baremetal/assert.h>

/** Singleton instance. */
PhysicalMemoryManager PhysicalMemoryManager::m_instance;

PhysicalMemoryManager::PhysicalMemoryManager() {
}

void PhysicalMemoryManager::Initialise(kernel_args_t *args) {
	m_page_stack = (uint64_t*)PAGE_STACK_LOCATION;
	m_stack_size = 0;
	m_stack_free = 0;
	m_static_free = NUM_STATIC_PAGES;

	kernel_args_memory_t *range = (kernel_args_memory_t*)args->phys_ranges;
	for(uint64_t i = 0; i < args->phys_range_count; i++) {
		EarlyAssert(range);
		if(range->type == kernel_args_memory_t::PHYS_MEMORY_FREE) {
			for(uint64_t j = range->start; j < range->end; j += PAGE_SIZE_4K) {
				Free(j);
			}
		}
		range = (kernel_args_memory_t*)range->next;
	}
}

void PhysicalMemoryManager::Free(uint64_t x) {
	if(m_static_free > 0) {
		m_static_stack[--m_static_free] = x;
		return;
	}

	if(m_stack_free == 0) {
		VirtualMemoryManager::instance().Map((uint64_t) (m_page_stack+m_stack_size), x, PAGE_WRITE);
		m_stack_free += PAGE_SIZE_4K / sizeof(uint64_t);
	} else {
		m_page_stack[m_stack_size++] = x;
		m_stack_free --;
	}
}

uint64_t PhysicalMemoryManager::Allocate() {
	if(m_static_free < NUM_STATIC_PAGES) {
		return m_static_stack[m_static_free++];
	}

	EarlyAssert(m_stack_size > 0);

	m_stack_free ++;
	return m_page_stack[--m_stack_size];
}
