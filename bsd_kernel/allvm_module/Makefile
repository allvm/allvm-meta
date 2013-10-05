ROOT = ${.CURDIR}

.PATH: /usr/src/sys/libkern ${ROOT}/runtime ${ROOT}/src

LLVM_COMPONENTS := nativecodegen jit ipo mcjit bitreader
LLVM_CONFIG=/home/will/llvm/slim/bin/llvm-config

CXXFLAGS := -Os -pipe -fno-rtti -fno-exceptions
CXXFLAGS += -Os -fno-strict-aliasing -D__NO_TLS -DKLD_MODULE -fno-common  -fno-omit-frame-pointer -mno-omit-leaf-frame-pointer  -mcmodel=kernel -mno-red-zone -mno-mmx -msoft-float  -fno-asynchronous-unwind-tables

CFLAGS += -Werror -I${ROOT}/include
CXXFLAGS += -Werror -I${ROOT}/include

LLVM_CXXFLAGS != ${LLVM_CONFIG} --cxxflags
LLVM_LDFLAGS != ${LLVM_CONFIG} --libs --ldflags ${LLVM_COMPONENTS}
LLVM_LDFLAGS := ${LLVM_LDFLAGS:N-lz:N-lm}

CXX := /home/will/src/uclibc-install/bin/g++-uc
LIBCXX := /home/will/src/uclibc-install/lib/libuClibc++.a

CXXFLAGS += ${LLVM_CXXFLAGS}
LINK_TAIL := -static ${LLVM_LDFLAGS} ${LIBCXX} /usr/lib/libm.a
LINK_TAIL += -wrap malloc -wrap realloc -wrap free
LINK_TAIL += -wrap getenv -wrap memcpy
LINK_TAIL += -wrap printf -wrap vprintf

SRCS := main.c
SRCS += allvm.cpp

SRCS += guard.cpp icxxabi.cpp
SRCS += mem.c memcpy.c
SRCS += abs.c atoi.c dl.c libc.c gcc.c printf.c signal.c strchr.c
SRCS += write.cpp

# Pull in libkern definitions for common functions
LIBKERN_SRC += strcat.c strcmp.c strcpy.c strcspn.c strdup.c strlcat.c strlcpy.c strlen.c
LIBKERN_SRC += strncmp.c strncpy.c strnlen.c
LIBKERN_SRC += strcasecmp.c strsep.c strspn.c strstr.c strtol.c strtoq.c
LIBKERN_SRC += memchr.c memcmp.c memmove.c memset.c
LIBKERN_SRC += index.c rindex.c
LIBKERN_SRC += qsort.c qsort_r.c

SRCS += ${LIBKERN_SRC}

KMOD := allvm-jit

.cpp.o:
	${CXX} ${CXXFLAGS} -c -o ${.TARGET} ${.IMPSRC}

.include <bsd.kmod.mk>

#############################
# Exported symbol checking
#############################

kern.syms: /boot/kernel/kernel
	@nm --defined-only -f posix ${.ALLSRC} | awk '{print $$1}' > ${.TARGET}

kmod.undef.syms: ${FULLPROG}
	@nm -u -f posix ${.ALLSRC} | awk '{print $$1}' > ${.TARGET}

missing.syms: kern.syms kmod.undef.syms
	@-grep -Fxvf kern.syms kmod.undef.syms > ${.TARGET}

symcheck: missing.syms
	@echo "Checking for undefined symbols not provided by kernel..."
	@test ! -s ${.ALLSRC} || (cat ${.ALLSRC} && exit 1)
	@echo "Check passed (none found)!"
	@-rm missing.syms kern.syms kmod.undef.syms

###################################
# Unsupported relocation checking
####################################

# Snippet taken from https://lists.cam.ac.uk/pipermail/cl-mirage/2012-July/msg00126.html
unsupported_relocs: ${FULLPROG}
	@-objdump -r ${.ALLSRC} | grep -E "(PLT32|GOTPCREL)" > ${.TARGET}

reloccheck: unsupported_relocs
	@echo "Checking for unsupported relocation types..."
	@test ! -s ${.ALLSRC} || (cat ${.ALLSRC} && exit 1)
	@echo "Check passed (none found)!"
	@-rm unsupported_relocs

sync:
	sync
	sync
	sync
	sleep 2

# Refuse to try loading kernel if we still have undef symbols..
load: symcheck reloccheck sync

#############################
# Finalization
#############################
LINK_TAIL += fin.o
CLEANFILES += fin.o
${FULLPROG}: fin.o

#############################
# Userspace test app
#############################

# TODO: Extract bc from running kernel (/dev/kmem)
#       or from final /boot/kernel/kernel.
# For now hijack build artifact

kernel.bc.o: /usr/obj/usr/src/sys/ALLVM/kernel.bc.o
	ln -sf ${.ALLSRC} ${.TARGET}

user.o: user.c
	${CC} -I${ROOT}/include -c -o ${.TARGET} ${.ALLSRC}

# Builds app *including* our fake-libc, but
# requires a few things we rely on the kernel for
#userapp: ${OBJS:Nmain.o} user.o
#	${LD} ${LDFLAGS} -o ${.TARGET} ${.ALLSRC} ${LINK_TAIL}
userapp: allvm.o user.o kernel.bc.o
	${CXX} -static ${.ALLSRC} -o ${.TARGET} ${LLVM_LDFLAGS} -lm -O2

usertest: userapp
	${.OBJDIR}/usertest kernel.bc

CLEANFILES += userapp user.o kernel.bc kernel.bc.o
