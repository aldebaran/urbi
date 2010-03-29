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
 ** \file binder/bind-debug.hh
 ** \brief Debugging macro for the bind
 */

#ifndef BINDER_INTERPRETER_BIND_DEBUG_HH
# define BINDER_INTERPRETER_BIND_DEBUG_HH

# ifdef ENABLE_BIND_DEBUG_TRACES

#  include <debug.hh>

#  define BIND_ECHO(X) DEBUG("BIND ", X)
#  define BIND_NECHO(X) DEBUGN("BIND ", X)

# else
#  define BIND_ECHO(X)
#  define BIND_NECHO(X)
# endif


#endif
