//<<-
// Generated, do not edit by hand.
//->>
/**
 ** \file ast/fwd.hh
 ** \brief Forward declarations of all node-classes of AST
 ** (needed by the visitors)
 */
#ifndef AST_FWD_HH
# define AST_FWD_HH

# include <list>
# include "misc/fwd.hh"

namespace ast
{

  class AssignExp;
  class Ast;
  class Exp;
  class IntExp;
  class Var;


/*
  // From decs.hh.
  class Decs;

  // From decs-list.hh.
  class DecsList;

  // From anydecs.hh.
  template <typename T>
  class AnyDecs;
  typedef AnyDecs<VarDec> VarDecs;
  typedef AnyDecs<TypeDec> TypeDecs;
  typedef AnyDecs<FunctionDec> FunctionDecs;

  // From anydecs.hh.
  typedef std::list<Exp*> exps_type;
  typedef std::list<FieldInit*> fieldinits_type;
  typedef std::list<Field*> fields_type;
*/

  template <template <typename> class Const>
  class GenVisitor;
  typedef GenVisitor<misc::constify_traits> ConstVisitor;
  typedef GenVisitor<misc::id_traits> Visitor;

} // namespace ast

#endif // !AST_FWD_HH
