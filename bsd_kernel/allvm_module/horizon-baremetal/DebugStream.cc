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

/**@file   DebugStream.cc
 * @author James Molloy <jamesm@osdev.org>
 * @date   Mon Jun  7 21:11:25 2010
 * @brief  Provides the implementation of DebugStream, a std::ostream like stream
           that writes to the primary serial port. */

#include <horizon/Baremetal/DebugStream.h>
#include <horizon/Baremetal/cpu.h>

/** Global DebugStream object. */
DebugStream dout;

void DebugStream::Initialise() {
#if 0
	Out8(SERIAL_BASE+SERIAL_INTEN, 0x00); // Disable all interrupts.
	Out8(SERIAL_BASE+SERIAL_LCTRL, 0x80); // Enable DLAB (set baud rate divisor)
	Out8(SERIAL_BASE+SERIAL_RXTX,  0x03); // Set divisor to 3 (lo byte) 38400 baud
	Out8(SERIAL_BASE+SERIAL_INTEN, 0x00); //                  (hi byte)
	Out8(SERIAL_BASE+SERIAL_LCTRL, 0x03); // 8 bits, no parity, one stop bit
	Out8(SERIAL_BASE+SERIAL_IIFIFO,0xC7); // Enable FIFO, clear them, with 14-byte threshold
	Out8(SERIAL_BASE+SERIAL_MCTRL, 0x0B); // IRQs enabled, RTS/DSR set
	Out8(SERIAL_BASE+SERIAL_INTEN, 0x0C); // enable all interrupts.
#endif
}

DebugStream &DebugStream::operator << (char c) {
	while(! (In8(SERIAL_BASE+SERIAL_LSTAT) & 0x20)) {
		asm volatile("nop");
	}
	
	if(c == '\n') {
		*this << '\r';
	}

	Out8(SERIAL_BASE+SERIAL_RXTX, (uint8_t)c);
	return *this;
}

DebugStream &DebugStream::operator << (const char *c) {
	while(*c) {
		*this << *c++;
	}
	return *this;
}

DebugStream &DebugStream::operator << (unsigned long l) {
	char c[16];

	*this << "0x";
	if(l == 0) {
		return *this << '0';
	}

	int i = 0;
	while(l) {
		char x = l & 0xF;
		if(x <= 9) {
			c[i++] = (char)('0' + x);
		} else {
			c[i++] = (char)('a' + x - 10);
		}
		l >>= 4;
	}

	for(i--; i >= 0; i--) {
		*this << c[i];
	}

	return *this;
}
