/*-
 * Copyright (c) 1999,2000,2001 Jonathan Lemon <jlemon@FreeBSD.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *      $FreeBSD: src/sys/sys/event.h,v 1.11 2001/02/24 01:41:31 jlemon Exp $
 */

#ifndef _KQUEUE_UEV_H_
#define _KQUEUE_UEV_H_
#include <stdint.h>
#include <sys/types.h>

#include <uev/uev.h>

#include "openbsd.h"

#define EVFILT_READ             (-1)
#define EVFILT_WRITE            (-2)
#define EVFILT_AIO              (-3)    /* attached to aio requests */
#define EVFILT_VNODE            (-4)    /* attached to vnodes */
#define EVFILT_PROC             (-5)    /* attached to struct process */
#define EVFILT_SIGNAL           (-6)    /* attached to struct process */
#define EVFILT_TIMER            (-7)    /* timers */

#define EVFILT_SYSCOUNT         7


#define EV_SET(kevp_, a, b, c, d, e, f) do {    \
        struct kevent *kevp = (kevp_);          \
        (kevp)->ident = (a);                    \
        (kevp)->filter = (b);                   \
        (kevp)->flags = (c);                    \
        (kevp)->fflags = (d);                   \
        (kevp)->data = (e);                     \
        (kevp)->udata = (f);                    \
} while(0)

struct kevent {
	uintptr_t  ident;	/* identifier for this event */
	short	   filter;	/* filter for event */
	u_short	   flags;	/* action flags for kqueue */
	u_int	   fflags;	/* filter flag value */
	quad_t	   data;	/* filter data value */
	void	   *udata;	/* opaque user data identifier */
};

/* actions */
#define EV_ADD          0x0001          /* add event to kq (implies enable) */
#define EV_DELETE       0x0002          /* delete event from kq */
#define EV_ENABLE       0x0004          /* enable event */
#define EV_DISABLE      0x0008          /* disable event (not reported) */

/* flags */
#define EV_ONESHOT      0x0010          /* only report one occurrence */
#define EV_CLEAR        0x0020          /* clear event state after reporting */

#define EV_SYSFLAGS     0xF000          /* reserved by system */
#define EV_FLAG1        0x2000          /* filter-specific flag */

/* returned values */
#define EV_EOF          0x8000          /* EOF detected */
#define EV_ERROR        0x4000          /* error, data contains errno */

int kqueue(void);
int kevent(int, const struct kevent *, int, struct kevent *, int, const struct timespec *);

#endif /* _KQUEUE_UEV_H_ */
