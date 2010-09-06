/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file ast/parametric-ast.hh
 ** \brief Declaration of ast::ParametricAst.
 */

#ifndef AST_PARAMETRIC_AST_HH
# define AST_PARAMETRIC_AST_HH

# include <boost/preprocessor/array.hpp>
# include <boost/preprocessor/repeat.hpp>
# include <boost/preprocessor/seq/for_each.hpp>

# include <libport/symbol.hh>
# include <libport/unique-pointer.hh>

# include <ast/cloner.hh>
# include <ast/exp.hh>
# include <ast/lvalue.hh>

# include <parser/metavar-map.hh>

# define URBI_PARAMETERIZED_AST_TYPES           \
  ((ast::rExp, exp))                            \
  ((libport::Symbol, id))                       \
  ((ast::exps_type*, exps))

# define URBI_PARAMETERIZED_AST_FOREACH(Macro)                  \
  BOOST_PP_SEQ_FOR_EACH(Macro, , URBI_PARAMETERIZED_AST_TYPES)

namespace ast
{

  /*----------------.
  | ParametricAst.  |
  `----------------*/

  class ParameterizedAst;

  class ParametricAst
  {
  public:
    /// Build a ParametricAst whose textual part is \a s.
    ParametricAst(const char* s, const loc& l = loc(),
                  bool desugar = false);
    /// Destroy the ParametricAst.
    /// \precondition empty()
    virtual ~ParametricAst();

    /// Pass the n-th argument.
    // The template version of the operator fails with an
    // incomprehensible ambiguity. It's duplicated for now.
# define PERCENT(Iter, None, Type)              \
    ParameterizedAst& operator% (BOOST_PP_TUPLE_ELEM(2, 0, Type));
  URBI_PARAMETERIZED_AST_FOREACH(PERCENT)
# undef PERCENT

    /// Return the Ast itself.
    const Ast* get() const;

  private:
    /// The ast, possibly with meta-variables.
    rConstAst ast_;
  };

  /*-------------------.
  | ParameterizedAst.  |
  `-------------------*/

  class ParameterizedAst
    : public Cloner

  // Inherit from MetavarMap of all types
# define INHERIT(Iter, None, Type)                                      \
  , public parser::MetavarMap<BOOST_PP_TUPLE_ELEM(2, 0, Type)>
  URBI_PARAMETERIZED_AST_FOREACH(INHERIT)
# undef INHERIT

  {
  public:
    typedef Cloner super_type;

    // Typedef MetavarMap of all types
#define TYPEDEF(Iter, None, Type)                               \
    typedef parser::MetavarMap<BOOST_PP_TUPLE_ELEM(2, 0, Type)> \
      BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(2, 1, Type), _map_type);
    URBI_PARAMETERIZED_AST_FOREACH(TYPEDEF)
#undef TYPEDEF

    /// Build a ParameterizedAst whose textual part is \a s.
    ParameterizedAst(const ParametricAst& s);
    /// Destroy the ParameterizedAst.
    /// \precondition empty()
    virtual ~ParameterizedAst();

    /// Pass the n-th argument.
    // The template version of the operator fails with an
    // incomprehensible ambiguity. It's duplicated for now.
# define PERCENT(Iter, None, Type)              \
    ParameterizedAst& operator% (BOOST_PP_TUPLE_ELEM(2, 0, Type));
  URBI_PARAMETERIZED_AST_FOREACH(PERCENT)
# undef PERCENT

    /// Fire the substitution, and return the result.
    /// Calls clear.
    template <typename T>
    libport::intrusive_ptr<T> result();

    /// Dump the master AST and the state of the tables.
    std::ostream& dump(std::ostream& o) const;

    /// Clear a possibly partially filled AST and prepare
    /// it for a new use. This method must be used when
    /// the AST filling may have been aborted before the
    /// result has been get.
    void clear();

  protected:
    /// Import from super.
    using super_type::visit;
    CONST_VISITOR_VISIT_NODES(
      (MetaArgs)
      (MetaCall)
      (MetaExp)
      (MetaId)
      (MetaLValue)
      );

    /// Metavariables manipulator.
    template <typename T> T take (unsigned s) throw (std::range_error);

  private:
    /// The ast.
    const ParametricAst& ast_;

    /// Check that the tables are empty.
    bool empty() const;

    /// Reset for a new instantiation (the ast is kept).
    /// \precondition empty()
    void reset();

    /// The location, based on provided arguments.
    loc effective_location_;

    /// The slot number for the next effective argument, starting at 0.
    unsigned count_;

# ifndef NDEBUG
    /// The set of pointers that must be unique.
    libport::UniquePointer unique_;
# endif
  };


  /*--------------------------.
  | Free standing functions.  |
  `--------------------------*/

  /// Convenience wrapper around ParameterizedAst::result<Exp>.
  rExp exp (ParameterizedAst& t);

  /// Dump \a a on \a o.
  /// For debugging.
  std::ostream& operator<< (std::ostream& o, const ParameterizedAst& a);

} // namespace ast


/// Define a parameterized ast, using the current file and line as location.
# define PARAMETRIC_AST_HELPER(Name, Content, Desugar)          \
  static ::ast::ParametricAst                                   \
  Name ## _Ast(Content, LOCATION_HERE, Desugar);                \
  ::ast::ParameterizedAst Name(Name ## _Ast);

# define PARAMETRIC_AST(Name, Content)          \
  PARAMETRIC_AST_HELPER(Name, Content, false)

# define PARAMETRIC_AST_DESUGAR(Name, Content)  \
  PARAMETRIC_AST_HELPER(Name, Content, true)

# include <ast/parametric-ast.hxx>

#endif // !AST_PARAMETRIC_AST_HH
