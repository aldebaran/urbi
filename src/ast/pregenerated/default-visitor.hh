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
 ** \file ast/default-visitor.hh
 ** \brief Definition of ast::DefaultVisitor.
 */

#ifndef AST_DEFAULT_VISITOR_HH
# define AST_DEFAULT_VISITOR_HH


# include <functional>
# include <ostream>

# include <libport/select-const.hh>

# include <ast/visitor.hh>


namespace ast
{

  /** \brief Just visit the whole Ast tree.
   **
   ** GenDefaultVisitor<CONSTNESS-SELECTOR> visits the whole Ast tree,
   ** but does nothing else. */
  template < template<typename> class Const >
  class GenDefaultVisitor : public GenVisitor<Const>
  {
  public:
    /// Super class type.
    typedef GenVisitor<Const> super_type;

    /** \name Ctor & dtor.
     ** \{ */
    /// Construct a Default Visitor.
    GenDefaultVisitor ();

    /// Destroy a Default Visitor.
    virtual ~GenDefaultVisitor ();
    /** \} */

    // Import from super class.
    using super_type::visit;

  protected:
    /// \name Visit nodes.
    /// \{

    GEN_VISITOR_VISIT_NODES(
                            (And)
                            (Assign)
                            (Assignment)
                            (Ast)
                            (At)
                            (Binding)
                            (Break)
                            (Call)
                            (CallMsg)
                            (Catch)
                            (Class)
                            (Composite)
                            (Continue)
                            (Declaration)
                            (Decrementation)
                            (Dictionary)
                            (Do)
                            (Emit)
                            (Event)
                            (Exp)
                            (Finally)
                            (Flavored)
                            (Float)
                            (Foreach)
                            (If)
                            (Implicit)
                            (Incrementation)
                            (LValue)
                            (LValueArgs)
                            (List)
                            (Local)
                            (LocalAssignment)
                            (LocalDeclaration)
                            (LocalWrite)
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
                            (PropertyAction)
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
                            (Unary)
                            (Unscope)
                            (Watch)
                            (While)
                            (Write)
                           )

    /// \}
  };

  /// Shorthand for a const visitor.
  typedef GenDefaultVisitor<libport::constify_traits> DefaultConstVisitor;
  /// Shorthand for a non const visitor.
  typedef GenDefaultVisitor<libport::id_traits> DefaultVisitor;

} // namespace ast

# include <ast/default-visitor.hxx>

#endif // !AST_DEFAULT_VISITOR_HH

