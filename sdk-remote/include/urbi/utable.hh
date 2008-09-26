/* \file urbi/utable.hh
 *******************************************************************************

 Definition of the callback tables.

 This file is part of UObject Component Architecture\n
 Copyright (c) 2006, 2007, 2008 Gostai S.A.S.

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

# include <libport/singleton-ptr.hh>
# include <libport/hash.hh>

# include <urbi/export.hh>
# include <urbi/uvalue.hh>

namespace urbi
{

  // A few list and hashtable types
  typedef libport::hash_map_type<std::string,
				 std::list<UGenericCallback*> >::type UTable;

  //! Clean a callback UTable from all callbacks linked to the
  //! object whose name is 'name'.
  void cleanTable(UTable &t, const std::string& name);

  // Lists and hashtables used.
  EXTERN_STATIC_INSTANCE_EX(UTable, accessmap, USDK_API);
  EXTERN_STATIC_INSTANCE_EX(UTable, eventendmap, USDK_API);
  EXTERN_STATIC_INSTANCE_EX(UTable, eventmap, USDK_API);
  EXTERN_STATIC_INSTANCE_EX(UTable, functionmap, USDK_API);
  EXTERN_STATIC_INSTANCE_EX(UTable, monitormap, USDK_API);

  typedef libport::hash_map_type<std::string, std::list<UVar*> >::type UVarTable;
  EXTERN_STATIC_INSTANCE_EX(UVarTable, varmap, USDK_API);

} // end namespace urbi

#endif // ! URBI_UTABLE_HH
