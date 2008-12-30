/*
*/

#ifndef CORO_DEFINED
#define CORO_DEFINED 1

#include <scheduler/libcoroutine/Common.h>
#include <scheduler/libcoroutine/PortableUContext.h>

#if defined(__SYMBIAN32__)
	#define CORO_STACK_SIZE     8192
	#define CORO_STACK_SIZE_MIN 1024
#else
	 //#define CORO_DEFAULT_STACK_SIZE     (65536/2)
	 #define CORO_DEFAULT_STACK_SIZE  (65536*4)
	//128k needed on PPC due to parser
	#define CORO_STACK_SIZE_MIN 8192
#endif

/* We dont build libcoro as a separate library: comment this.
#if !defined(__MINGW32__) && defined(WIN32)
#if defined(BUILDING_CORO_DLL) || defined(BUILDING_IOVMALL_DLL)
#define CORO_API __declspec(dllexport)
#else
#define CORO_API __declspec(dllimport)
#endif

#else
#define CORO_API
#endif
*/
#define CORO_API

/*
#if defined(__amd64__) && !defined(__x86_64__)
	#define __x86_64__ 1
#endif
*/

// Pick which coro implementation to use
// The make file can set -DUSE_FIBERS, -DUSE_UCONTEXT or -DUSE_SETJMP to force this choice.
#if !defined(USE_FIBERS) && !defined(USE_UCONTEXT) && !defined(USE_SETJMP)

#if defined(WIN32) && defined(HAS_FIBERS)
#define USE_FIBERS
#elif defined(HAS_UCONTEXT)
//#elif defined(HAS_UCONTEXT) && !defined(__x86_64__)
#define USE_UCONTEXT
#else
#define USE_SETJMP
#endif

#endif

#if defined(USE_FIBERS)
	#define CORO_IMPLEMENTATION "fibers"
#elif defined(USE_UCONTEXT)
// OSX (at least 10.5.4) does define all the function (swapcontext
// etc.) we want, but the currently this library still wants to define
// (and declare) them.  Since they are declared, with different
// prototypes, in <ucontext.h>, there is a clash.  So instead of
// pulling the definition of these functions from <ucontext.h>, just
// pull the definition of the structures we need from <sys/ucontext.h>
// and let this library provide its functions.
# if defined __APPLE__
	#include <sys/ucontext.h>
# else
	#include <ucontext.h>
#endif
	#define CORO_IMPLEMENTATION "ucontext"
#elif defined(USE_SETJMP)
	#include <setjmp.h>
	#define CORO_IMPLEMENTATION "setjmp"
#endif



typedef struct Coro Coro;

struct Coro
{
	size_t requestedStackSize;
	size_t allocatedStackSize;
	void *stack;

#ifdef USE_VALGRIND
	unsigned int valgrindStackId;
#endif

#if defined(USE_FIBERS)
	void *fiber;
#elif defined(USE_UCONTEXT)
	ucontext_t env;
#elif defined(USE_SETJMP)
	jmp_buf env;
#endif

	unsigned char isMain;
};

CORO_API Coro *Coro_new(void);
CORO_API void Coro_free(Coro *self);

// stack

CORO_API void *Coro_stack(Coro *self);
CORO_API size_t Coro_stackSize(Coro *self);
CORO_API void Coro_setStackSize_(Coro *self, size_t sizeInBytes);
CORO_API size_t Coro_bytesLeftOnStack(Coro *self);
CORO_API int Coro_stackSpaceAlmostGone(Coro *self);

CORO_API void Coro_initializeMainCoro(Coro *self);

typedef void (CoroStartCallback)(void *);

CORO_API void Coro_startCoro_(Coro *self, Coro *other, void *context, CoroStartCallback *callback);
CORO_API void Coro_switchTo_(Coro *self, Coro *next);
CORO_API void Coro_setup(Coro *self, void *arg); // private

#endif
