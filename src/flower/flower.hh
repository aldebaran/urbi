#ifndef FLOWER_FLOWER_HH
# define FLOWER_FLOWER_HH

# include <ast/analyzer.hh>

namespace flower
{

  /// Transform goto-like control-flow structures into tag-based code.
  ///
  /// The following syntactic constructs are eliminated:
  /// - "break", "continue" (which impacts "while" and "foreach")
  /// - "return" (which impacts "function", not "closure").
  class Flower : public ast::Analyzer
  {
  public:
    typedef ast::Analyzer super_type;
    using super_type::visit;

    Flower();

  protected:
    CONST_VISITOR_VISIT_NODES((Break)
			      (Continue)
			      (Foreach)
			      (Function)
			      (Return)
			      (While));

  private:
    bool has_break_;
    bool has_continue_;
    bool has_return_;
    bool in_function_;
    bool in_loop_;
    unsigned int catch_all_;
  };

} // namespace flower

#endif // FLOWER_FLOWER_HH
