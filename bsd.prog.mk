OBJS = ${SRCS:.c=.o}

COMPATLIB = libopenbsd.a
COMPATSRCS= arc4random.c arc4random_uniform.c getentropy_linux.c sha2.c
COMPATSRCS+=pledge-noop.c explicit_bzero.c kqueue_epoll.c
COMPATOBJS= $(addprefix libopenbsd/,${COMPATSRCS:.c=.o})

override CFLAGS+= -O2 -MD -MP
override CFLAGS+=-I./libopenbsd/include
LDFLAGS=-Wl,--as-needed

CPPFLAGS= -D_DEFAULT_SOURCE -D_GNU_SOURCE
CPPFLAGS+= -include libopenbsd/openbsd.h
CPPFLAGS+=-D__linux__

default: ${PROG}

${COMPATLIB}: ${COMPATOBJS}
	${AR} -r $@ $?

${PROG}: ${OBJS} ${COMPATLIB}
	${CC} ${CFLAGS} ${CPPFLAGS} $^ -o $@ ${LDFLAGS}

clean:
	rm -f ${PROG} ${COMPATLIB}
	rm -f ${OBJS} ${OBJS:.o=.d} ${COMPATOBJS} ${COMPATOBJS:.o=.d}

-include ${OBJS:.o=.d} ${COMPATOBJS:.o=.d}

.PHONY: default clean
