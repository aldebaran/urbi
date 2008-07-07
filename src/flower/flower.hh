#ifndef FLOWER_FLOWER_HH
# define FLOWER_FLOWER_HH

# include <ast/analyzer.hh>

namespace flower
{

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
  };

} // namespace flower

#endif // FLOWER_FLOWER_HH
