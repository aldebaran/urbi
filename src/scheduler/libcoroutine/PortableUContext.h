#ifndef PORTABLEUCONTEXT_DEFINED
#define PORTABLEUCONTEXT_DEFINED 1

//#if defined(__APPLE__) || defined(linux) || defined(__NetBSD__) || defined(__FreeBSD__) || (defined(__SVR4) && defined (__sun))
#if defined(linux) || defined(__NetBSD__) || defined(__FreeBSD__) || (defined(__SVR4) && defined (__sun))
#define HAS_UCONTEXT 1
#endif

#if defined(__APPLE__)
#if !defined(_BSD_PPC_SETJMP_H_)
#include <setjmp.h>
#define HAS_UCONTEXT 1
#endif
#endif

#if defined(__FreeBSD__) ||  defined(__APPLE__)

#include <stdarg.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sched.h>
#include <signal.h>
#include <sys/utsname.h>
#include <inttypes.h>
#include <sys/ucontext.h>
typedef unsigned long ulong;

#if defined(__FreeBSD__) && __FreeBSD__ < 5
# ifdef __cplusplus
extern "C"
{
# endif
extern	int		getmcontext(mcontext_t*);
extern	void		setmcontext(mcontext_t*);
#define	setcontext(u)	setmcontext(&(u)->uc_mcontext)
#define	getcontext(u)	getmcontext(&(u)->uc_mcontext)
extern	int		swapcontext(ucontext_t*, ucontext_t*);
extern	void		makecontext(ucontext_t*, void(*)(), int, ...);
# ifdef __cplusplus
}
# endif
#endif



#if defined(__APPLE__)
#	define mcontext libthread_mcontext
#	define mcontext_t libthread_mcontext_t
#	define ucontext libthread_ucontext
#	define ucontext_t libthread_ucontext_t
#	if defined(__i386__)
#		include <scheduler/libcoroutine/PortableUContext386.h>
#	else
#		include <scheduler/libcoroutine/PortableUContextPPC.h>
#	endif
#endif

#if defined(__OpenBSD__)
#	define mcontext libthread_mcontext
#	define mcontext_t libthread_mcontext_t
#	define ucontext libthread_ucontext
#	define ucontext_t libthread_ucontext_t
#	if defined __i386__
#		include <scheduler/libcoroutine/PortableUContext386.h>
#	else
#		include <scheduler/libcoroutine/PortableUContextPPC.h>
#	endif
extern pid_t rfork_thread(int, void*, int(*)(void*), void*);
#endif

#if 0 &&  defined(__sun__)
#	define mcontext libthread_mcontext
#	define mcontext_t libthread_mcontext_t
#	define ucontext libthread_ucontext
#	define ucontext_t libthread_ucontext_t
#	include <scheduler/libcoroutine/sparc-ucontext.h>
#endif

#if defined(__arm__)
int getmcontext(mcontext_t*);
void setmcontext(const mcontext_t*);
#define        setcontext(u)   setmcontext(&(u)->uc_mcontext)
#define        getcontext(u)   getmcontext(&(u)->uc_mcontext)
#endif

// --------------------------

#if defined(__APPLE__) && defined(__i386__)
#define NEEDX86MAKECONTEXT
#define NEEDSWAPCONTEXT
#endif

#if defined(__APPLE__) && !defined(__i386__)
#define NEEDPOWERMAKECONTEXT
#define NEEDSWAPCONTEXT
#endif

#if defined(__FreeBSD__) && defined(__i386__) && __FreeBSD__ < 5
#define NEEDX86MAKECONTEXT
#define NEEDSWAPCONTEXT
#endif


#endif


#if defined(HAS_UCONTEXT) &&  defined(__arm__)
#include <features.h>
#if defined(__UCLIBC__)
/* UClibc does not have ucontext. Use our implementation. */
#include <sys/ucontext.h>
# ifdef __cplusplus
extern "C" {
# endif
int getmcontext(mcontext_t*);
void setmcontext(const mcontext_t*);
int		swapcontext(ucontext_t*, ucontext_t*);
void		makecontext(ucontext_t*, void(*)(), int, ...);
# ifdef __cplusplus
}
# endif
/* The sigcontext structure (asm/sigcontext.h) in uc_mcontext starts with three
 * unsigned long fields before the registers, but setmcontext and getmcontext
 * do not expect them.
 */
#define	setcontext(u) \
   setmcontext((mcontext_t*)( (char*)(&(u)->uc_mcontext) + 12))
#define	getcontext(u)  \
   getmcontext((mcontext_t*)((char*)(&(u)->uc_mcontext) + 12))
#define NEEDSWAPCONTEXT
#define NEEDARMMAKECONTEXT
#endif
#endif

#endif
