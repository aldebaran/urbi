#ifndef REWRITE_DESUGAR_HH
# define REWRITE_DESUGAR_HH

# include <boost/type_traits/remove_const.hpp>

# include <ast/analyzer.hh>

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
      (And)
      (Assign)
      (Binding)
      (Class)
      (Decrementation)
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

  private:
    void desugar_modifiers(const ast::Assign* assign);

    bool pattern_;
    bool allow_decl_;
    bool allow_subdecl_;
  };
}

#endif
