#ifndef REWRITE_PATTERN_BINDER_HH
# define REWRITE_PATTERN_BINDER_HH

# include <ast/fwd.hh>
# include <ast/analyzer.hh>

namespace rewrite
{
  class PatternBinder: public ast::Analyzer
  {
  public:
    typedef ast::Analyzer super_type;
    PatternBinder(const ast::rComposite& container);

  protected:
    using super_type::visit;
    CONST_VISITOR_VISIT_NODES(
      (Binding)
      );

  private:
    ast::rComposite container_;
  };
}

#endif
