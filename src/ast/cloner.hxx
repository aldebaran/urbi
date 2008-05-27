/**
 ** \file ast/cloner.hxx
 ** \brief Template methods of ast::Cloner.
 */

#ifndef AST_CLONER_HXX
# define AST_CLONER_HXX

# include <libport/foreach.hh>

# include "ast/print.hh"
# include "ast/cloner.hh"

namespace ast
{

  template <typename T>
  libport::shared_ptr<T>
  Cloner::recurse(libport::shared_ptr<const T> t)
  {
    t->accept(*this);
    libport::shared_ptr<T> res = result_.unsafe_cast<T>();
    // We do have situations where t is not null, but result_ is, for
    // instance when we process a ParametricAst which substitutes a
    // MetaVar for a target, and the effective target is 0.
    //
    // FIXME: We should stop accepting 0 in our tree, this is really
    // painful and dangerous.
    passert(t, !result_ || res);
    return res;
  }

  template <typename T>
  libport::shared_ptr<T>
  Cloner::recurse (libport::shared_ptr<T> t)
  {
    libport::shared_ptr<const T> other = t;
    return t ? recurse(other) : libport::shared_ptr<T>(0);
  }

  // args_type can include 0 in its list of args to denote the default
  // target.  So use an iteration that is OK with 0 in arguments.
  // FIXME: Maybe by default we should reject 0, and overload for the
  // specific case of args_type.
  template <typename CollectionType>
  CollectionType*
  Cloner::recurse_collection (const CollectionType& c)
  {
    CollectionType* res = new CollectionType;
    foreach (typename CollectionType::const_reference e, c)
      res->push_back(recurse(e));
    return res;
  }

  // Specializations to workaround some limitations of ast-cloner-gen.
  template <>
  inline
  symbols_type*
  Cloner::recurse_collection<symbols_type> (const symbols_type& c)
  {
    return new symbols_type(c);
  }

  template <>
  inline
  exps_type*
  Cloner::recurse_collection<exps_type> (const exps_type& c)
  {
    exps_type* res = new exps_type;
    // We cannot use the container's clone feature, since it uses the
    // stock AST cloner and none of its specializations, such as the
    // very needed override from ParametricAst.  As a result, we clone
    // the meta-ast instead of substituting the meta-vars with their
    // actual values.
    foreach (rExp e, c)
      res->push_back(recurse(e));
    return res;
  }

} // namespace ast

#endif // !AST_CLONER_HXX
