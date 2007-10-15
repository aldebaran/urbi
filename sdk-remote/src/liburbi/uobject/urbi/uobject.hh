/*! \file urbi/uobject.hh
 *******************************************************************************

 File: uobject.hh\n
 Definition of the UObject class and necessary related classes.

 This file is part of UObject Component Architecture\n
 (c) 2006 Gostai S.A.S.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.com

 **************************************************************************** */

#ifndef URBI_UOBJECT_HH
# define URBI_UOBJECT_HH

# include <string>

# include "libport/ufloat.h"

# include "urbi/fwd.hh"
# include "urbi/ucallbacks.hh"
# include "urbi/utypes-common.hh"
# include "urbi/uvar.hh"

// Tell our users that it is fine to use void returning functions.
#define USE_VOID 1

/** Bind a variable to an object.

 This macro can only be called from within a class inheriting from
 UObject.  It binds the UVar x within the object to a variable
 with the same name in the corresponding URBI object.  */
# define UBindVar(obj,x) x.init(__name,# x)

/** This macro inverts a UVar in/out accesses.

 After this call is made, writes by this module affect the sensed
 value, and reads read the target value. Writes by other modules
 and URBI code affect the target value, and reads get the sensed
 value. Without this call, all operations affect the same
 underlying variable.  */
# define UOwned(x) x.setOwned()
// for backward compatibility
# define USensor(x) x.setOwned()

/** Bind the function x in current URBI object to the C++ member
 function of same name.  The return value and parameters must be of
 a basic integral or floating types, char *, std::string, UValue,
 UBinary, USound or UImage, or any type that can cast to/from
 UValue.  */
# define UBindFunction(obj,x)						\
  ::urbi::createUCallback(__name, "function", this,			\
			  (&obj::x), __name + "." #x,			\
			  ::urbi::functionmap, false)

/** Registers a function x in current object that will be called each
 time the event of same name is triggered. The function will be
 called only if the number of arguments match between the function
 prototype and the URBI event.
 */
# define UBindEvent(obj,x)						\
  ::urbi::createUCallback(__name, "event", this,			\
			  (&obj::x), __name + "." #x,			\
			  ::urbi::eventmap, false)

/** Registers a function x in current object that will be called each
 * time the event of same name is triggered, and a function fun called
 * when the event ends. The function will be called only if the number
 * of arguments match between the function prototype and the URBI
 * event.
 */
# define UBindEventEnd(obj,x,fun)					\
  ::urbi::createUCallback(__name, "eventend", this,			\
			  (&obj::x),(&obj::fun), __name + "." #x,	\
			  ::urbi::eventendmap)

