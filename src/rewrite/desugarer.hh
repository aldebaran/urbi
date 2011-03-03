/*
 * Copyright (C) 2008-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef REWRITE_DESUGAR_HH
# define REWRITE_DESUGAR_HH

# include <memory>
# include <boost/type_traits/remove_const.hpp>

# include <ast/analyzer.hh>
# include <ast/factory.hh>
# include <ast/loc.hh>

namespace rewrite
{
  /// Desugar complex constructs to the core language
  class Desugarer: public ast::Analyzer
  {
  public:
    /// \name Useful shorthands.
    /// \{
    /// Super class type.
    typedef ast::Analyzer super_type;
    /// \}

    Desugarer();
    virtual void operator()(const ast::Ast* node);

    /// Recurse in \a t, but allow declarations in its children.
    template <typename T>
    libport::intrusive_ptr<typename boost::remove_const<T>::type>
    recurse_with_subdecl(T* t);

    template <typename T>
    libport::intrusive_ptr<typename boost::remove_const<T>::type>
    recurse_with_subdecl(libport::intrusive_ptr<T> t);

  protected:
    /// Import visit from DefaultVisitor.
    using super_type::visit;
    /// Nodes to desugar
    CONST_VISITOR_VISIT_NODES(
      (At)
      (And)
      (Assign)
      (Binding)
      (Catch)
      (Class)
      (Decrementation)
      (Do)
      (Emit)
      (Incrementation)
      (If)
      (OpAssignment)
      (Pipe)
      (Scope)
      (Stmt)
      (Subscript)
      (Try)
      (While)
      );
    void visit_dincrementation(ast::rLValue what, libport::Symbol meth);

  private:
    /// Report error
    void err(const ast::loc& loc, const std::string& msg);

    void desugar_modifiers(const ast::Assign* assign);

    bool pattern_;
    /// Whether Declarations are allowed in the current node.
    bool allow_decl_;
    /// Whether Declarations are allowed in children.
    bool allow_subdecl_;
  };
}

#endif
