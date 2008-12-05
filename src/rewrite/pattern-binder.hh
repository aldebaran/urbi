#ifndef REWRITE_PATTERN_BINDER_HH
# define REWRITE_PATTERN_BINDER_HH

# include <ast/fwd.hh>
# include <ast/cloner.hh>

namespace rewrite
{
  class PatternBinder: public ast::Cloner
  {
  public:
    typedef ast::Cloner super_type;
    PatternBinder(ast::rLValue pattern, const ast::loc& loc);
    ast::rPipe bindings_get() const;
    ast::rExp value_get();

  protected:
    using super_type::visit;
    CONST_VISITOR_VISIT_NODES(
      (Binding)
      (Call)
      (Property)
      (Subscript)
      );

  private:
    libport::Symbol next_name();
    ast::rExp to_binding(ast::rConstExp original);
    void bind(ast::rConstLValue what, bool decl);
    ast::rPipe bindings_;
    ast::rPipe declarations_;
    ast::rLValue pattern_;
    int i_;
  };
}

#endif
