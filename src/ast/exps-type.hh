#ifndef AST_EXPS_TYPE_HH
# define AST_EXPS_TYPE_HH

# include <iosfwd>
# include <list>

# include <ast/fwd.hh>

namespace ast
{

  class Exp;

  // FIXME: we would very much like to make exps_type a deque
  // (sometimes we prepend, so vector is not OK), but the top-level
  // REP loop uses a Nary which is an exps_type wrapper, and we
  // sometimes push stuff into it at the same time that a runner works
  // on the first part of the Nary.  As a result, boom: things moved
  // and pointers are junk.  On the other hand, lists give the
  // guarantee that stored element will not move when appending, so it
  // just works.

  /// List of expressions, for List, Nary, Call etc.
  typedef std::list<rExp> exps_type;

  /// Separated by commas.
  std::ostream&
  operator<<(std::ostream& o, const ast::exps_type& ss);

}

#endif // ! AST_EXPS_TYPE_HH
