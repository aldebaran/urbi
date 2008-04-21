#ifndef SCHEDULER_COROUTINE_HH
#define SCHEDULER_COROUTINE_HH

# include "scheduler/libcoroutine/Coro.h"

// This package provides an interface to the libcoroutine. Using this
// interface allows for various checks and instrumentations to be
// easily added without modifying the imported libcoroutine.

// Create a new coroutine with a specified stack size, or the default one
// if stack_size is 0.
Coro* coroutine_new(size_t stack_size = 0);

// Free the space used by a coroutine. Cannot be called from the coroutine
// itself.
void coroutine_free(Coro*);

// Start a coroutine
void coroutine_start(Coro* self, Coro* other, void* context,
		     CoroStartCallback* callback);

// Switch to a coroutine
void coroutine_switch_to(Coro* self, Coro* next);

// Check whether the stack space is sufficient or near exhaustion.
bool coroutine_stack_space_almost_gone(Coro*);

// Initialize main coroutine
void coroutine_initialize_main(Coro*);

# include "scheduler/coroutine.hxx"

#endif // SCHEDULER_COROUTINE_HH
