/**
 ** \file ast/cloner.hxx
 ** \brief Template methods of ast::Cloner.
 */

#ifndef AST_CLONER_HXX
# define AST_CLONER_HXX

# include <boost/foreach.hpp>

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
  T*
  Cloner::recurse (const T* const t)
  {
    T* res = 0;
    if (t)
      {
	t->accept (*this);
	res = dynamic_cast<T*> (result_);
	assert (res);
      }
    return res;
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

    typedef typename CollectionType::value_type elt_type;
    BOOST_FOREACH (const elt_type& e, c)
    {
      if (e)
      {
	e->accept (*this);
	elt_type elt = dynamic_cast<elt_type> (result_);
	assert (elt);
	res->push_back (elt);
      }
      else
	res->push_back (0);
    }

    return res;
  }

  // Specializations to workaround some limitations of ast-cloner-gen.
  template <>
  inline
  symbols_type*
  Cloner::recurse_collection<symbols_type> (const symbols_type& c)
  {
    return new symbols_type (c);
  }

  template <>
  inline
  exec_exps_type*
  Cloner::recurse_collection<exec_exps_type> (const exec_exps_type& c)
  {
    exec_exps_type* res = new exec_exps_type;

    BOOST_FOREACH (const Stmt* e, c)
    {
      e->accept (*this);
      Stmt* s = dynamic_cast<Stmt*> (result_);
      assert (s);
      res->push_back (s);
    }

    return res;
  }

} // namespace ast

#endif // !AST_CLONER_HXX
