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
 ** \file ast/pretty-printer.hh
 ** \brief Definition of ast::PrettyPrinter.
 */

#ifndef AST_PRETTY_PRINTER_HH
# define AST_PRETTY_PRINTER_HH

# include <iosfwd>

# include <ast/default-visitor.hh>

namespace ast
{

  /// Ast pretty-printer.
  class PrettyPrinter : public DefaultConstVisitor
  {
    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Super class type.
    typedef DefaultConstVisitor super_type;

    /// Construct a PrettyPrinter.
    PrettyPrinter (std::ostream& s);

    /// Destroy a PrettyPrinter.
    virtual ~PrettyPrinter ();
    /** \} */

    /// Visit entry point
    virtual void operator() (const Ast* e);

  protected:
    using super_type::visit;

    CONST_VISITOR_VISIT_NODES(
                              (And)
                              (Assign)
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
                              (Flavored)
                              (Float)
                              (Foreach)
                              (If)
                              (Implicit)
                              (Incrementation)
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
                              (Unscope)
                              (Watch)
                              (While)
                              (Write)
                             )


  private:
    // The stream we output to.
    std::ostream& ostr_;
  };

} // namespace ast

#endif // !AST_PRETTY_PRINTER_HH

