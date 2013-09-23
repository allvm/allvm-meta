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

#include <horizon/Baremetal/SpecialFile.h>
#include <horizon/Baremetal/DebugStream.h>
#include <horizon/Baremetal/assert.h>
#include <string.h>

const unsigned int NumSpecialFiles = 5;
SpecialFile *s_files[NumSpecialFiles];

/** .hbc module base address */
static uint64_t s_module_base;
/** .hbc module length */
static uint64_t s_module_size;

static const char *s_gconv_module = "\x24\x03\x01\x20\x10\x00\xce\x02\x47\x00\xea\x03\x3e\x04\x00\x00\x00\x4f\x53\x46\x30\x30\x30\x31\x30\x31\x30\x30\x2f\x2f\x00\x4f\x53\x46\x30\x30\x30\x31\x30\x30\x32\x30\x2f\x2f\x00\x4f\x53\x46\x30\x35\x30\x31\x30\x30\x30\x31\x2f\x2f\x00\x4f\x53\x46\x30\x30\x30\x31\x30\x31\x30\x31\x2f\x2f\x00\x49\x53\x4f\x5f\x36\x34\x36\x2e\x49\x52\x56\x3a\x31\x39\x39\x31\x2f\x2f\x00\x55\x43\x53\x2d\x32\x2f\x2f\x00\x4f\x53\x46\x30\x30\x30\x31\x30\x31\x30\x32\x2f\x2f\x00\x55\x43\x53\x32\x2f\x2f\x00\x49\x53\x4f\x2d\x49\x52\x2d\x31\x39\x33\x2f\x2f\x00\x31\x30\x36\x34\x36\x2d\x31\x3a\x31\x39\x39\x33\x2f\x2f\x00\x55\x43\x53\x2d\x34\x2f\x2f\x00\x41\x4e\x53\x49\x5f\x58\x33\x2e\x34\x2f\x2f\x00\x4f\x53\x46\x30\x30\x30\x31\x30\x31\x30\x34\x2f\x2f\x00\x43\x53\x55\x43\x53\x34\x2f\x2f\x00\x4f\x53\x46\x30\x30\x30\x31\x30\x31\x30\x35\x2f\x2f\x00\x49\x53\x4f\x2d\x49\x52\x2d\x36\x2f\x2f\x00\x4f\x53\x46\x30\x30\x30\x31\x30\x31\x30\x36\x2f\x2f\x00\x49\x53\x4f\x2d\x31\x30\x36\x34\x36\x2f\x2f\x00\x41\x4e\x53\x49\x5f\x58\x33\x2e\x34\x2d\x31\x39\x38\x36\x2f\x2f\x00\x49\x42\x4d\x33\x36\x37\x2f\x2f\x00\x43\x50\x33\x36\x37\x2f\x2f\x00\x55\x54\x46\x2d\x38\x2f\x2f\x00\x41\x4e\x53\x49\x5f\x58\x33\x2e\x34\x2d\x31\x39\x36\x38\x2f\x2f\x00\x55\x54\x46\x38\x2f\x2f\x00\x55\x43\x53\x2d\x32\x42\x45\x2f\x2f\x00\x55\x43\x53\x2d\x34\x42\x45\x2f\x2f\x00\x55\x43\x53\x2d\x32\x4c\x45\x2f\x2f\x00\x55\x43\x53\x2d\x34\x4c\x45\x2f\x2f\x00\x55\x4e\x49\x43\x4f\x44\x45\x4c\x49\x54\x54\x4c\x45\x2f\x2f\x00\x55\x4e\x49\x43\x4f\x44\x45\x42\x49\x47\x2f\x2f\x00\x55\x53\x2d\x41\x53\x43\x49\x49\x2f\x2f\x00\x43\x53\x41\x53\x43\x49\x49\x2f\x2f\x00\x49\x53\x4f\x36\x34\x36\x2d\x55\x53\x2f\x2f\x00\x57\x43\x48\x41\x52\x5f\x54\x2f\x2f\x00\x49\x53\x4f\x2d\x31\x30\x36\x34\x36\x2f\x55\x43\x53\x32\x2f\x00\x31\x30\x36\x34\x36\x2d\x31\x3a\x31\x39\x39\x33\x2f\x55\x43\x53\x34\x2f\x00\x49\x53\x4f\x2d\x31\x30\x36\x34\x36\x2f\x55\x43\x53\x34\x2f\x00\x49\x53\x4f\x2d\x31\x30\x36\x34\x36\x2f\x55\x54\x46\x2d\x38\x2f\x00\x49\x53\x4f\x2d\x31\x30\x36\x34\x36\x2f\x55\x54\x46\x38\x2f\x00\x3d\x49\x4e\x54\x45\x52\x4e\x41\x4c\x2d\x3e\x75\x63\x73\x32\x00\x3d\x49\x4e\x54\x45\x52\x4e\x41\x4c\x2d\x3e\x75\x63\x73\x34\x00\x3d\x49\x4e\x54\x45\x52\x4e\x41\x4c\x2d\x3e\x75\x74\x66\x38\x00\x3d\x75\x63\x73\x32\x2d\x3e\x49\x4e\x54\x45\x52\x4e\x41\x4c\x00\x3d\x75\x63\x73\x34\x2d\x3e\x49\x4e\x54\x45\x52\x4e\x41\x4c\x00\x3d\x75\x74\x66\x38\x2d\x3e\x49\x4e\x54\x45\x52\x4e\x41\x4c\x00\x3d\x75\x63\x73\x34\x6c\x65\x2d\x3e\x49\x4e\x54\x45\x52\x4e\x41\x4c\x00\x3d\x75\x63\x73\x32\x72\x65\x76\x65\x72\x73\x65\x2d\x3e\x49\x4e\x54\x45\x52\x4e\x41\x4c\x00\x3d\x61\x73\x63\x69\x69\x2d\x3e\x49\x4e\x54\x45\x52\x4e\x41\x4c\x00\x3d\x49\x4e\x54\x45\x52\x4e\x41\x4c\x2d\x3e\x75\x63\x73\x34\x6c\x65\x00\x3d\x49\x4e\x54\x45\x52\x4e\x41\x4c\x2d\x3e\x75\x63\x73\x32\x72\x65\x76\x65\x72\x73\x65\x00\x3d\x49\x4e\x54\x45\x52\x4e\x41\x4c\x2d\x3e\x61\x73\x63\x69\x69\x00\x00\x5d\x01\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x43\x01\x03\x00\x69\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x62\x00\x06\x00\x75\x01\x01\x00\x00\x00\x00\x00\xa7\x00\x05\x00\x00\x00\x00\x00\x1d\x00\x04\x00\x4c\x00\x06\x00\x00\x00\x00\x00\x39\x00\x01\x00\x6a\x01\x01\x00\x7f\x01\x01\x00\x0f\x00\x01\x00\xe3\x00\x01\x00\x00\x00\x00\x00\xc9\x00\x05\x00\xa9\x00\x05\x00\x00\x00\x00\x00\x00\x00\x00\x00\x85\x00\x05\x00\x00\x00\x00\x00\x30\x02\x00\x00\x00\x00\x00\x00\xfd\x00\x01\x00\xb8\x01\x05\x00\xd9\x01\x04\x00\x00\x00\x00\x00\xbe\x00\x01\x00\x1e\x01\x04\x00\x6d\x01\x01\x00\x01\x00\x06\x00\x8d\x00\x01\x00\x25\x01\x02\x00\xc8\x01\x04\x00\xf4\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x2f\x01\x05\x00\x2b\x00\x06\x00\x00\x00\x00\x00\x99\x00\x05\x00\x0d\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x05\x01\x04\x00\xd7\x00\x05\x00\x00\x00\x00\x00\x54\x00\x06\x00\x00\x00\x00\x00\x86\x01\x01\x00\x76\x00\x05\x00\xb0\x00\x05\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x95\x01\x06\x00\x00\x00\x00\x00\x4d\x01\x06\x00\xa5\x01\x05\x00\x8b\x01\x00\x00\x00\x00\x00\x00\x39\x01\x06\x00\x00\x00\x00\x00\x30\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0d\x01\x00\x00\xac\x02\x00\x00\x72\x02\x00\x00\x5d\x01\x00\x00\x95\x02\x00\x00\x5b\x02\x00\x00\x43\x01\x00\x00\x83\x02\x00\x00\x49\x02\x00\x00\xd9\x01\x00\x00\x09\x02\x00\x00\x39\x02\x00\x00\xb8\x01\x00\x00\xf9\x01\x00\x00\x29\x02\x00\x00\x95\x01\x00\x00\xe9\x01\x00\x00\x19\x02\x00\x00";
static int s_gconv_size = 1086;

