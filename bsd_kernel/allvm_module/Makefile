
.PATH: /usr/src/sys/libkern horizon-baremetal

LLVM_COMPONENTS := nativecodegen jit
LLVM_CONFIG=/usr/local/bin/llvm-config

CXX := /usr/local/bin/clang++
CXXFLAGS := -O2 -pipe -fno-rtti -fno-exceptions
CXXFLAGS += -Ihorizon-baremetal

LLVM_CXXFLAGS != ${LLVM_CONFIG} --cxxflags
LLVM_LDFLAGS != ${LLVM_CONFIG} --libs --ldflags ${LLVM_COMPONENTS}

CXXFLAGS += ${LLVM_CXXFLAGS}
LINK_TAIL := -static ${LLVM_LDFLAGS} -lstdc++ -lm -lc

SRCS := main.c jit.cpp

SRCS += unwind.c
# SRCS += libc.c
# LIBKERN_SRC := memchr.c

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
	@grep -Fxvf kern.syms kmod.undef.syms > ${.TARGET}

symcheck: missing.syms
	@echo "Checking for undefined symbols not provided by kernel..."
	@test ! -s ${.ALLSRC} || (cat ${.ALLSRC} && exit 1)
	@echo "Check passed (none found)!"

# Refuse to try loading kernel if we still have undef symbols..
load: symcheck

#############################
# insn-swap building
#############################

INSN_LLVM_LDFLAGS != ${LLVM_CONFIG} --libs --ldflags ${LLVM_COMPONENTS} all
INSN_LDFLAGS := ${INSN_LLVM_LDFLAGS} -ludis86 -ltinfo

insn-swap: insn-swap.cc
	${CXX} -I/usr/local/include ${.ALLSRC} ${LLVM_CXXFLAGS} ${INSN_LDFLAGS} -o ${.TARGET}

CLEANFILES += insn-swap
