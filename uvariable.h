/*! \file uvariable.h
 *******************************************************************************

 File: uvariable.h\n
 Definition of useful types in the URBI server kernel.

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

#ifndef UVARIABLE_H_DEFINED
#define UVARIABLE_H_DEFINED

#include <string>
#include <stdlib.h>
#include "memorymanager/memorymanager.h"
#include <uvalue.h>
#include "ustring.h"
#include "utypes.h"
#include <list>
using std::list;

class UNamedParameters;
class UCommand;
class UCommand_ASSIGN_VALUE;
class UValue;

namespace urbi {
  class UGenericCallback;
};

//! Uvariable is used to store variables
/*! You can pass to the constructor three importants parameters:
    - notifyWrite: when true, the device associated to the variable (if there is
      a device), will be notified by a call to its virtual function notifyWrite() 
      each time the variable has been modified.  
    - notifyRead: when true, the device associated to the variable (if there is
      a device), will be notified by a call to its virtual function notifyRead()
      before any access to the variable value. This can be used to update a sensor value
      only when needed (useful for low bandwith sensor bus). 
    - autoUpdate: when true, assignment are set immediatly, otherwise, they are handled outside
      of the execution loop. This is useful for sensors or devices whose value can be set/read like
      joints: the read value should be the real sensed value, not the assigned one.

      The methods "get" and "set" should be used to get and set the variable value if needed.
 */
class UVariable {
public:
  MEMORY_MANAGED;
  UVariable(const char* name, ufloat val, 
            bool _notifyWrite = false,
            bool _notifyRead = false,
            bool _autoUpdate = true);
  UVariable(const char* name, const char* str, 
            bool _notifyWrite = false,
            bool _notifyRead = false,
            bool _autoUpdate = true);
  UVariable(const char* name, UValue* _value, 
            bool _notifyWrite = false,
            bool _notifyRead = false,
            bool _autoUpdate = true);
  UVariable(const char* _id, const char* _method, UValue* _value, 
            bool _notifyWrite = false,
            bool _notifyRead = false,
            bool _autoUpdate = true);
  UVariable(const char* _id, const char* _method, ufloat val, 
            bool _notifyWrite = false,
            bool _notifyRead = false,
            bool _autoUpdate = true);
  UVariable(const char* _id, const char* _method, const char *str, 
            bool _notifyWrite = false,
            bool _notifyRead = false,
            bool _autoUpdate = true);
  ~UVariable();

  UString         *varname;  ///< full associated var name if it exists
  UString         *method;   ///< method in the varname 
  UString         *devicename; ///< device in the varname

  //properties

  UBlend          blendType; ///< the variable blend type
  UString         *unit;     ///< device unit
  ufloat          rangemin;  ///< rangemin
  ufloat          rangemax;  ///< rangemax
  ufloat          speedmin;  ///< rangemin
  ufloat          speedmax;  ///< rangemax
  ufloat          delta;     ///< delta
  
  int             nbAssigns;///< nb superposition of assignments
  int             nbAverage; ///< nb superposition of mixing or adding assignments
  int             activity;  ///< stage of usage in the reinit list: 0 (off), 1(in) or 2(going out)
  bool            uservar; ///< indicates user variables
  ufloat          target;   ///< temporary value container 
  ufloat          previous,
                  previous2,
                  previous3; ///< previous theoretical value container 
  ufloat          previous_sensed; ///< previous sensed value 
  UValue          *value; ///< variable value
  ufloat          valPrev,
                  valPrev2; // used for 'd and 'dd calculation
  bool            notifyRead; ///< true when UDevice::notifyRead must be called
  bool            notifyWrite;  ///< true when UDevice::notifyWrite must be called
  bool            autoUpdate;  ///< true when the target value is automatically mapped to value
  bool            modified; ///< true when the value of the variable has been modified
  bool            toDelete; ///< mark the variable for deletion
  bool            speedmodified; ///< when a speedmax reajustment has been executed
  bool            reloop; ///< when speedmodified has been detected, asking for a reloop on pending
                          ///< finished assignements.
  UBinder         *binder; ///< cached binder pointer
  
  list<urbi::UGenericCallback*> internalBinder; ///< binder for internal monitors  
  list<urbi::UGenericCallback*> internalAccessBinder; ///< binder for internal access monitors

  UCommand_ASSIGN_VALUE *cancel; ///< used for the "cancel" blend type
  

  const char*   setName(const char *s);
  const char*   setName(const char *_id, const char* _method);
  const char*   setName(UString *s) { return( setName(s->str()) );};

  UVarSet       set(UValue *v);
  UVarSet       setFloat(ufloat f);
  UVarSet       selfSet(ufloat *valcheck);

  void          setSensorVal(ufloat f);
  void          initSensorVal(ufloat f);

  void          updated();

  UValue*       get();
  
  private:

  void    init();

};



//! Set a value->val value. Must be called instead of value->val direct assignment
inline void
UVariable::setSensorVal(ufloat f)
{
  valPrev2 = valPrev;
  valPrev = value->val;
  value->val = f;
}

//! Init a value->val value (valPrev and valPrev2 will be init too)
inline void
UVariable::initSensorVal(ufloat f)
{
  value->dataType = DATA_NUM;
  valPrev2 = f;
  valPrev = f;
  value->val = f;
}

#endif
