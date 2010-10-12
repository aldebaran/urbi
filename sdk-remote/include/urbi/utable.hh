/*
 * Copyright (C) 2006-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file urbi/utable.hh
/// \brief Definition of the callback tables.

#ifndef URBI_UTABLE_HH
# define URBI_UTABLE_HH

# include <list>
# include <string>

# include <libport/hash.hh>
# include <libport/safe-container.hh>

# include <urbi/export.hh>
# include <urbi/uvalue.hh>

namespace urbi
{
  // FIXME: There are probably more opportunities for factoring here,
  // fusing these two (three?) classes.

  /*---------.
  | UTable.  |
  `---------*/

  // A few list and hashtable types
  struct URBI_SDK_API UTable
    : boost::unordered_map<std::string,
        libport::SafeContainer< std::list, UGenericCallback*> >
  {
    /// The keys.
    typedef std::string key_type;

    /// The list call backs.
    typedef libport::SafeContainer<std::list, UGenericCallback*> callbacks_type;
    typedef callbacks_type mapped_type;

    /// Type of the super class.
    typedef boost::unordered_map<key_type, mapped_type> super_type;

    /// Iterator types.
    typedef super_type::const_iterator const_iterator;
    typedef super_type::iterator iterator;

    /// Contructor.
    UTable();

    /// Return the list of callbacks, otherwise 0.
    mapped_type* find0(const key_type& name);

    /// Clean a callback UTable from all callbacks linked to the
    /// object whose name is \a name.
    void clean(const key_type& name);
  };

  /*------------.
  | UVarTable.  |
  `------------*/

  struct URBI_SDK_API UVarTable
    : boost::unordered_map<std::string, std::list<UVar*> >
  {
    /// The keys.
    typedef std::string key_type;

    /// The list call backs.
    typedef std::list<UVar*> callbacks_type;
    typedef callbacks_type mapped_type;

    /// Type of the super class.
    typedef boost::unordered_map<key_type, mapped_type> super_type;

    /// Iterator types.
    typedef super_type::const_iterator const_iterator;
    typedef super_type::iterator iterator;


    /// Return the list of callbacks, otherwise 0.
    mapped_type* find0(const key_type& name);

    void clean(const UVar& uvar);
  };

  /*--------------.
  | Timer table.  |
  `--------------*/

  typedef std::list<UTimerCallback*> UTimerTable;

} // end namespace urbi


# ifdef _MSC_VER
// Without this, msvc 2005 fails at link time with:
// unresolved external symbol "public: __thiscall libport::SafeContainer<class std::list,class urbi::UGenericCallback *>::real_value_type::real_value_type(class urbi::UGenericCallback * const &,class libport::SafeContainer<class std::list,class urbi::UGenericCallback *> &)"

template libport::SafeContainer<std::list, urbi::UGenericCallback*>;
# endif

#endif // ! URBI_UTABLE_HH
