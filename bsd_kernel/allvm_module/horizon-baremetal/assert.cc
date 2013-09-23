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

/**@file   assert.cc
 * @author James Molloy <jamesm@osdev.org>
 * @date   Tue Jun  8 11:41:47 2010
 * @brief  Defines the "EarlyAssert" function, which panics on early boot. */

#include <horizon/Baremetal/assert.h>
#include <horizon/Baremetal/DebugStream.h>

#include <string.h>

void _EarlyAssert(const char *cond, int line, const char *file) {
	/* Nasty hacks to get a stringified decimal value when sprintf not available :( */
	char c[32];
	memset(c, 0, 32);
	int i = 0;
	if(line == 0) {
		c[i++] = '0';
	} else {
		while(line > 0) {
			c[i++] = '0' + (line % 10);
			line /= 10;
		}
	}
	dout << "horizon: PANIC: Assertion `" << cond << "' failed at " << file << ":";

	while(i > 0) {
		dout << c[--i];
	}
	dout << "\n";

	for(;;) asm volatile("hlt");
}

void Panic(const char *msg) {
	dout << "horizon: PANIC: " << msg << "\n";
	for(;;) asm volatile("hlt");
}
