#ifndef _LIBOPENBSD_H_
#define _LIBOPENBSD_H_

#ifdef __linux__
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_SYSMACROS_H
# include <sys/sysmacros.h> /* For MIN, MAX, etc */
#endif

#define __BEGIN_DECLS
#define __END_DECLS

#ifndef __weak_alias
# define __weak_alias(new, old) \
	extern __typeof(old) new __attribute__((weak, alias(#old)))
#endif

#ifndef __UNUSED
# define __UNUSED __attribute__ ((unused))
#endif

#ifndef __dead
# define __dead __attribute__((__noreturn__))
#endif

#ifndef __bounded__
# define __bounded__(x, y, z)
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

int  pledge(const char *promises, const char *paths[]);
void explicit_bzero(void *buf, size_t len);
int  getentropy(void *buf, size_t len);

uint32_t arc4random(void);
uint32_t arc4random_uniform(uint32_t);
void     arc4random_buf(void *buf, size_t n);

#endif /* _LIBOPENBSD_H_ */
