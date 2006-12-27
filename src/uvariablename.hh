/*! \file uvariablename.hh
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

#ifndef UVARIABLENAME_HH
# define UVARIABLENAME_HH

# include <list>

# include "fwd.hh"
# include "utypes.hh"
# include "ustring.hh"

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

  void           print() const;
  UVariableName* copy() const;

  UVariable*     getVariable (UCommand* command, UConnection* connection);
  UFunction*     getFunction(UCommand *command, UConnection *connection);
  bool           isFunction(UCommand *command, UConnection *connection);
  UString*       buildFullname(UCommand* command, UConnection* connection,
			       bool withalias = true);
  UString*       getFullname ()
  {
    return fullname_;
  }
  void           nameUpdate(const char* _device, const char* _id);
  void           resetCache();
  UString*       getDevice();
  UString*       getMethod();

  /// First part of a compound variable.
  UString*          device;
  /// Variable name (second part of a compound var.).
  UString*          id;
  /// Extracted variable name second part..
  UString*          method;
  /// True when the compound variable starts with '@'.
  bool              rooted;
  /// List of indexes for arrays.
  UNamedParameters* index;
  /// List of indexes for arrays in object names.
  UNamedParameters* index_obj;
  /// String expression for $ variables.
  UExpression*      str;
  /// True if the variable is static.
  bool              isstatic;
  /// True if the var is in normalized mode.
  bool              isnormalized;
  /// Deriv type for the underlying variable.
  UDeriveType       deriv;
  /// True to request the target-val evaluation.
  bool              varerror;
  /// True to request the target evaluation.
  bool              varin;
  /// The variable associated to the variable name.
  UVariable*        variable;
  /// The function associated to the variable name.
  UFunction*        function;
  /// The var is part of a command spawned by a.  group morphing. this
  /// is used to avoid error messages when the variable does not exist
  bool              fromGroup;

  /// Before the first local function prefix resolution.
  bool         firsttime;
  /// Is nostruct if it comes from a simple.  IDENTIFIER in the
  /// parsing phase
  bool         nostruct;

  /// Type of the symbol: UDEF_FUNCTION, UDEF_VAR or UDEF_EVENT.
  UDefType          id_type;
  /// Name resolution will be limited to local scope in functions.
  bool              local_scope;
  /// True when the :: construct is used.
  bool              doublecolon;

protected:

  /// Used as a hash key.
  UString*		fullname_;
  /// True for variables local to a func..
  bool			localFunction;
  /// True for variables with self prefix.
  bool			selfFunction;
  /// Internal.
  bool			cached;
};

#endif
