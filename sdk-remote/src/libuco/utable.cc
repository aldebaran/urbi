/*
 * Copyright (C) 2007-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file libuco/utable.cc

#include <libport/foreach.hh>

#include <urbi/ucallbacks.hh>
#include <urbi/utable.hh>
#include <urbi/uvar.hh>

// Lists and hashtables used.

namespace urbi
{

  namespace
  {
    // FIXME: Use libport::find0 which first needs to be educated to
    // support the case where the mapped_type is not a pointer type.
    template <typename Container>
    inline
    typename Container::mapped_type*
    find0(Container& c, const typename Container::key_type& k)
    {
      typename Container::iterator i = c.find(k);
      if (i != c.end())
        return &i->second;
      else
        return 0;
    }
  }


  /*---------.
  | UTable.  |
  `---------*/

  UTable::UTable()
  {
  }

  UTable::mapped_type*
  UTable::find0(const std::string& name)
  {
    return urbi::find0(*this, name);
  }

  //! Clean a callback UTable from all callbacks linked to the
  //! object whose name is 'name'
  void
  UTable::clean(const std::string& name)
  {
    std::list<iterator> todelete;
    for (UTable::iterator i = begin(); i != end(); ++i)
    {
      mapped_type& cs = i->second;
      for (mapped_type::iterator j = cs.begin(); j != cs.end(); ++j)
      {
	if ((*j)->objname == name)
	{
	  delete *j;
	  cs.erase(j);
	}
      }

      if (cs.empty())
	todelete.push_back(i);
    }

    foreach (iterator& i, todelete)
      erase(i);
  }


  /*------------.
  | UVarTable.  |
  `------------*/

  UVarTable::mapped_type*
  UVarTable::find0(const std::string& name)
  {
    return urbi::find0(*this, name);
  }

  //! Clean a callback UTable from all callbacks linked to the
  //! object whose name is 'name'
  void
  UVarTable::clean(const UVar& uvar)
  {
    iterator i = find(uvar.get_name());
    if (i != end())
    {
      for (mapped_type::iterator j = i->second.begin();
           j != i->second.end();)
        if (*j == &uvar)
          j = i->second.erase(j);
        else
          ++j;

      if (i->second.empty())
        erase(i);
    }
  }

} // namespace urbi
