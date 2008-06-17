/// \file urbi/ustarter.hh

// This file is part of UObject Component Architecture
// Copyright (c) 2007, 2008 Gostai S.A.S.
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//
// For more information, comments, bug reports: http://www.urbiforge.com

#ifndef URBI_STARTER_HH
# define URBI_STARTER_HH

# include <algorithm>
# include <string>

# include <urbi/fwd.hh>

/// This macro must be called once for every UObject class.
# define UStartRename(Type, Name)                                       \
  ::urbi::URBIStarter<Type>                                             \
  Name ##  ____URBI_object(std::string(#Name), ::urbi::objectlist)

/// This macro must be called once for every UObject class.
# define UStart(Type)							\
  UStartRename(Type, Type)

/// This macro must be called once for each UObjectHub class.
# define UStartHub(Type)                                                \
  ::urbi::URBIStarterHub<Type>                                          \
  Type ##  ____URBI_object(std::string(#Type), ::urbi::objecthublist)

namespace urbi
{

  typedef std::list<baseURBIStarter*> UStartlist;
  typedef std::list<baseURBIStarterHub*> UStartlistHub;

  // Two singleton lists to handle the object and hubobject registration.
  EXTERN_STATIC_INSTANCE(UStartlist, objectlist);
  EXTERN_STATIC_INSTANCE(UStartlistHub, objecthublist);

  /// URBIStarter base class used to store heterogeneous template
  /// class objects in starterlist.
  class baseURBIStarter
  {
  public:
    baseURBIStarter(const std::string& name);
    virtual ~baseURBIStarter();

    virtual UObject* getUObject() = 0;

    /// Called before deletion.
    virtual void clean() = 0;
    /// Used to provide a wrapper to initialize objects in starterlist.
    virtual void init(const std::string&) = 0;
    /// Used to provide a copy of a C++ object based on its name.
    virtual void copy(const std::string&) = 0;
    std::string name;
  };

  //! This is the class containing URBI starters
  /** A starter is a class whose job is to start an instance of a particular
   * UObject subclass, resulting in the initialization of this object
   * (registration to the kernel)
   */
  template <class T>
  class URBIStarter
    : public baseURBIStarter
  {
  public:
    URBIStarter(const std::string& name, UStartlist& _slist);

    virtual ~URBIStarter();

    virtual void clean();

    virtual void copy(const std::string& objname);

    /// Access to the object from the outside.
    virtual UObject* getUObject();

  protected:
    /// Called when the object is ready to be initialized.
    virtual void init(const std::string& objname);

    UStartlist* slist;
    T* object;
  };



#define SETBACKCASTCTOR(T)			\
  inline					\
  UValue					\
  back_cast(T& t)				\
  {						\
    return UValue(t);				\
  }

SETBACKCASTCTOR(bool)
SETBACKCASTCTOR(int)
SETBACKCASTCTOR(long)
SETBACKCASTCTOR(ufloat)
SETBACKCASTCTOR(UValue)
SETBACKCASTCTOR(char*)
SETBACKCASTCTOR(void*)
SETBACKCASTCTOR(const std::string)
SETBACKCASTCTOR(std::string)
SETBACKCASTCTOR(const UBinary)
SETBACKCASTCTOR(const UList)
SETBACKCASTCTOR(const UObjectStruct)
SETBACKCASTCTOR(const USound)
SETBACKCASTCTOR(const UImage)



  /// URBIStarter base class used to store heterogeneous template
  /// class objects in starterlist
  class baseURBIStarterHub
  {
  public:
    baseURBIStarterHub(const std::string& name);
    virtual ~baseURBIStarterHub();

    /// Used to provide a wrapper to initialize objects in starterlist.
    virtual void init(const std::string&) = 0;
    virtual UObjectHub* getUObjectHub() = 0;
    std::string name;
  };

  //! This is the class containing URBI starters
  /** A starter is a class whose job is to start an instance of a particular
   * UObject subclass, resulting in the initialization of this object
   * (registration to the kernel)
   */
  template <class T>
  class URBIStarterHub
    : public baseURBIStarterHub
  {
  public:
    URBIStarterHub(const std::string& name, UStartlistHub& _slist);
    virtual ~URBIStarterHub();

  protected:
    /// Called when the object is ready to be initialized.
    virtual void init(const std::string& objname);

    /// Access to the object from the outside.
    virtual UObjectHub* getUObjectHub();

    UStartlistHub* slist;
    T* object;
  };

} // end namespace urbi

# include <urbi/ustarter.hxx>

#endif // ! URBI_STARTER_HH
