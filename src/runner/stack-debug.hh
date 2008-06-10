/**
 ** \file runner/stack-debug.hh
 ** \brief Debugging macro for the stack
 */

#ifndef RUNNER_INTERPRETER_STACK_DEBUG_HH
# define RUNNER_INTERPRETER_STACK_DEBUG_HH

# ifdef ENABLE_STACK_DEBUG_TRACES

#  include <debug.hh>

#  define STACK_ECHO(X) DEBUG("STACK", X)
#  define STACK_NECHO(X) DEBUGN("STACK", X)

#  include <boost/lexical_cast.hpp>

static std::string stack_debug(ast::rConstDeclaration decl, unsigned idx)
{
  std::string res = decl->what_get().name_get() + " (";
  if (decl->closed_get())
    res += "closed #";
  else
    res += "local #";
  res += boost::lexical_cast<std::string>(decl->local_index_get());
  res += ") @[" + boost::lexical_cast<std::string>(idx) + "]";
  return res;
}

static std::string stack_debug(ast::rConstAssignment a, unsigned idx)
{
  std::string res = a->what_get().name_get() + " (";
  if (a->closed_get())
    if (a->depth_get())
      res += "captured #";
    else
      res += "closed #";
  else
    res += "local #";
  res += boost::lexical_cast<std::string>(a->local_index_get());
  res += ") @[" + boost::lexical_cast<std::string>(idx) + "]";
  return res;
}

static std::string stack_debug(ast::rConstLocal l, unsigned idx)
{
  std::string res = l->name_get().name_get() + " (";
  if (l->closed_get())
    if (l->depth_get())
      res += "captured #";
    else
      res += "closed #";
  else
    res += "local #";
  res += boost::lexical_cast<std::string>(l->local_index_get());
  res += ") @[" + boost::lexical_cast<std::string>(idx) + "]";
  return res;
}

# else
#  define STACK_ECHO(X)
#  define STACK_NECHO(X)
# endif

#endif
