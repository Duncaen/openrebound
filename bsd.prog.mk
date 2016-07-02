OBJS = ${SRCS:.c=.o}

COMPATLIB = libopenbsd.a
COMPATSRCS= pledge-noop.c arc4random.c explicit_bzero.c kqueue_epoll.c
COMPATOBJS= $(addprefix libopenbsd/,${COMPATSRCS:.c=.o})

CFLAGS+= -std=c11 -D_DEFAULT_SOURCE -D_GNU_SOURCE -D__linux__
CFLAGS+= -MD -MP -I./libopenbsd

default: ${PROG}

${COMPATLIB}: ${COMPATOBJS}
	${AR} -r $@ $?

${PROG}: ${OBJS} ${COMPATLIB}
	${CC} ${CFLAGS} $^ -o $@ ${LDFLAGS}

clean:
	rm -f ${PROG} ${COMPATLIB}
	rm -f ${OBJS} ${OBJS:.o=.d} ${COMPATOBJS} ${COMPATOBJS:.o=.d}

-include ${OBJS:.o=.d} ${COMPATOBJS:.o=.d}

.PHONY: default clean
