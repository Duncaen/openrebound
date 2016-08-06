OBJS= ${SRCS:.c=.o}
LIB =libopenbsd/libopenbsd.a

LIBSRCS = arc4random.c arc4random_uniform.c getentropy_linux.c sha2.c
LIBSRCS+= pledge-noop.c explicit_bzero.c kqueue_epoll.c
LIBOBJS = ${LIBSRCS:%.c=libopenbsd/%.o}

CPPFLAGS+= -D_DEFAULT_SOURCE -D_GNU_SOURCE -D__linux__
CPPFLAGS+=-I./libopenbsd/include -include libopenbsd/openbsd.h
override CFLAGS+= -MD -MP

default: ${PROG}

${LIB}: ${LIBOBJS}
	${AR} -r $@ $?

${PROG}: ${OBJS} ${LIB}
	${CC} ${CFLAGS} ${CPPFLAGS} $^ -o $@ ${LDFLAGS}

clean:
	rm -f ${PROG} ${LIB} *.o *.d libopenbsd/*.o libopenbsd/*.d

-include ${OBJS:.o=.d} ${LIBOBJS:.o=.d}

.PHONY: default clean
