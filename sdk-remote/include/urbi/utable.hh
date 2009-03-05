/* \file urbi/utable.hh
 *******************************************************************************

 Definition of the callback tables.

 This file is part of UObject Component Architecture\n
 Copyright (c) 2006-2009 Gostai S.A.S.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.com

 **************************************************************************** */

#ifndef URBI_UTABLE_HH
# define URBI_UTABLE_HH

# include <list>
# include <string>

# include <libport/hash.hh>

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
    : libport::hash_map_type<std::string,
                             std::list<UGenericCallback*> >::type
  {
    /// The keys.
    typedef std::string key_type;

    /// The list call backs.
    typedef std::list<UGenericCallback*> callbacks_type;
    typedef callbacks_type mapped_type;

    /// Type of the super class.
    typedef libport::hash_map_type<key_type, mapped_type>::type
      super_type;

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

  // Lists and hashtables used.
  URBI_SDK_API UTable& accessmap();
  URBI_SDK_API UTable& eventendmap();
  URBI_SDK_API UTable& eventmap();
  URBI_SDK_API UTable& functionmap();
  URBI_SDK_API UTable& monitormap();


  /*------------.
  | UVarTable.  |
  `------------*/

  struct URBI_SDK_API UVarTable
    : libport::hash_map_type<std::string, std::list<UVar*> >::type
  {
    /// The keys.
    typedef std::string key_type;

    /// The list call backs.
    typedef std::list<UVar*> callbacks_type;
    typedef callbacks_type mapped_type;

    /// Type of the super class.
    typedef libport::hash_map_type<key_type, mapped_type>::type
      super_type;

    /// Iterator types.
    typedef super_type::const_iterator const_iterator;
    typedef super_type::iterator iterator;


    /// Return the list of callbacks, otherwise 0.
    mapped_type* find0(const key_type& name);

    void clean(const UVar& uvar);
  };
  URBI_SDK_API UVarTable& varmap();


  /*--------------.
  | Timer table.  |
  `--------------*/

  typedef std::list<UTimerCallback*> UTimerTable;
  URBI_SDK_API UTimerTable& timermap();

} // end namespace urbi

#endif // ! URBI_UTABLE_HH
