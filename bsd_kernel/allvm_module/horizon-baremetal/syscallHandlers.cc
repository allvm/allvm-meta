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

#include <horizon/Baremetal/syscallHandlers.h>
#include <horizon/Baremetal/cpu.h>
#include <horizon/Baremetal/VirtualMemoryManager.h>
#include <horizon/Baremetal/PhysicalMemoryManager.h>
#include <horizon/Baremetal/DebugStream.h>
#include <horizon/Baremetal/assert.h>
#include <horizon/Baremetal/SpecialFile.h>
#include <SyscallList.h>

#include <sys/utsname.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/uio.h>
#include <asm/prctl.h>
#include <sys/prctl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <errno.h>

SyscallHandler g_syscall_handlers[NumLinuxSyscalls];

#define NUM_FDTAB 64
static SpecialFile *s_fdtab[NUM_FDTAB];
static unsigned int s_fdtab_pos[NUM_FDTAB];

#define BRK_START 0x40000000
static uint64_t s_brk = BRK_START;

/* How large is too large to brk()? */
#define STUPID_BRK 0x10000000

#define MMAP_START 0xffffffff40000000UL
static uint64_t s_mmap = MMAP_START;

static void SyscallUname(InterruptState &s) {
	LOAD_P1(name, struct utsname *);

	dout << "horizon: uname()\n";

	strcpy(name->sysname, "Horizon");
	strcpy(name->nodename, "hzn-1");
	strcpy(name->release, "2.6.32-24-generic");
	strcpy(name->version, "1");
	strcpy(name->machine, "x86_64");

	SAVE_R(0);
	dout << "horizon: \t-> 0\n";
}

static void SyscallOpen(InterruptState &s) {
	LOAD_P1(path, const char *);
	LOAD_P2(oflag, int);

	dout << "horizon: open(" << path << ") - oflag " << (unsigned long)oflag << "\n";
	/* Find a free FD. */
	int fd = -1;
	for(int i = 0; i < NUM_FDTAB; i++) {
		if(s_fdtab[i] == 0) {
			fd = i;
			break;
		}
	}
	if(fd == -1) {
		Panic("Out of file descriptors!");
	}

	SpecialFile *file = SpecialFile::get(path);
	if(file) {	
		s_fdtab[fd] = file;
		s_fdtab_pos[fd] = 0;
	} else {
		fd = -ENOENT;
	}

	SAVE_R(fd);
	dout << "horizon: \t-> " << (unsigned long)fd << "\n";
}

static void SyscallRead(InterruptState &s) {
	LOAD_P1(fd, int);
	LOAD_P2(buf, void*);
	LOAD_P3(nbyte, int);

	dout << "horizon: read(" << (unsigned long)fd << ", ..., " << (unsigned long)nbyte << ")\n";

	EarlyAssert(fd < NUM_FDTAB);
	EarlyAssert(s_fdtab[fd]);
	int n = s_fdtab[fd]->Read((char *)buf, s_fdtab_pos[fd], nbyte);
	s_fdtab_pos[fd] += n;

	SAVE_R(n);
	dout << "horizon: \t-> " << (unsigned long)n << "\n";
}

static void SyscallClose(InterruptState &s) {
	LOAD_P1(fd, int);

	dout << "horizon: close(" << (unsigned long)fd << ")\n";

	EarlyAssert(fd < NUM_FDTAB);
	EarlyAssert(s_fdtab[fd]);

	s_fdtab[fd] = 0;

	SAVE_R(0);
	dout << "horizon: \t-> 0\n";
}

static void SyscallWrite(InterruptState &s) {
	LOAD_P1(fd, int);
	LOAD_P2(buf, void*);
	LOAD_P3(nbyte, int);

	/* Make the output a little easier to read - if it's printing to stdout or stderr, don't bother with the debug text. */
	if(fd != 1 && fd != 2) {
		dout << "horizon: write(" << (unsigned long)fd << ", ..., " << (unsigned long)nbyte << ")\n";
	}

 	EarlyAssert(fd < NUM_FDTAB);
	EarlyAssert(s_fdtab[fd]);
	int n = s_fdtab[fd]->Write((const char *)buf, s_fdtab_pos[fd], nbyte);
	s_fdtab_pos[fd] += n;

	SAVE_R(n);
	if(fd != 1 && fd != 2) {
		dout << "horizon: \t-> " << (unsigned long)n << "\n";
	}
}

static void SyscallWritev(InterruptState &s) {
	LOAD_P1(fildes, int);
	LOAD_P2(iov, const struct iovec *);
	LOAD_P3(iovcnt, int);

	EarlyAssert(iovcnt > 0);

	dout << "horizon: writev(" << (unsigned long)fildes << ", ..., " << (unsigned long)iovcnt << ")\n";

 	EarlyAssert(fildes < NUM_FDTAB);
	EarlyAssert(s_fdtab[fildes]);

	int n = 0;
	for(int i = 0; i < iovcnt; i++) {
		int m = s_fdtab[fildes]->Write((const char *)iov[i].iov_base, s_fdtab_pos[fildes], iov[i].iov_len);
		if(m != (int)iov[i].iov_len) {
			break;
		}
		n += m;
	}

	SAVE_R(n);
	dout << "horizon: \t-> " << (unsigned long)n << "\n";
}

