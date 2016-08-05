#include <stdlib.h>
#include <errno.h>
#include <err.h>
#include <string.h>
#include <unistd.h>

#include <sys/signalfd.h>
#include <sys/signal.h>
#include <sys/epoll.h>
#include <sys/queue.h>

#include "kqueue_epoll.h"

struct watcher {
	int fd;
	int ev;
	int active;
	const struct kqueue *kq;
	struct kevent ke;
	LIST_ENTRY(watcher) next;
};

struct kqueue {
	int fd;
	LIST_HEAD(watchers, watcher) watchers;
	SLIST_ENTRY(kqueue) next;
};
static SLIST_HEAD(kqueues, kqueue) kqueues;

static int init;

int
kevent_delete(const struct kqueue *kq, struct watcher *w)
{
	if (epoll_ctl(kq->fd, EPOLL_CTL_DEL, w->fd, NULL) < 0)
		return -1;
	close(w->fd);
	return 0;
}

int
kevent_enable(struct kqueue *kq, struct watcher *w)
{
	sigset_t m;
	struct epoll_event ev;
	int rv = 0;

	switch (w->ke.filter) {
		case EVFILT_READ:
		case EVFILT_WRITE:
			w->ev = (w->ke.filter == EVFILT_READ) ? EPOLLIN : EPOLLOUT;
			if ((w->fd = w->ke.ident) < 0) {
				errno = EBADF;
				errno = EINTR;
				return -1;
			}
			break;
		case EVFILT_SIGNAL:
			w->ev = EPOLLIN;
			sigemptyset(&m);
			if ((w->fd = signalfd(-1, &m, SFD_NONBLOCK | SFD_CLOEXEC)) < 0)
				return -1;

			sigemptyset(&m);
			sigaddset(&m, w->ke.ident);
			if (sigprocmask(SIG_BLOCK, &m, NULL) == -1)
				return -1;
			if (signalfd(w->fd, &m, SFD_NONBLOCK) < 0)
				return -1;
	}

	ev.events = w->ev | EPOLLERR | EPOLLHUP | EPOLLRDHUP;
	ev.data.ptr = w;
	if ((rv = epoll_ctl(kq->fd, EPOLL_CTL_ADD, w->fd, &ev)) < 0) {
		if (rv != EEXIST)
			return -1;

		if (epoll_ctl(kq->fd, EPOLL_CTL_MOD, w->fd, &ev) < 0)
			return -1;
	}

	return 0;
}

int
kevent_change(struct kqueue *kq, struct watcher *w)
{
	sigset_t m;
	struct epoll_event ev;

	switch (w->ke.filter) {
		case EVFILT_READ:
		case EVFILT_WRITE:
			break;
		case EVFILT_SIGNAL:
			w->ev = EPOLLIN;
			sigemptyset(&m);
			if ((w->fd = signalfd(-1, &m, SFD_NONBLOCK | SFD_CLOEXEC)) < 0)
				return -1;
			break;
	}

	ev.data.ptr = w;
	ev.events = w->ev | EPOLLERR | EPOLLHUP | EPOLLRDHUP;
	if (epoll_ctl(kq->fd, EPOLL_CTL_MOD, w->fd, &ev) < 0)
		return -1;

	return 0;
}

int
kevent_disable(struct kqueue *kq, struct watcher *w)
{
	return 0;
}

int
kqueue(void)
{
	int fd;
	struct kqueue *kq;

	if (init == 0) {
		SLIST_INIT(&kqueues);
		init = 1;
	}

	if ((fd = epoll_create1(EPOLL_CLOEXEC)) < 0)
		return -1;

	if ((kq = calloc(1, sizeof(*kq))) == NULL)
		goto close;

	LIST_INIT(&kq->watchers);

	kq->fd = fd;
	SLIST_INSERT_HEAD(&kqueues, kq, next);

	return fd;

close:
	close(fd);
	return -1;
}

