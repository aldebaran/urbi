/*! \file uvariablename.h
 *******************************************************************************

 File: uvariablename.h\n
 Definition of the UVariableName class.

 This file is part of
 %URBI Kernel, version __kernelversion__\n
 (c) Jean-Christophe Baillie, 2004-2005.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.net

 **************************************************************************** */

#ifndef UVARIABLENAME_H_DEFINED
#define UVARIABLENAME_H_DEFINED

#include <list>

#include "fwd.hh"
#include "utypes.h"
#include "ustring.h"

// ****************************************************************************
//! Contains a variable name description
/*! This class can contain complex names for variables, as returned by the
    parser. Arrays, expression based variables like $(x) are all stored in this
    object and the final name is expanded on request with the "buildFullname"
    method.
    A caching mecanism exist to avoid rebuilding the variable name if it is not
    needed.
*/
class UVariableName
{
public:
  MEMORY_MANAGED;

  UVariableName(UExpression *str, bool rooted = false);
  UVariableName(UString* device, UString *id, bool rooted,
                UNamedParameters *index);
  UVariableName(UString* objname,
                UNamedParameters *index_obj,
                UString* attribute,
                UNamedParameters *index_att = 0);

  virtual ~UVariableName();

  void           print();
  UVariableName* copy();

  UVariable*     getVariable(UCommand *command, UConnection *connection);
  UFunction*     getFunction(UCommand *command, UConnection *connection);
  bool           isFunction(UCommand *command, UConnection *connection);
  UString*       buildFullname(UCommand *command,
                               UConnection *connection,
                               bool withalias = true);
  UString*       getFullname()  { return (fullname_);};
  void           nameUpdate(const char* _device, const char* _id);
  void           resetCache();
  UString*       getDevice();
  UString*       getMethod();

  UString*          device;   ///< First part of a compound variable
  UString*          id;     ///< Variable name (second part of a compound var.)
  UString*          method;   ///< extracted variable name second part.
  bool              rooted;///< true when the compound variable starts with '@'
  UNamedParameters* index;    ///< list of indexes for arrays
  UNamedParameters* index_obj; ///< list of indexes for arrays in object names
  UExpression*      str;      ///< string expression for $ variables
  bool              isstatic; ///< true if the variable is static
  bool              isnormalized; ///< true if the var is in normalized mode
  UDeriveType       deriv;    ///< deriv type for the underlying variable
  bool              varerror; ///< true to request the target-val evaluation
  UVariable*        variable; ///< the variable associated to the variable name
  UFunction*        function; ///< the function associated to the variable name
  bool              fromGroup; ///< the var is part of a command spawned by a
                               ///< group morphing. this is used to avoid error
                               ///< messages when the variable does not exist
  bool         firsttime;///< before the first local function prefix resolution
  bool         nostruct; ///< is nostruct if it comes from a simple
                         ///< IDENTIFIER in the parsing phase
  UDefType          id_type; ///< type of the symbol: UDEF_FUNCTION,
                             ///< UDEF_VAR or UDEF_EVENT
  bool              local_scope; ///< name resolution will be limited to
                                 ///< local scope in functions
  bool              doublecolon; ///< true when the :: construct is used

protected:

  UString*        fullname_; ///< used as a hash key

  HMaliastab::iterator    hmi, past_hmi; ///< internal
  HMvariabletab::iterator hmi2;         ///< internal
  HMfunctiontab::iterator hmf;          ///< internal
  bool                    localFunction; ///< true for variables
                                         ///< local to a func.
  bool                    selfFunction;///< true for variables with self prefix
  bool                    cached;       ///< internal
};

#endif
