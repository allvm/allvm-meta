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

/**@file   InterruptManager.cc
 * @author James Molloy <jamesm@osdev.org>
 * @date   Tue Jun  8 13:20:49 2010
 * @brief  Manages the assignment and dispatch of interrupts.
 * @note   Some of this code is taken from Pedigree (http://www.pedigree-project.org), with this code written by James Molloy and Jörg Pfahler. */

/* Must include before syscallHandlers.h */
#include <SyscallList.h>

#include <horizon/Baremetal/InterruptManager.h>
#include <horizon/Baremetal/assert.h>
#include <horizon/Baremetal/DebugStream.h>
#include <horizon/Baremetal/cpu.h>
#include <horizon/Baremetal/syscallHandlers.h>

/** Singleton instance. */
InterruptManager InterruptManager::m_instance;

/** Dumps register contents to dbgout. */
void reg_dump(InterruptState &state);

/** Default page fault exception handler. */
void page_fault_handler(InterruptState &state);

/** Default syscall handler. */
void syscall_handler(InterruptState &state);

/** Handles the exit() syscall, INT 0xFE. */
void exit_handler(InterruptState &state);

static const char* exception_names[] =
{
	"Divide Error",
	"Debug",
	"NMI Interrupt",
	"Breakpoint",
	"Overflow",
	"BOUND Range Exceeded",
	"Invalid Opcode",
	"Device Not Available",
	"Double Fault",
	"Coprocessor Segment Overrun", /* recent IA-32 processors don't generate this */
	"Invalid TSS",
	"Segment Not Present",
	"Stack Fault",
	"General Protection Fault",
	"Page Fault",
	"FPU Floating-Point Error",
	"Alignment Check",
	"Machine-Check",
	"SIMD Floating-Point Exception",
	"Reserved: Interrupt 20",
	"Reserved: Interrupt 21",
	"Reserved: Interrupt 22",
	"Reserved: Interrupt 23",
	"Reserved: Interrupt 24",
	"Reserved: Interrupt 25",
	"Reserved: Interrupt 26",
	"Reserved: Interrupt 27",
	"Reserved: Interrupt 28",
	"Reserved: Interrupt 29",
	"Reserved: Interrupt 30",
	"Reserved: Interrupt 31"
};

InterruptManager::InterruptManager() {
}

void InterruptManager::Initialise() {
	/* Zero all handlers. */
	for(int i = 0; i < 256; i++) {
		m_handlers[i] = 0;
	}

	/* Load in the IDT already created by the bootloader. */
	struct {uint16_t size; uint64_t idt;} __attribute__((packed)) idtr = {4096, (uint64_t)&m_idt};

	extern uintptr_t interrupt_handler_array[];
	for(int i = 0; i < 256; i++) {
		SetInterruptGate(i, interrupt_handler_array[i]);
	}

	/* Save the IDT again to ensure that the changes get re-cached by
	   the processor */
	asm volatile("lidt %0" : : "m" (idtr));

	RegisterHandler(14,  &page_fault_handler);
	RegisterHandler(255, &syscall_handler);
	RegisterHandler(254, &exit_handler);
}

void InterruptManager::RegisterHandler(unsigned int int_no, InterruptHandler handler) {
	EarlyAssert(int_no < 256);

	m_handlers[int_no] = handler;
}

void InterruptManager::Interrupt(InterruptState &state) {
	InterruptHandler handler = instance().m_handlers[state.int_no];

	if(handler) {
		handler(state);
	} else {
		dout << "horizon: PANIC: unhandled exception: " << state.int_no;
		if(state.int_no < 32) {
			dout << "(" << exception_names[state.int_no] << ")";
		}
		dout << " @ " << state.rip << "\n";
		reg_dump(state);
		for(;;) asm volatile("hlt");
	}
}

void InterruptManager::SetInterruptGate(unsigned int int_no, uint64_t handler) {
	m_idt[int_no].offset0  = handler & 0xFFFF;
	m_idt[int_no].selector = 0x20;
	m_idt[int_no].ist      = 0;
	m_idt[int_no].flags    = 0xEE /*0x8E*/;
	m_idt[int_no].offset1  = (handler >> 16) & 0xFFFF;
	m_idt[int_no].offset2  = (handler >> 32) & 0xFFFFFFFF;
	m_idt[int_no].res      = 0;
}

void reg_dump(InterruptState &state) {
	dout << "rax: " << state.rax << "\t\trbx: " << state.rbx << "\n";
	dout << "rcx: " << state.rcx << "\t\trdx: " << state.rdx << "\n";
	dout << "rsi: " << state.rsi << "\t\trdi: " << state.rdi << "\n";
	dout << "r8 : " << state.r8 << "\t\tr9 : " << state.r9 << "\n";
	dout << "r10: " << state.r10 << "\t\tr11: " << state.r11 << "\n";
	dout << "r12: " << state.r12 << "\t\tr13: " << state.r13 << "\n";
	dout << "r14: " << state.r14 << "\t\tr15: " << state.r15 << "\n";
	dout << "rbp: " << state.rbp << "\t\trsp: " << state.rsp << "\n";
	dout << "cr0: " << ReadCR0() << "\t\tcr3: " << ReadCR3() << "\n";
	dout << "cr4: " << ReadCR4() << "\n";
}

void page_fault_handler(InterruptState &state) {
	uint64_t cr2;
	asm volatile("mov %%cr2, %0" : "=r" (cr2));

	dout << "horizon: PANIC: page fault @ " << state.rip << " - faulting address: " << cr2 << "\n";
	reg_dump(state);
	for(;;) asm volatile("hlt");
}

void syscall_handler(InterruptState &state) {
	SyscallHandler handler = 0;
	if(state.rax < NumLinuxSyscalls) {
		handler = g_syscall_handlers[state.rax];
	}
	if(handler) {
		handler(state);
		return;
	}

	dout << "horizon: PANIC: syscall " << state.rax << " (";
	if(state.rax >= NumLinuxSyscalls) {
		dout << "<invalid>";
	} else {
		dout << LinuxSyscalls[state.rax];
	}
	dout << ") @ " << state.rip << "\n";
	for(;;) asm volatile("hlt");
}

void exit_handler(InterruptState &state) {
	dout << "horizon: *** kernel exited. ***\n";
	Reboot();
}