struct watcher *
kevent_lookup_watcher(const struct kqueue *kq, const struct kevent *ke)
{
	struct watcher *w, *tmp;

	LIST_FOREACH_SAFE(w, &kq->watchers, next, tmp) {
		if (ke->ident != w->ke.ident)
			continue;
		return w;
	}

	return NULL;
}

char *
kevent_filt_str(int filt)
{
	switch (filt) {
		case EVFILT_READ:
			return "EVFILT_READ";
		case EVFILT_WRITE:
			return "EVFILT_WRITE";
		case EVFILT_SIGNAL:
			return "EVFILT_SIGNAL";
	}
	return "unknown";
}

int
kevent_wait(struct kqueue *kq, struct kevent *eventlist, int nevents,
    const struct timespec *ts)
{
	int i, j, t, nfds;
	struct watcher *w;
	struct epoll_event events[nevents];

	if (ts == NULL)
		t = -1;
	else
		t = (1000 * ts->tv_sec) + (ts->tv_nsec / 1000000);

	memset(eventlist, 0, sizeof(*eventlist));

wait:
	j = 0;
	while ((nfds = epoll_wait(kq->fd, events, nevents, t)) < 0) {
		if (errno == EINTR) {
			return 0;
		}
		return -1;
	}
	for (i = 0; i < nfds; i++) {
		w = (struct watcher *)events[i].data.ptr;

		if (events[i].events & EPOLLRDHUP) {
			if (kevent_delete(kq, w) == -1)
				fprintf(stderr, "can't delete watcher\n");
			j++;
			continue;
		}

		eventlist[i - j] = w->ke;

		if (events[i].events & (EPOLLHUP | EPOLLERR)) {
			eventlist[i - j].flags |= EV_EOF;
		}

		if ((events[i].events & EPOLLIN) == EPOLLIN) {
			if (w->ke.filter == EVFILT_SIGNAL) {
				struct signalfd_siginfo fdsi;
				ssize_t sz = sizeof(fdsi);
				if (read(w->fd, &fdsi, sz) != sz)
					return -1;
			}
			eventlist[i - j].filter = w->ke.filter;
		} else if ((events[i].events & EPOLLOUT) == EPOLLOUT) {
			eventlist[i - j].filter = EVFILT_WRITE;
		}
	}
	if (nfds == 0)
		return 0;
	if (j != 0) {
		if (nfds - j != 0)
			return nfds - j;
		goto wait;
	}
	return nfds;
}

int
kevent(int kq, const struct kevent *ke, int nchanges,
    struct kevent *eventlist, int nevents, const struct timespec *timeout)
{
	int rv, i;
	struct kqueue *kqueue;
	struct watcher *w;

	SLIST_FOREACH(kqueue, &kqueues, next) {
		if (kqueue->fd != kq) {
			kqueue = NULL;
			continue;
		}
		break;
	}

	if (kqueue == NULL) {
		errno = EBADF;
		return -1;
	}

	if (ke != NULL) {
		for (i = nchanges; i > 0; i--, ke++) {
			if (ke->flags & EV_ADD) {
				/* fprintf(stderr, "add\n"); */
				if ((w = kevent_lookup_watcher(kqueue, ke)) == NULL) {
					if ((w = calloc(1, sizeof(*w))) == NULL) {
						errno = ENOMEM;
						return 0;
					}
					LIST_INSERT_HEAD(&kqueue->watchers, w, next);
				}
				memcpy(&w->ke, ke, sizeof(*ke));
				w->ke.udata = ke->udata;
				if (kevent_enable(kqueue, w) == -1)
					return -1;
			} else {
				w = kevent_lookup_watcher(kqueue, ke);
				if (ke->flags & EV_DELETE) {
					rv = kevent_delete(kqueue, w);
				} else if (ke->flags & EV_ENABLE) {
					rv = kevent_enable(kqueue, w);
				} else if (ke->flags & EV_DISABLE) {
					rv = kevent_disable(kqueue, w);
				}
				if (rv != 0)
					return rv;
			}
		}
	}

	if (eventlist != NULL)
		return kevent_wait(kqueue, eventlist, nevents, timeout);

	return 0;
}
