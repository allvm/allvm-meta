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

/**@file   cpu.cc
 * @author James Molloy <jamesm@osdev.org>
 * @date   Mon Jun  7 21:13:08 2010
 * @brief  Defines low-level routines for CPU-specific functions. */

#include <horizon/Baremetal/cpu.h>
#include <horizon/Baremetal/DebugStream.h>

void Out8(uint16_t port, uint8_t value) {
	asm volatile("outb %1, %0" : : "dN" (port), "a" (value));
}

uint8_t In8(uint16_t port) {
	uint8_t ret;
	asm volatile("inb %1, %0" : "=a" (ret) : "dN" (port));
	return ret;
}

uint16_t In16(uint16_t port) {
	uint16_t ret;
	asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
	return ret;
}

uint64_t ReadCR0() {
	uint64_t ret;
	asm volatile ("mov %%cr0, %0" : "=r" (ret));
	return ret;
}

void WriteCR0(uint64_t x) {
	asm volatile ("mov %0, %%cr0" : : "r" (x));
}

uint64_t ReadCR3() {
	uint64_t ret;
	asm volatile ("mov %%cr3, %0" : "=r" (ret));
	return ret;
}

uint64_t ReadCR4() {
	uint64_t ret;
	asm volatile ("mov %%cr4, %0" : "=r" (ret));
	return ret;
}

void WriteCR4(uint64_t x) {
	asm volatile ("mov %0, %%cr4" : : "r" (x));
}

void WriteFPUCW(uint16_t cw) {
	asm volatile ("fldcw %0" : : "m" (cw));
}

void InitialiseFPU() {
	/* We're on x86_64 - we can assume SSE3 + FPU. */

	/* Set the OSFXSR bit. */
	WriteCR4(ReadCR4() | 0x200);

	WriteCR0( (ReadCR0() | CR0_NE | CR0_MP) & ~(CR0_EM | CR0_TS) );

	/* Initialise the FPU. */
	asm volatile("finit");

	WriteFPUCW(0x37F);
}

void WriteMSR(uint32_t msr, uint64_t val) {
	uint32_t hi = val & 0xFFFFFFFF;
	uint32_t lo = (val >> 32) & 0xFFFFFFFF;
	asm volatile("wrmsr" : : "c"(msr), "a"(hi), "d"(lo));
}

uint64_t ReadMSR(uint32_t msr) {
	uint32_t hi, lo;
	asm volatile("rdmsr" : "=a"(hi), "=d"(lo) : "c"(msr));
	return ( ((uint64_t)hi) << 32 ) | (uint64_t)lo;
}

void Reboot() {
	/* Short code taken from http://wiki.osdev.org/Reboot */
	/* Just performs an 8042 reset. */
	unsigned char good = 0x02;
	while ((good & 0x02) != 0) {
		good = In8(0x64);
	}
	Out8(0x64, 0xFE);

	/* If that didn't work, perform a triple fault. */
	asm volatile("xor %eax,%eax; lidt (%eax); int $100;");
}
