#ifndef _LIBOPENBSD_H_
#define _LIBOPENBSD_H_

#include <stdlib.h>

#ifdef __linux__
#include <sys/param.h>
#include <stdint.h>
#endif

#ifdef HAVE_SYS_SYSMACROS_H
# include <sys/sysmacros.h> /* For MIN, MAX, etc */
#endif

#ifndef __UNUSED
# define __UNUSED __attribute__ ((unused))
#endif

#ifndef __dead
# define __dead
#endif

/* src/sys/sys/time.h */
#ifndef timespeccmp
#define	timespeccmp(tsp, usp, cmp)                                      \
        (((tsp)->tv_sec == (usp)->tv_sec) ?                             \
            ((tsp)->tv_nsec cmp (usp)->tv_nsec) :                       \
            ((tsp)->tv_sec cmp (usp)->tv_sec))
#endif


#define timespecsub(tsp, usp, vsp)                                      \
        do {                                                            \
                (vsp)->tv_sec = (tsp)->tv_sec - (usp)->tv_sec;          \
                (vsp)->tv_nsec = (tsp)->tv_nsec - (usp)->tv_nsec;       \
                if ((vsp)->tv_nsec < 0) {                               \
                        (vsp)->tv_sec--;                                \
                        (vsp)->tv_nsec += 1000000000L;                  \
                }                                                       \
        } while (0)


#ifndef BSD_TYPES
typedef uint32_t u_int32_t;
#endif

int pledge(const char *promises, const char *paths[]);

void explicit_bzero(void *buf, size_t len);

u_int32_t arc4random_uniform(u_int32_t upper_bound);
void arc4random_buf(void *buf, size_t n);

#endif /* _LIBOPENBSD_H_ */
