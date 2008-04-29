/**
 ** \file ast/parametric-ast.hh
 ** \brief Declaration of ast::ParametricAst.
 */

#ifndef AST_PARAMETRIC_AST_HH
# define AST_PARAMETRIC_AST_HH

# include <libport/unique-pointer.hh>

# include "ast/cloner.hh"
# include "ast/exp.hh"

# include "parser/metavar-map.hh"

namespace ast
{

  class ParametricAst
    : public Cloner
    , public parser::MetavarMap<ast::Exp>
  {
  public:
    typedef parser::MetavarMap<ast::Exp> exp_map_type;

    ParametricAst(const std::string& s);
    virtual ~ParametricAst();

    /// Pass the n-th argument.
    template <typename T>
    ParametricAst& operator% (const T& t);

    /// Fire the substitution, and return the result.
    /// Calls clear.
    template <typename T>
    T* result();

    /// Reset for a new instantiation (the ast is kept).
    /// Check that the tables are empty.
    void clear();

    /// Dump the master AST and the state of the tables.
    std::ostream& dump(std::ostream& o) const;

  protected:
    /// Import from super.
    using super_type::operator();
    virtual void operator() (const MetaExp& e);
    /// \}

    /// Metavariables manipulator.
    template <typename T> T* take (unsigned s) throw (std::range_error);

  private:
    /// The ast, possibly with meta-variables.
    const Ast* ast_;

    /// The slot number for the next effective argument, starting at 0.
    unsigned count_;

# ifndef NDEBUG
    /// The set of pointers that must be unique.
    libport::UniquePointer unique_;
# endif
  };

  /// Convenience wrapper around ParametricAst::result<Exp>.
  Exp* exp (ParametricAst& t);

  /// Dump \a a on \a o.
  /// For debugging.
  std::ostream& operator<< (std::ostream& o, const ParametricAst& a);

} // namespace ast

# include "ast/parametric-ast.hxx"

#endif // !AST_PARAMETRIC_AST_HH
