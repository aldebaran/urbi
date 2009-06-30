/// \file urbi/ustarter.hxx

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

#ifndef URBI_STARTER_HXX
# define URBI_STARTER_HXX

# include <algorithm>
# include <string>

# include <urbi/uobject.hh>
# include <urbi/ustarter.hh>

namespace urbi
{

  /*------------------.
  | baseURBIStarter.  |
  `------------------*/

  inline
  baseURBIStarter::baseURBIStarter(const std::string& name, bool local)
    : name(name)
    , local(local)
  {}

  inline
  baseURBIStarter::~baseURBIStarter()
  {}

  /*--------------.
  | URBIStarter.  |
  `--------------*/
  template <class T>
  inline
  URBIStarter<T>::URBIStarter(const std::string& name, bool local)
    : baseURBIStarter(name, local)
  {
    list().push_back(this);
  }

  template <class T>
  inline
  URBIStarter<T>::~URBIStarter()
  {
    list().remove(this);
  }

  template <class T>
  inline
  UObject*
  URBIStarter<T>::instanciate(impl::UContextImpl* ctx,
                                 const std::string& n)
    {
      std::string rn = n;
      if (rn.empty())
        rn = name;
      // FIXME: not exception-safe
      setCurrentContext(ctx);
      UObject* res =  new T(rn);
      res->cloner = this;
      return res;
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
  URBIStarterHub<T>::URBIStarterHub(const std::string& name)
    : baseURBIStarterHub(name)
  {
    list().push_back(this);
  }

  template <class T>
  inline
  URBIStarterHub<T>::~URBIStarterHub()
  {
    list().remove(this);
  }
  template <class T>
  inline
  UObjectHub*
  URBIStarterHub<T>::instanciate(impl::UContextImpl* ctx,
                          const std::string& n)
  {
    setCurrentContext(ctx);
    return new T(n.empty()? name:n);
  }

} // end namespace urbi

#endif // !URBI_STARTER_HXX
