/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef FLOWER_FLOWER_HH
# define FLOWER_FLOWER_HH

# include <ast/analyzer.hh>
# include <ast/loc.hh>

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
			      (Catch)
			      (Continue)
			      (Foreach)
			      (Return)
			      (Routine)
			      (Throw)
			      (Try)
			      (While));

  private:
    void err(const ast::loc& loc, const std::string& msg);
    bool has_break_;
    bool has_continue_;
    bool has_general_catch_;
    bool has_return_;
    bool in_catch_;
    bool in_function_;
    bool in_loop_;
    unsigned int catch_all_;
  };

} // namespace flower

#endif // FLOWER_FLOWER_HH
