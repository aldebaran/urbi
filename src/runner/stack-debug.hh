/**
 ** \file runner/stack-debug.hh
 ** \brief Debugging macro for the stacks
 */

#ifndef RUNNER_INTERPRETER_STACK_DEBUG_HH
# define RUNNER_INTERPRETER_STACK_DEBUG_HH

# ifdef ENABLE_STACK_DEBUG_TRACES

#  include <debug.hh>

#  define STACK_ECHO(X) DEBUG("STACK", X)
#  define STACK_NECHO(X) DEBUGN("STACK", X)
#  define STACK_OPEN(X) DEBUG_OPEN("STACK")
#  define STACK_IF_DEBUG(X) X

# else

#  define STACK_ECHO(X)
#  define STACK_NECHO(X)
#  define STACK_IF_DEBUG(X)
#  define STACK_OPEN()

# endif

#endif
