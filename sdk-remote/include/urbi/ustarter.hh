/*
 * Copyright (C) 2007, 2008, 2009, 2010, 2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file urbi/ustarter.hh
#ifndef URBI_STARTER_HH
# define URBI_STARTER_HH

# include <libport/preproc.hh>

# include <string>

# include <urbi/export.hh>
# include <urbi/fwd.hh>

#include <urbi/ucontext.hh>
#include <urbi/uobject.hh>
#include <urbi/version-check.hh>

/// This macro must be called once for every UObject class.
# define UStartRename(Type, Name)                       \
  ::urbi::URBIStarter<Type>                             \
  Name ##  ____URBI_object(#Name);                      \
  URBI_CHECK_SDK_VERSION_BARE(#Name)

/// Append connectionID to object name
# define UStartWithID(Type)                             \
  ::urbi::URBIStarter<Type>                             \
  Type ##  ____URBI_object(#Type, true);                \
  URBI_CHECK_SDK_VERSION_BARE(#Type)

/// This macro must be called once for every UObject class.
# define UStart(Type)                           \
  UStartRename(Type, Type)

/// This macro must be called once for each UObjectHub class.
# define UStartHub(Type)                                \
  ::urbi::URBIStarterHub<Type>                          \
  Type ##  ____URBI_object(#Type);                      \
  URBI_CHECK_SDK_VERSION_BARE(#Type)

namespace urbi
{

  /*-----------.
  | UStarter.  |
  `-----------*/

  /*** UObject factory class.
  */
  class URBI_SDK_API baseURBIStarter
  {
  public:
    baseURBIStarter(const std::string& name, bool local = false);
    virtual ~baseURBIStarter();
    /// Create an instance of the UObject.
    virtual UObject* instanciate(impl::UContextImpl* ctx,
                                 const std::string& n = std::string()) = 0;

    std::string name;
    /// Set to true to have the UObjects use a random unique name.
    bool local;

    /// List of starters.
    typedef std::list<baseURBIStarter*> list_type;
    static list_type& list();
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

    virtual UObject* instanciate(impl::UContextImpl* ctx,
                                 const std::string& n=std::string());

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
    virtual UObjectHub* instanciate(impl::UContextImpl* ctx,
                                 const std::string& n=std::string()) = 0;
    std::string name;

    /// UObjectHub list.
    typedef std::list<baseURBIStarterHub*> list_type;
    static list_type& list();

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
    virtual UObjectHub* instanciate(impl::UContextImpl* ctx,
                                 const std::string& n=std::string());
    virtual ~URBIStarterHub();

  };

} // end namespace urbi

# include <urbi/ustarter.hxx>

#endif // ! URBI_STARTER_HH