static int hzn_strcmp(const char *p1, const char *p2)
{
	int i = 0;
	int failed = 0;
	while(p1[i] != '\0' && p2[i] != '\0')
	{
		if(p1[i] != p2[i])
		{
			failed = 1;
			break;
		}
		i++;
	}
	// why did the loop exit?
	if( (p1[i] == '\0' && p2[i] != '\0') || (p1[i] != '\0' && p2[i] == '\0') )
		failed = 1;

	return failed;
}

static int hzn_strncpy(char *dest, const char *src, int len)
{
	int n = 0;
	while (*src && len)
	{
		*dest++ = *src++;
		len--;
		n++;
	}
	*dest = '\0';

	return n;
}

static void *hzn_memcpy(void *dest, const void *src, size_t len)
{
	const unsigned char *sp = (const unsigned char *)src;
	unsigned char *dp = (unsigned char *)dest;
	for (; len != 0; len--) *dp++ = *sp++;
	return dest;
}


SpecialFile::SpecialFile(const char *path, const char *str, int size) {
	m_path = path;
	m_str = str;
	if(size == 0) {
		/* Can't use strlen yet - it uses SSE. */
		m_len = 0;
		while(m_str[m_len] != '\0') {
			m_len++;
		}
	} else {
		m_len = size;
	}
}

