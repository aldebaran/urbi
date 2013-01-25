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
 ** \file ast/dot-generator.hh
 ** \brief Declaration of ast::Cloner.
 */

#ifndef AST_DOT_GENERATOR_HH
# define AST_DOT_GENERATOR_HH


# include <list>
# include <sstream>

# include <libport/foreach.hh>

# include <ast/ast.hh>
# include <ast/visitor.hh>
# include <ast/symbols-type.hh>

namespace ast
{
  /// Print an ast in Dot format
  class DotPrinter: public ast::ConstVisitor
  {
  public:
    typedef ast::ConstVisitor super_type;

    DotPrinter(std::ostream& output, const std::string& title = "");
    virtual ~DotPrinter();
    virtual void operator()(const ast::Ast* n);

  private:
    // Import overloaded virtual functions.
    using super_type::visit;
        CONST_VISITOR_VISIT_NODES(
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

    std::ostream& output_;
    unsigned id_;
    std::list<std::pair<unsigned, std::string> > ids_;
    bool root_;
    /// The name of the graph.
    std::string title_;

    template<typename T>
    void recurse(const T& c);

    template<typename T>
    void recurse(const T* c);
  };
}

#endif
