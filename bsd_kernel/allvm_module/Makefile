
.PATH: /usr/src/sys/libkern

CXX := /usr/local/bin/clang++
CXXFLAGS := -O2 -pipe -fno-rtti -fno-exceptions -fno-builtin
LINK_TAIL := -static
# LINK_TAIL += -lstdc++ -lm

SRCS := main.c jit.cpp

SRCS += unwind.c libc.c

LIBKERN_SRC := memchr.c

SRCS += ${LIBKERN_SRC}

KMOD := allvm-jit

.cpp.o:
	${CXX} ${CXXFLAGS} -c -o ${.TARGET} ${.IMPSRC}

.include <bsd.kmod.mk>


