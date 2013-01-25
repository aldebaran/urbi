/*
 * Copyright (C) 2006-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

// Generated, do not edit by hand.

/**
 ** \file ast/fwd.hh
 ** \brief Forward declarations of all AST classes
 ** (needed by the visitors).
 */

#ifndef AST_FWD_HH
# define AST_FWD_HH

# include <boost/ptr_container/ptr_vector.hpp>

# include <libport/fwd.hh>
# include <libport/hash.hh>
# include <libport/separate.hh>


# include <boost/preprocessor/array.hpp>
# include <boost/preprocessor/repeat.hpp>
# include <boost/preprocessor/seq/for_each.hpp>

# include <libport/intrusive-ptr.hh>
# include <libport/typelist.hh>

namespace ast
{

# define AST_NODES_SEQ          \
  (And)                         \
  (Assign)                      \
  (Assignment)                  \
  (Ast)                         \
  (At)                          \
  (Binding)                     \
  (Break)                       \
  (Call)                        \
  (CallMsg)                     \
  (Catch)                       \
  (Class)                       \
  (Composite)                   \
  (Continue)                    \
  (Declaration)                 \
  (Decrementation)              \
  (Dictionary)                  \
  (Do)                          \
  (Emit)                        \
  (Event)                       \
  (Exp)                         \
  (Finally)                     \
  (Flavored)                    \
  (Float)                       \
  (Foreach)                     \
  (If)                          \
  (Implicit)                    \
  (Incrementation)              \
  (LValue)                      \
  (LValueArgs)                  \
  (List)                        \
  (Local)                       \
  (LocalAssignment)             \
  (LocalDeclaration)            \
  (LocalWrite)                  \
  (Match)                       \
  (MetaArgs)                    \
  (MetaCall)                    \
  (MetaExp)                     \
  (MetaId)                      \
  (MetaLValue)                  \
  (Nary)                        \
  (Noop)                        \
  (OpAssignment)                \
  (Pipe)                        \
  (Property)                    \
  (PropertyAction)              \
  (PropertyWrite)               \
  (Return)                      \
  (Routine)                     \
  (Scope)                       \
  (Stmt)                        \
  (String)                      \
  (Subscript)                   \
  (TaggedStmt)                  \
  (This)                        \
  (Throw)                       \
  (Try)                         \
  (Unary)                       \
  (Unscope)                     \
  (Watch)                       \
  (While)                       \
  (Write)                       \

#  define AST_FOR_EACH_NODE(Macro)                 \
    BOOST_PP_SEQ_FOR_EACH(Macro, , AST_NODES_SEQ)


#  define VISIT(Macro, Data, Node)                                  \
  class Node;                                                       \
  typedef libport::intrusive_ptr<Node> BOOST_PP_CAT(r, Node);       \
  typedef libport::intrusive_ptr<const Node> BOOST_PP_CAT(rConst, Node);

  AST_FOR_EACH_NODE(VISIT);
#  undef VISIT
  typedef TYPELIST_52(And, Assign, Assignment, At, Binding, Break, Call, CallMsg, Catch, Class, Continue, Declaration, Decrementation, Dictionary, Do, Emit, Event, Finally, Float, Foreach, If, Implicit, Incrementation, List, Local, LocalAssignment, LocalDeclaration, Match, MetaArgs, MetaCall, MetaExp, MetaId, MetaLValue, Nary, Noop, OpAssignment, Pipe, Property, PropertyWrite, Return, Routine, Scope, Stmt, String, Subscript, TaggedStmt, This, Throw, Try, Unscope, Watch, While) Nodes;


  // From visitor.hh
  template <template <typename> class Const>
  class GenVisitor;
  typedef GenVisitor<libport::constify_traits> ConstVisitor;
  typedef GenVisitor<libport::id_traits> Visitor;


  // factory.hh.
  class Factory;

  // event-match.hh.
  struct EventMatch;

  typedef boost::unordered_map<libport::Symbol, ast::rExp> modifiers_type;
  typedef std::pair<ast::rExp, ast::rExp> dictionary_elt_type;
  typedef std::vector<dictionary_elt_type> dictionary_elts_type;


} // namespace ast

#endif // !AST_FWD_HH
