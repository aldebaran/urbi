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
 ** \file ast/visitor.hh
 ** \brief Definition of ast::Visitor.
 */

#ifndef AST_VISITOR_HH
# define AST_VISITOR_HH


# include <functional>

# include <boost/preprocessor/array.hpp>
# include <boost/preprocessor/repeat.hpp>
# include <boost/preprocessor/seq/for_each.hpp>

# include <libport/select-const.hh>

# include <ast/fwd.hh>


namespace ast
{

  /*----------------------------------------------------.
  | Boost PP macros to ease the declaration of visits.  |
  `----------------------------------------------------*/

/// Declare an abstract visit method for Elem for a GenVisitor.
/// Also, declare the node as a friend: it is allowed to call visit.
/// It would be nice to declare only Elem::accept, but then we have
/// a mutual recursion problem.  Maybe we can address this from the
/// derived visitors, not from the abstract visitor.
# define ABSTRACT_VISITOR_VISIT_NODE_(R, Data, Elem)                    \
  friend class ast::Elem;                                               \
  virtual void visit (typename Const<ast::Elem>::type* e) = 0;

/// Declare abstract visits for an abstract GenVisitor.
# define ABSTRACT_VISITOR_VISIT_NODES(Nodes)                    \
  BOOST_PP_SEQ_FOR_EACH(ABSTRACT_VISITOR_VISIT_NODE_, ~, Nodes)

/// Required, or for some reason a space is inserted in the concatenation
/// below (tested with gcc and vcxx).
#ifndef CONCAT
# define CONCAT(a,b) a##b
#endif

/// Declare a visit method for Elem for a GenVisitor.
# define GEN_VISITOR_VISIT_NODE_(R, Data, Elem)                   \
    typedef typename Const<ast::Elem>::type CONCAT(Elem,_type);   \
    virtual void visit (CONCAT(Elem,_type)* e);

/// Declare visits for a GenVisitor.
# define GEN_VISITOR_VISIT_NODES(Nodes)                    \
  BOOST_PP_SEQ_FOR_EACH(GEN_VISITOR_VISIT_NODE_, ~, Nodes)

/// Declare a visit method for Elem.  Pass Const = const or not.
# define VISITOR_VISIT_NODE_(R, Const, Elem)            \
    virtual void visit (Const ast::Elem* e);

/// Declare visit methods for Nodes.  Pass Const = const or not.
# define VISITOR_VISIT_NODES_(Nodes, Const)             \
  BOOST_PP_SEQ_FOR_EACH(VISITOR_VISIT_NODE_, Const, Nodes)


  /** \brief Root class of all Ast visitors.
   **
   ** GenVisitor<CONSTIFY> is the root class of all Ast visitors. */
  template <template <typename> class Const>
  class GenVisitor : public std::unary_function<Ast, void>
  {
    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Destroy a GenVisitor.
    virtual ~GenVisitor ();
    /** \} */

    /// This intermediate typedef is required by VCXX 2005
    typedef typename Const<Ast>::type* ast_type;
    /// The entry point: visit \a e.
    virtual void operator() (ast_type e);

  protected:
    ABSTRACT_VISITOR_VISIT_NODES(
                                 (And)
                                 (Assign)
                                 (Assignment)
                                 (At)
                                 (Binding)
                                 (Break)
                                 (Call)
                                 (CallMsg)
                                 (Catch)
                                 (Class)
                                 (Continue)
                                 (Declaration)
                                 (Decrementation)
                                 (Dictionary)
                                 (Do)
                                 (Emit)
                                 (Event)
                                 (Finally)
                                 (Float)
                                 (Foreach)
                                 (If)
                                 (Implicit)
                                 (Incrementation)
                                 (List)
                                 (Local)
                                 (LocalAssignment)
                                 (LocalDeclaration)
                                 (Match)
                                 (MetaArgs)
                                 (MetaCall)
                                 (MetaExp)
                                 (MetaId)
                                 (MetaLValue)
                                 (Nary)
                                 (Noop)
                                 (OpAssignment)
                                 (Pipe)
                                 (Property)
                                 (PropertyWrite)
                                 (Return)
                                 (Routine)
                                 (Scope)
                                 (Stmt)
                                 (String)
                                 (Subscript)
                                 (TaggedStmt)
                                 (This)
                                 (Throw)
                                 (Try)
                                 (Unscope)
                                 (Watch)
                                 (While)
                                )


  };

  /*----------.
  | Visitor.  |
  `----------*/

  /// Shorthand for a non const visitor.
  typedef GenVisitor<libport::id_traits> Visitor;

  /// Declare visits for a non-const visitor.
# define VISITOR_VISIT_NODES(Nodes)                     \
    VISITOR_VISIT_NODES_(Nodes, )


  /*---------------.
  | ConstVisitor.  |
  `---------------*/

  /// Shorthand for a const visitor.
  typedef GenVisitor<libport::constify_traits> ConstVisitor;

  /// Declare visits for a const visitor.
# define CONST_VISITOR_VISIT_NODES(Nodes)               \
    VISITOR_VISIT_NODES_(Nodes, const)


} // namespace ast

# include <ast/visitor.hxx>

#endif // !AST_VISITOR_HH