static void SyscallBrk(InterruptState &s) {
	LOAD_P1(addr, uint64_t);

	dout << "horizon: brk(" << addr << ")\n";

	if(addr < 0x400000) {
		SAVE_R(s_brk);
		dout << "horizon: \t-> " << s_brk << "\n";
		return;
	} else if(addr > s_brk && addr - s_brk > STUPID_BRK) {
		/* Far too large amount requested. */
		SAVE_R(s_brk);
		dout << "horizon: \t-> " << s_brk << " (Error: too large amount requested)\n";
		return;
	}

	/* Page-align the address. */
	if( addr & 0xFFFUL ) {
		addr = (addr & ~0xFFFUL) + 0x1000;
	}

	if(addr < s_brk) {
		while(addr < s_brk) {
			s_brk -= 0x1000;
			VirtualMemoryManager::instance().UnmapAndFree(s_brk);
		}
	} else {
		while(addr > s_brk) {
			VirtualMemoryManager::instance().Map(s_brk,
							     PhysicalMemoryManager::instance().Allocate(),
							     PAGE_PRESENT|PAGE_WRITE);
			/** @todo Improve this. We can't use memset yet. */
			for(int i = 0; i < 0x1000/8; i++) {
				((uint64_t*)s_brk)[i] = 0UL;
			}
			s_brk += 0x1000;
		}
	}
	SAVE_R(s_brk);

	dout << "horizon: \t-> " << s_brk << "\n";
}

static void SyscallArchPrctl(InterruptState &s) {
	LOAD_P1(code, int);
	LOAD_P2(addr, unsigned long);

	uint64_t r = 0;

	dout << "horizon: prctl(";
	switch(code) {
	case ARCH_SET_FS:
		dout << "ARCH_SET_FS";
		WriteMSR(MSR_FS_BASE, addr);
		break;
	case ARCH_GET_FS:
		dout << "ARCH_GET_FS";
		r = ReadMSR(MSR_FS_BASE);
		break;
	case ARCH_SET_GS:
		dout << "ARCH_SET_GS";
		WriteMSR(MSR_GS_BASE, addr);
		break;
	case ARCH_GET_GS:
		dout << "ARCH_GET_GS";
		r = ReadMSR(MSR_GS_BASE);
		break;
	}
	dout << "), " << addr << "\n";

	dout << "horizon: \t-> " << r << "\n";
	SAVE_R(r);
}

static void SyscallGetEuid(InterruptState &s) {
	dout << "horizon: geteuid()\n";
	dout << "horizon: \t-> 0\n";
	SAVE_R(0);
}
static void SyscallGetEgid(InterruptState &s) {
	dout << "horizon: getegid()\n";
	dout << "horizon: \t-> 0\n";
	SAVE_R(0);
}
static void SyscallGetuid(InterruptState &s) {
	dout << "horizon: getuid()\n";
	dout << "horizon: \t-> 0\n";
	SAVE_R(0);
}
static void SyscallGetgid(InterruptState &s) {
	dout << "horizon: getgid()\n";
	dout << "horizon: \t-> 0\n";
	SAVE_R(0);
}

static void SyscallRtSigprocmask(InterruptState &s) {
	dout << "horizon: rt_sigprocmask()\n";
	dout << "horizon: \t-> 0\n";
	SAVE_R(0);
}

static void SyscallGettid(InterruptState &s) {
	dout << "horizon: gettid()\n";
	dout << "horizon: \t-> 0\n";
	SAVE_R(0);
}

static void SyscallTgkill(InterruptState &s) {
	dout << "horizon: tgkill()\n";
	dout << "horizon: \t-> 0\n";
	dout << "horizon: *** Exiting due to tgkill()\n";

	Reboot();

	SAVE_R(0);
}

static void SyscallRtSigaction(InterruptState &s) {
	dout << "horizon: rt_sigaction()\n";
	dout << "horizon: \t-> 0\n";
	SAVE_R(0);
}

