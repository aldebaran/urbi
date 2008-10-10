/**
 ** \file ast/cloner.hxx
 ** \brief Template methods of ast::Cloner.
 */

#ifndef AST_CLONER_HXX
# define AST_CLONER_HXX

# include <libport/foreach.hh>

# include <ast/print.hh>
# include <ast/cloner.hh>

namespace ast
{

  template <typename T>
  libport::shared_ptr<T>
  Cloner::recurse(libport::shared_ptr<const T> t)
  {
    operator()(t.get());
    libport::shared_ptr<T> res = result_.unsafe_cast<T>();
    // We do have situations where t is not null, but result_ is, for
    // instance when we process a ParametricAst which substitutes a
    // MetaVar for a target, and the effective target is 0.
    //
    // FIXME: We should stop accepting 0 in our tree, this is really
    // painful and dangerous.
    passert(*t, !result_ || res);
    return res;
  }

  template <typename T>
  libport::shared_ptr<T>
  Cloner::recurse (libport::shared_ptr<T> t)
  {
    libport::shared_ptr<const T> other = t;
    return t ? recurse(other) : libport::shared_ptr<T>(0);
  }

  template <typename CollectionType>
  CollectionType*
  Cloner::maybe_recurse_collection(const CollectionType* c)
  {
    return c ? new CollectionType(recurse_collection(*c)) : 0;
  }

  template <typename CollectionType>
  CollectionType
  Cloner::recurse_collection (const CollectionType& c)
  {
    CollectionType res;
    foreach (typename CollectionType::const_reference e, c)
      res.push_back(recurse(e));
    return res;
  }

  // Specializations to workaround some limitations of ast-cloner-gen.
  template <>
  inline
  symbols_type
  Cloner::recurse_collection<symbols_type> (const symbols_type& c)
  {
    return c;
  }

  template <>
  inline
  exps_type
  Cloner::recurse_collection<exps_type> (const exps_type& c)
  {
    exps_type res;
    // We cannot use the container's clone feature, since it uses the
    // stock AST cloner and none of its specializations, such as the
    // very needed override from ParametricAst.  As a result, we clone
    // the meta-ast instead of substituting the meta-vars with their
    // actual values.
    foreach (rExp e, c)
      res.push_back(recurse(e));
    return res;
  }

  template <>
  inline
  modifiers_type
  Cloner::recurse_collection<modifiers_type> (const modifiers_type& modifiers)
  {
    modifiers_type res;

    foreach (const modifiers_type::value_type& m, modifiers)
      res[m.first] = recurse(m.second);

    return res;
  }

} // namespace ast

#endif // !AST_CLONER_HXX
