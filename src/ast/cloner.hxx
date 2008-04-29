/**
 ** \file ast/cloner.hxx
 ** \brief Template methods of ast::Cloner.
 */

#ifndef AST_CLONER_HXX
# define AST_CLONER_HXX

# include <libport/foreach.hh>

# include "ast/cloner.hh"

namespace ast
{

  template <typename T>
  T*
  Cloner::recurse (const T& t)
  {
    t.accept (*this);
    T* res = dynamic_cast<T*> (result_);
    assert (res);
    return res;
  }

  template <typename T>
  typename boost::remove_const<T>::type*
  Cloner::recurse (T* t)
  {
    return t ? recurse(*t) : 0;
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
    symbols_type* ret = new symbols_type();
    foreach(libport::Symbol* s, c)
      ret->push_back(new libport::Symbol(*s));
    return ret;
  }

} // namespace ast

#endif // !AST_CLONER_HXX
