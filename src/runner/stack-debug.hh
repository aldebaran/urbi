/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file runner/stack-debug.hh
 ** \brief Debugging macro for the stacks
 */

#ifndef RUNNER_INTERPRETER_STACK_DEBUG_HH
# define RUNNER_INTERPRETER_STACK_DEBUG_HH

# ifdef ENABLE_STACK_DEBUG_TRACES

#  include <kernel/debug.hh>

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