int SpecialFile::Read(char *buf, unsigned int pos, unsigned int sz) {
	if(pos >= m_len) {
		return 0;
	}
	if(pos+sz >= m_len) {
		sz = m_len-pos;
	}

	hzn_memcpy(buf, &m_str[pos], sz);
	return sz;
}

int SpecialFile::Write(const char *buf, unsigned int pos, unsigned int sz) {
	/* Writes always succeed. */
	return sz;
}

SpecialTty::SpecialTty(const char *path) :
	SpecialFile(path, "") {
};

int SpecialTty::Read(char *buf, unsigned int pos, unsigned int sz) {
	return sz;
}
int SpecialTty::Write(const char *buf, unsigned int pos, unsigned int sz) {
	/* Writes always succeed. */
	static char str[256];
	hzn_strncpy(str, buf, sz);
	str[sz] = '\0';

	dout << str;

	return sz;
}

SpecialRnd::SpecialRnd(const char *path) :
	SpecialFile(path, "") {
}

unsigned char SpecialRnd::Clock() {
	static uint16_t lfsr = 0x5556;

	uint8_t val = lfsr & 0xFF;

	uint16_t tap = ((lfsr >> 15) & 0x1) ^
		((lfsr >> 13) & 0x1) ^
		((lfsr >> 12) & 0x1) ^
		((lfsr >> 10) & 0x1) ^
	        1;
	lfsr = (lfsr >> 1) | (tap << 15);

	return val;
}

int SpecialRnd::Read(char *buf, unsigned int pos, unsigned int sz) {
	for(unsigned int i = 0; i < sz; i++) {
		buf[i] = (char)Clock();
	}

	return sz;
}
int SpecialRnd::Write(const char *buf, unsigned int pos, unsigned int sz) {
	return sz;
}

SpecialModule::SpecialModule(const char *path) :
	SpecialFile(path, "") {
}

int SpecialModule::Read(char *buf, unsigned int pos, unsigned int sz) {
	if(pos >= s_module_size) {
		return 0;
	}
	if(pos+sz >= s_module_size) {
		sz = s_module_size-pos;
	}

	char c[5];
	hzn_memcpy(c, (const void*)(s_module_base), 4);
	c[4] = 0;

	dout << "SpecialModule::read: " << c << "\n";
	hzn_memcpy(buf, (const void*)(s_module_base+pos), sz);	

	return sz;
}
int SpecialModule::Write(const char *buf, unsigned int pos, unsigned int sz) {
	return sz;
}


SpecialFile *SpecialFile::get(const char *path) {
	for(unsigned int i = 0; i < NumSpecialFiles; i++) {
		if(!hzn_strcmp(path, s_files[i]->m_path)) {
			return s_files[i];
		}
	}
	return 0;
}

void InitialiseSpecialFiles(kernel_args_t *args) {
	/* Check for the bytecode kernel module. */
	EarlyAssert(args->module_count == 1);
	kernel_args_module_t *mod = (kernel_args_module_t*)args->modules;
	EarlyAssert(mod->next == 0);
	
	s_module_base = mod->base;
	s_module_size = mod->size;

	static SpecialFile osrelease("/proc/sys/kernel/osrelease", "2.6.32-24-generic");
	s_files[0] = &osrelease;
	static SpecialTty tty("/dev/tty");
	s_files[1] = &tty;
	static SpecialRnd rnd("/dev/urandom");
	s_files[2] = &rnd;
	static SpecialModule modl("/module");
	s_files[3] = &modl;
	static SpecialFile gconv_cache("/usr/lib/gconv/gconv-modules.cache", s_gconv_module, s_gconv_size);
	s_files[4] = &gconv_cache;
}