static void SyscallMmap(InterruptState &s) {
	LOAD_P1(addr, void *);
	LOAD_P2(length, size_t);
	LOAD_P3(prot, int);
	LOAD_P4(flags, int);
	LOAD_P5(fd, int);
	LOAD_P6(offset, off_t);

	dout << "horizon: mmap(" << (unsigned long)addr << ", " << (unsigned long)length << ", ";
	
	bool b = false;
	if(prot & PROT_EXEC) {
		dout << "PROT_EXEC";
		b = true;
	}
	if(prot & PROT_READ) {
		if(b) {dout << "|";}
		dout << "PROT_READ";
		b = true;
	}
	if(prot & PROT_WRITE) {
		if(b) {dout << "|";}
		dout << "PROT_WRITE";
		b = true;
	}
	if(prot == PROT_NONE) {
		dout << "PROT_NONE";
	}

	dout << ", ";
	if(flags & MAP_SHARED) {
		dout << "MAP_SHARED";
	} else {
		dout << "MAP_PRIVATE";
	}
	if(flags & MAP_ANONYMOUS) {
		dout << "|MAP_ANONYMOUS";
	}
	dout << ", " << (unsigned long)fd << ", " << (unsigned long)offset << ")\n";

	if((length & 0xFFF) != 0) {
		dout << "horizon: length & 0xFFF != 0\n";
		dout << "horizon: \t -> MAP_FAILED\n";
		SAVE_R(MAP_FAILED);
		return;
	}
	EarlyAssert((length & 0xFFF) == 0);

	/* We only know how to deal with anonymous mappings at the moment. */
	if(flags & MAP_ANONYMOUS) {
		uint64_t start = s_mmap;
		uint64_t end = start + length;
		while(end > s_mmap) {
			VirtualMemoryManager::instance().Map(s_mmap,
							     PhysicalMemoryManager::instance().Allocate(),
							     PAGE_PRESENT|PAGE_WRITE);
			/** @todo Improve this. We can't use memset yet. */
			for(int i = 0; i < 0x1000/8; i++) {
				((uint64_t*)s_mmap)[i] = 0UL;
			}
			s_mmap += 0x1000;
		}
		
		dout << "horizon: \t-> " << start << "\n";
		SAVE_R(start);
	} else {
		dout << "horizon: Can't deal with non-anonymous mmaps.";
		dout << "horizon: \t -> MAP_FAILED\n";
		SAVE_R(MAP_FAILED);
	}
}

static void SyscallMunmap(InterruptState &s) {
	LOAD_P1(addr, void*);
	LOAD_P2(length, size_t);

	dout << "horizon: munmap(" << (unsigned long)addr << ", " << length << ")\n";
	dout << "horizon: \t -> 0\n";

	SAVE_R(0);
}

static void SyscallFstat(InterruptState &s) {
	LOAD_P1(fd, int);
	LOAD_P2(buf, struct stat *);

	dout << "horizon: fstat(" << (unsigned long)fd << ", ...)\n";

	EarlyAssert(fd < NUM_FDTAB);
	EarlyAssert(s_fdtab[fd]);

	buf->st_dev = 0;
	buf->st_ino = (ino_t)s_fdtab[fd];
	buf->st_mode = 0;
	buf->st_nlink = 1;
	buf->st_uid = 0;
	buf->st_gid = 0;
	buf->st_rdev = 0;
	buf->st_size = 1086; /** @todo only gconv requires this atm, so it's hard coded. Change. */
	buf->st_blksize = 1024;
	buf->st_blocks = 0;
	buf->st_atime = 0;
	buf->st_mtime = 0;
	buf->st_ctime = 0;

	SAVE_R(0);
}

void InitialiseSyscallHandlers() {
	for(unsigned int i = 0; i < NUM_FDTAB; i++) {
		s_fdtab[i] = 0;
		s_fdtab_pos[i] = 0;
	}
	for(unsigned int i = 0; i < NumLinuxSyscalls; i++) {
		g_syscall_handlers[i] = 0;
	}

	/* Open stdin, stdout and stderr. */
	s_fdtab[0] = SpecialFile::get("/dev/tty");
	s_fdtab[1] = SpecialFile::get("/dev/tty");
	s_fdtab[2] = SpecialFile::get("/dev/tty");
	
	g_syscall_handlers[HZN_SC_uname] = &SyscallUname;
	g_syscall_handlers[HZN_SC_open] = &SyscallOpen;
	g_syscall_handlers[HZN_SC_read] = &SyscallRead;
	g_syscall_handlers[HZN_SC_close] = &SyscallClose;
	g_syscall_handlers[HZN_SC_write] = &SyscallWrite;
	g_syscall_handlers[HZN_SC_writev] = &SyscallWritev;
	g_syscall_handlers[HZN_SC_brk] = &SyscallBrk;
	g_syscall_handlers[HZN_SC_arch_prctl] = &SyscallArchPrctl;
	g_syscall_handlers[HZN_SC_geteuid] = &SyscallGetEuid;
	g_syscall_handlers[HZN_SC_getegid] = &SyscallGetEgid;
	g_syscall_handlers[HZN_SC_getuid] = &SyscallGetuid;
	g_syscall_handlers[HZN_SC_getgid] = &SyscallGetgid;
	g_syscall_handlers[HZN_SC_rt_sigprocmask] = &SyscallRtSigprocmask;
	g_syscall_handlers[HZN_SC_gettid] = &SyscallGettid;
	g_syscall_handlers[HZN_SC_tgkill] = &SyscallTgkill;
	g_syscall_handlers[HZN_SC_rt_sigaction] = &SyscallRtSigaction;
	g_syscall_handlers[HZN_SC_mmap] = &SyscallMmap;
	g_syscall_handlers[HZN_SC_fstat] = &SyscallFstat;
	g_syscall_handlers[HZN_SC_munmap] = &SyscallMunmap;
}
