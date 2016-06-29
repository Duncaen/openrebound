OBJS = ${SRCS:.c=.o}

CFLAGS += -MD -MP -std=c11 -D_DEFAULT_SOURCE -D_GNU_SOURCE -D__linux__

OPENBSD_SRCS = libopenbsd/pledge-noop.c libopenbsd/arc4random.c libopenbsd/explicit_bzero.c
OPENBSD_OBJS = ${OPENBSD_SRCS:.c=.o}
STATIC_LIBS += libopenbsd.a
CFLAGS += -I./libopenbsd

# KQUEUE_CFLAGS = $(shell pkg-config libkqueue --cflags)
# KQUEUE_LDFLAGS = $(shell pkg-config libkqueue --libs)
KQUEUE_CFLAGS = -I./libkqueue/include
KQUEUE_LDFLAGS = -lpthread # $(shell pkg-config libkqueue --libs)
STATIC_LIBS += libkqueue/.libs/libkqueue.a
CFLAGS += ${KQUEUE_CFLAGS}
LDFLAGS += ${KQUEUE_LDFLAGS}

default: ${PROG}

libopenbsd.a: ${OPENBSD_OBJS}
	${AR} -r $@ $?

${PROG}: ${OBJS} ${STATIC_LIBS}
	${CC} ${CFLAGS} $^ -o $@ ${LDFLAGS}

clean:
	rm -f ${OBJS} ${OBJS:.o=.d}
	rm -f ${OPENBSD_OBJS} ${OPENBSD_OBJS:.o=.d}

-include ${OBJS:.o=.d} ${OPENBSD_OBJS:.o=.d}

.PHONY: default clean
