
.PATH: /usr/src/sys/libkern

LLVM_COMPONENTS := nativecodegen jit
LLVM_CONFIG=/usr/local/bin/llvm-config

CXX := /usr/local/bin/clang++
CXXFLAGS := -O2 -pipe -fno-rtti -fno-exceptions -fno-builtin

LLVM_CXXFLAGS != ${LLVM_CONFIG} --cxxflags
LLVM_LDFLAGS != ${LLVM_CONFIG} --libs --ldflags ${LLVM_COMPONENTS}

CXXFLAGS += ${LLVM_CXXFLAGS}
LINK_TAIL := -static ${LLVM_LDFLAGS}

SRCS := main.c jit.cpp

SRCS += unwind.c libc.c

LIBKERN_SRC := memchr.c

SRCS += ${LIBKERN_SRC}

KMOD := allvm-jit

.cpp.o:
	${CXX} ${CXXFLAGS} -c -o ${.TARGET} ${.IMPSRC}

.include <bsd.kmod.mk>