/// Register current object to the UObjectHub named 'hub'.
# define URegister(hub)						\
  do {								\
    objecthub = ::urbi::getUObjectHub(#hub);			\
    if (objecthub)						\
      objecthub->addMember(dynamic_cast<UObject*>(this));	\
    else							\
      ::urbi::echo("Error: hub name '%s' is unknown\n", #hub);	\
  } while (0)


//macro to send urbi commands
# ifndef URBI
/** Send unquoted URBI commands to the server. Add an extra layer of parenthesis
    for safety.
*/
#   define URBI(a) ::urbi::uobject_unarmorAndSend(# a)
# endif


namespace urbi
{

  typedef int UReturn;

  // For remote mode.
  extern void main(int argc, char *argv[]);

  /// an empty dummy UObject used by UVar to set a NotifyChange
  /// This avoid coupling a UVar to a particular object
  extern UObject* dummyUObject;

  // Global function of the urbi:: namespace to access kernel features

  /// Write a message to the server debug output. Printf syntax.
  void echo(const char* format, ... );
  /// Retrieve a UObjectHub based on its name or return 0 if not found.
  UObjectHub* getUObjectHub(const std::string& n);
  /// Retrieve a UObject based on its name or return 0 if not found.
  UObject* getUObject(const std::string& n);

  /// Send URBI code (ghost connection in plugin mode, default
  /// connection in remote mode).
  void uobject_unarmorAndSend(const char* str);
  /// Send the string to the connection hosting the UObject.
  void send(const char* str);
  /// Send buf to the connection hosting the UObject.
  void send(void* buf, int size);

  /** Main UObject class definition
      Each UObject instance corresponds to an URBI object. It provides mechanisms to
      bind variables and functions between C++ and URBI.
  */
  class UObject
  {
  public:

    UObject(const std::string&);
    /// dummy UObject constructor
    UObject(int);
    virtual ~UObject();

    /// Calls the specified function each time the variable v is modified.
    template <class T>
    void UNotifyChange (UVar& v, int (T::*fun) ())
    {
      // This call registers both an UObject (say of type
      // UObjectDerived), and a callback working on it (named here
      // fun).  createUCallback wants both the object and the callback
      // to have the same type, which is not the casem this is static
      // type of the former is UObject (its runtime type is indeed
      // UObjectDerived though), and the callback wants a
      // UObjectDerived.  So we need a cast, until a more elegant way
      // is found (e.g., using free standing functions instead of a
      // member functions).
      createUCallback(__name, "var",
		      dynamic_cast<T*>(this),
		      fun, v.get_name(), monitormap, v.owned);
    }

    /// Calls the specified function each time the variable v is modified.
    template <class T>
    void UNotifyChange (UVar& v, int (T::*fun) (UVar&))
    {
      UGenericCallback* cb =
	createUCallback(__name, "var",
			dynamic_cast<T*>(this),
			fun, v.get_name(), monitormap, v.owned);
      if (cb)
	cb->storage = &v;
    }

    /// Calls the specified function when the variable value is updated on
    /// request by requestValue().
    template <class T>
    void UNotifyOnRequest (UVar& v, int (T::*fun) ())
    {
      createUCallback(__name, "var_onrequest",
		      dynamic_cast<T*>(this),
		      fun, v.get_name(), monitormap, v.owned);
    }

    /// Calls the specified function when the variable value is updated on
    /// request by requestValue().
    template <class T>
    void UNotifyOnRequest (UVar& v, int (T::*fun) (UVar&))
    {
      UGenericCallback* cb =
	createUCallback(__name, "var_onrequest",
			dynamic_cast<T*>(this),
			fun, v.get_name(), monitormap, v.owned);
      if (cb)
	cb->storage = &v;
    }

    /// Calls the specified function each time the variable v is read.
    template <class T>
    void UNotifyAccess (UVar& v, int (T::*fun) ())
    {
      createUCallback(__name, "varaccess",
		      dynamic_cast<T*>(this),
		      fun, v.get_name(), accessmap, v.owned);
    }

    /// Calls the specified function each time the variable v is read.
    template <class T>
    void UNotifyAccess (UVar& v, int (T::*fun) (UVar&))
    {
      UGenericCallback* cb =
	createUCallback(__name, "varaccess",
			dynamic_cast<T*>(this),
			fun, v.get_name(), accessmap, v.owned);
      if (cb)
	cb->storage = &v;
    }

    /// Calls the specified function each time the variable v is modified.
    template <class T>
    void UNotifyChange (const std::string& name, int (T::*fun) ())
    {
      createUCallback(__name, "var",
		      dynamic_cast<T*>(this), fun, name, monitormap);
    }

    /// Calls the specified function each time the variable v is modified.
    template <class T>
    void UNotifyChange (const std::string& name, int (T::*fun) (UVar&))
    {
      UGenericCallback* cb =
	createUCallback(__name, "var",
			dynamic_cast<T*>(this), fun, name, monitormap, false);
      if (cb)
	cb->storage = new UVar(name);
    }

    /// Set a timer that will call tune 'fun' function every 't' milliseconds.
    template <class T>
    void USetTimer(ufloat t, int (T::*fun) ())
    {
      new UTimerCallbackobj<T> (__name, t,
				dynamic_cast<T*>(this), fun, timermap);
    }

    /// Request permanent synchronization for v.
    void USync(UVar &v);

    /// Name of the object as seen in URBI.
    std::string __name;
    /// Name of the class the object is derived from.
    std::string classname;
    /// True when the object has been newed by an urbi command.
    bool   derived;

    UObjectList members;

    /// The hub, if it exists.
    UObjectHub  *objecthub;

    /// The connection used by send().
    UGhostConnection* gc;
    /// Send a command to URBI.
    int send(const std::string& s);

    /// Set a timer that will call the update function every 'period'
    /// milliseconds
    void USetUpdate(ufloat period);
    virtual int update() {return 0;};
    /// Set autogrouping facility for each new subclass created..
    void UAutoGroup() { autogroup = true; };
    /// Called when a subclass is created if autogroup is true.
    virtual void addAutoGroup() { UJoinGroup(classname+"s"); };

    /// Join the uobject to the 'gpname' group.
    virtual void UJoinGroup(const std::string& gpname);
    /// Void function used in USync callbacks.
    int voidfun() {return  0;};
    /// Add a group with a 's' after the base class name.
    bool autogroup;

    /// Flag to know whether the UObject is in remote mode or not
    bool remote;

    /// Remove all bindings, this method is called by the destructor.
    void clean();

    /// The load attribute is standard and can be used to control the
    /// activity of the object.
    UVar load;

  private:
    /// Pointer to a globalData structure specific to the
    /// remote/plugin architectures who defines it.
    UObjectData*  objectData;
    ufloat period;
  };


  //! Main UObjectHub class definition
  class UObjectHub
  {
  public:

    UObjectHub(const std::string&);
    virtual ~UObjectHub();

    void addMember(UObject* obj);

    /// Set a timer that will call update every 'period' milliseconds.
    void USetUpdate(ufloat period);
    virtual int update() {return 0;}

    UObjectList  members;
    UObjectList* getSubClass(const std::string&);
    //   UObjectList* getAllSubClass(const std::string&); //TODO

  protected:
    /// This function calls update and the subclass update.
    int updateGlobal();

    ufloat period;
    std::string name;
  };

} // end namespace urbi

// This file needs the definition of UObject, so included last.
// To be cleaned later.
# include "urbi/ustarter.hh"

#endif // ! URBI_UOBJECT_HH

/// Local Variables:
/// mode: c++
/// End:
