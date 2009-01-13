/// \file urbi/ustarter.hh

// This file is part of UObject Component Architecture
// Copyright (c) 2007, 2008, 2009 Gostai S.A.S.
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

# include <string>

# include <urbi/export.hh>
# include <urbi/uobject.hh>

/// This macro must be called once for every UObject class.
# define UStartRename(Type, Name)               \
  ::urbi::URBIStarter<Type>                     \
  Name ##  ____URBI_object(#Name)

/// Append connectionID to object name
# define UStartWithID(Type)                     \
  ::urbi::URBIStarter<Type>                     \
  Type ##  ____URBI_object(#Type, true)

/// This macro must be called once for every UObject class.
# define UStart(Type)                           \
  UStartRename(Type, Type)

/// This macro must be called once for each UObjectHub class.
# define UStartHub(Type)                        \
  ::urbi::URBIStarterHub<Type>                  \
  Type ##  ____URBI_object(#Type)

namespace urbi
{

  /*-----------.
  | UStarter.  |
  `-----------*/

  /// URBIStarter base class used to store heterogeneous template
  /// class objects in starterlist.
  class URBI_SDK_API baseURBIStarter
  {
  public:
    baseURBIStarter(const std::string& name, bool local = false);
    virtual ~baseURBIStarter();

    virtual UObject* getUObject() = 0;

    /// Called before deletion.
    virtual void clean() = 0;
    /// Used to provide a wrapper to initialize objects in starterlist.
    virtual void init(const std::string&) = 0;
    /// Used to provide a copy of a C++ object based on its name.
    virtual void copy(const std::string&) = 0;
    std::string name;
    /// Set to true to have the UObjects use a random unique name.
    bool local;
    /// Return the full name to pass to the UObject constructor.
    std::string getFullName(const std::string& name);
    /// List of starters.
    typedef std::list<baseURBIStarter*> list_type;
    static list_type& list();
    /// Retrieve an UObject, otherwise 0.
    static UObject* find(const std::string& name);
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
    URBIStarter(const std::string& name, bool local = false);

    virtual ~URBIStarter();

    virtual void clean();

    virtual void copy(const std::string& objname);

    /// Access to the object from the outside.
    virtual UObject* getUObject();

  protected:
    /// Called when the object is ready to be initialized.
    virtual void init(const std::string& objname);

    T* object;
  };



  /*--------------.
  | UStarterHub.  |
  `--------------*/

  /// URBIStarter base class used to store heterogeneous template
  /// class objects in starterlist
  class URBI_SDK_API baseURBIStarterHub
  {
  public:
    baseURBIStarterHub(const std::string& name);
    virtual ~baseURBIStarterHub();

    /// Used to provide a wrapper to initialize objects in starterlist.
    virtual void init(const std::string&) = 0;
    virtual UObjectHub* getUObjectHub() = 0;
    std::string name;

    /// UObjectHub list.
    typedef std::list<baseURBIStarterHub*> list_type;
    static list_type& list();
    /// Retrieve an UObjectHub, otherwise 0.
    static UObjectHub* find(const std::string& name);
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
    URBIStarterHub(const std::string& name);
    virtual ~URBIStarterHub();

  protected:
    /// Called when the object is ready to be initialized.
    virtual void init(const std::string& objname);

    /// Access to the object from the outside.
    virtual UObjectHub* getUObjectHub();

    T* object;
  };

} // end namespace urbi

# include <urbi/ustarter.hxx>

#endif // ! URBI_STARTER_HH
