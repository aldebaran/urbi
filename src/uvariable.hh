/*! \file uvariable.hh
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

#ifndef UVARIABLE_HH
#define UVARIABLE_HH

#include <string>
#include <list>

#include <cstdlib>

#include "memorymanager/memorymanager.hh"
#include "fwd.hh"
#include "utypes.hh"
#include "uasyncregister.hh"

//! Uvariable is used to store variables
/* ! You can pass to the constructor three importants parameters:

     - notifyWrite: when true, the device associated to the variable
       (if there is a device), will be notified by a call to its
       virtual function notifyWrite() each time the variable has been
       modified.

     - notifyRead: when true, the device associated to the variable
       (if there is a device), will be notified by a call to its
       virtual function notifyRead() before any access to the variable
       value. This can be used to update a sensor value only when
       needed (useful for low bandwith sensor bus).

     - autoUpdate: when true, assignment are set immediatly,
       otherwise, they are handled outside of the execution loop. This
       is useful for sensors or devices whose value can be set/read
       like joints: the read value should be the real sensed value,
       not the assigned one.

       The methods get() and set() should be used to get and set the
       variable value if needed.  */
class UVariable : public UASyncRegister
{
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

  /// Full associated var name if it exists.
  UString         *varname;
  /// Method in the varname.
  UString         *method;
  /// Device in the varname.
  UString         *devicename;

  //properties

  /// The variable blend type.
  UBlendType blendType;
  /// Device unit.
  UString         *unit;
  ufloat          rangemin;
  ufloat          rangemax;
  ufloat          speedmin;
  ufloat          speedmax;
  ufloat          delta;

  /// Nb superposition of assignments.
  int             nbAssigns;
  /// Nb superposition of mixing or adding assignments.
  int             nbAverage;
  /// Stage of usage in the reinit list: 0 (off), 1(in) or 2(going
  /// out).
  int             activity;
  /// Nb of UVar pointing to this UVariable
  int             useCpt;
  /// Indicates user variables.
  bool            uservar;
  /// Temporary value container.
  ufloat          target;
  ufloat          previous,
		  previous2,
		  previous3; ///< previous theoretical value container
  /// Sensed value at the beginning of the cycle.
  ufloat          cyclevalue;
  /// Time of the last cycle beginning  (used to update cyclevalue)
  ufloat          cycleBeginTime;
  /// Variable value.
  UValue          *value;
  ufloat          valPrev,
		  valPrev2; // used for 'd and 'dd calculation
  /// True when UDevice::notifyRead must be called.
  bool            notifyRead;
  /// True when UDevice::notifyWrite must be called.
  bool            notifyWrite;
  /// True when the target value is automatically mapped to value.
  bool            autoUpdate;
  /// True when the value of the variable has been modified.
  bool            modified;
  /// Mark the variable for deletion.
  bool            toDelete;
  /// When a speedmax reajustment has been executed.
  bool            speedmodified;
  /// When speedmodified has been detected, asking for a reloop on
  /// pending finished assignements.
  bool            reloop;

  /// Cached binder pointer.
  UBinder         *binder;

  /// Binder for internal monitors.
  std::list<urbi::UGenericCallback*> internalBinder;
  /// Binder for internal access monitors.
  std::list<urbi::UGenericCallback*> internalAccessBinder;

  /// Is the variable on the access_and_change_varlist ?
  bool access_and_change;

  /// Used for the "cancel" blend type.
  UCommand_ASSIGN_VALUE *cancel;

  const char*   setName(const char *s);
  const char*   setName(const char *_id, const char* _method);
  const char*   setName(UString *s);

  /// \name Updates.
  /// \{
  /// Return code for variable Update
  enum UVarSet
    {
      UOK,
      USPEEDMAX
    };

  UVarSet set(UValue *v);
  UVarSet setFloat(ufloat f);
  UVarSet selfSet(ufloat *valcheck);
  /// \}

  ///  Set a value->val value.
  ///
  /// Must be called instead of value->valdirect assignment.
  void setSensorVal(ufloat f);

  /// Init a value->val value (valPrev and valPrev2 will be init too).
  void initSensorVal(ufloat f);

  /// True when the variable does not contain an object with
  /// subclasses.
  bool isDeletable();

  void          updated(bool uvar_assign = false);

  UValue*       get();

  void setContext(UCallid * ctx)  {context = ctx;}

  private:

  /// context if scope is a function
  UCallid * context;

  void    init();
};

#endif
