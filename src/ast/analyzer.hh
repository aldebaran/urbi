/**
 ** \file ast/analyzer.hh
 ** \brief Declaration of ast::Analyzer.
 */

#ifndef AST_ANALYZER_HH
# define AST_ANALYZER_HH

# include <ast/cloner.hh>
# include <ast/error.hh>

namespace ast
{

  /// \brief Duplicate an Ast.
  class Analyzer : public ast::Cloner
  {
  public:
    typedef ast::DefaultConstVisitor super_type;

    /// The errors seen so far.
    ast::Error& errors_get();

  protected:
    /// The errors found so far.
    ast::Error errors_;
  };

  /// Apply a visitor on a.
  /// Handle errors.
  rExp analyze(Analyzer& v, rConstAst a);

} // namespace ast

#endif // !AST_ANALYZER_HH
