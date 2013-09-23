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

/**@file   dynamicLookup.cc
 * @author James Molloy <jamesm@osdev.org>
 * @date   Tue Aug 24 10:14:32 2010
 * @brief  Defines functions that would normally be in libdl.so, for looking up the address
           of functions dynamically. */

#include <horizon/Baremetal/cpu.h>
#include <horizon/Baremetal/DebugStream.h>
#include <horizon/Baremetal/assert.h>

#include <map>
#include <string>
#include <RuntimeAddressList.h>

#include <dlfcn.h>

#define MAIN_PROGRAM_HANDLE (void*)0x10

extern "C" void *dlopen(const char *file, int mode) {
	if(file) {
		dout << "horizon: dlopen(" << file << ")\n";
		EarlyAssert(0);
		return 0;
	} else {
		dout << "horizon: dlopen(NULL)\n";
		dout << "horizon: \t-> " << (unsigned long)MAIN_PROGRAM_HANDLE << "\n";
		return MAIN_PROGRAM_HANDLE;
	}
}

extern "C" char *dlerror() {
	return (char*)"Undefined Error <horizon>";
}

extern "C" void *dlsym(void * handle, const char * name) {
	EarlyAssert(handle == MAIN_PROGRAM_HANDLE);
	dout << "horizon: dlsym(" << name << ")\n";

	void *x = s_runtime_addresses[name];
	dout << "horizon: \t-> " << (unsigned long)x << "\n";
	return x;
}

extern "C" int dladdr(const void *addr, Dl_info *info) {
	dout << "horizon: dladdr(" << (unsigned long)addr << ")\n";
	EarlyAssert(0);
	return 0;
}
