/// \file urbi/ustarter.hxx

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

#ifndef URBI_STARTER_HXX
# define URBI_STARTER_HXX

# include <algorithm>
# include <string>

# include <libport/containers.hh>

# include <urbi/fwd.hh>
# include <urbi/ustarter.hh>

namespace urbi
{

  /*------------------.
  | baseURBIStarter.  |
  `------------------*/

  inline
  baseURBIStarter::baseURBIStarter(const std::string& name)
    : name(name)
  {}

  inline
  baseURBIStarter::~baseURBIStarter()
  {}


  /*--------------.
  | URBIStarter.  |
  `--------------*/
  template <class T>
  inline
  URBIStarter<T>::URBIStarter(const std::string& name, UStartlist& slist)
    : baseURBIStarter(name)
    , slist_(slist)
  {
    slist_.push_back(this);
  }

  template <class T>
  inline
  URBIStarter<T>::~URBIStarter()
  {
    clean();
  }

  template <class T>
  inline
  void
  URBIStarter<T>::clean()
  {
    delete getUObject();
    UStartlist::iterator i = libport::find(slist_, this);
    if (i != slist_.end())
      slist_.erase(i);
  }

  template <class T>
  inline
  void
  URBIStarter<T>::copy(const std::string& objname)
  {
    URBIStarter<T>* ustarter = new URBIStarter<T>(objname, slist_);
    ustarter->init(objname);
    UObject* uso = ustarter->object;
    getUObject()->members.push_back(uso);
    uso->derived = true;
    uso->classname = getUObject()->classname;
    if (uso->autogroup)
      uso->addAutoGroup();
  }

  /// Access to the object from the outside.
  template <class T>
  inline
  UObject*
  URBIStarter<T>::getUObject()
  {
    return object;
  }

  template <class T>
  inline
  void
  URBIStarter<T>::init(const std::string& objname)
  {
    object = new T(objname);
  }

    /*---------------------.
    | baseURBIStarterHub.  |
    `---------------------*/

  inline
  baseURBIStarterHub::baseURBIStarterHub(const std::string& name)
    : name(name)
  {}

  inline
  baseURBIStarterHub::~baseURBIStarterHub()
  {}

    /*-----------------.
    | URBIStarterHub.  |
    `-----------------*/

  template <class T>
  inline
  URBIStarterHub<T>::URBIStarterHub(const std::string& name,
                                    UStartlistHub& slist)
    : baseURBIStarterHub(name)
    , slist_(slist)
  {
    slist_.push_back(this);
  }

  template <class T>
  inline
  URBIStarterHub<T>::~URBIStarterHub()
  {
    /* noone can kill a hub*/
  }

  template <class T>
  inline
  void
  URBIStarterHub<T>::init(const std::string& objname)
  {
    object = new T(objname);
  }

  /// Access to the object from the outside.
  template <class T>
  inline
  UObjectHub*
  URBIStarterHub<T>::getUObjectHub()
  {
    return object;
  }

} // end namespace urbi

#endif // !URBI_STARTER_HXX
