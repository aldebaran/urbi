/*
 * Copyright (C) 2009-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/bind.hh>

namespace urbi
{
  namespace impl
  {

    /*---------------.
    | UContextImpl.  |
    `---------------*/

    // Declared pure virtual, but needs an implementation.  Known
    // idiom.
    inline
    UContextImpl::~UContextImpl()
    {}

    inline
    void
    UContextImpl::send(const std::string& s)
    {
      send(s.c_str(), s.length());
    }

    /*--------------.
    | UObjectImpl.  |
    `--------------*/

    inline
    UObjectImpl::~UObjectImpl()
    {
    }

    /*-----------.
    | UVarImpl.  |
    `-----------*/
    inline
    UVarImpl::~UVarImpl()
    {
    }

    /*---------------.
    | UContextImpl.  |
    `---------------*/

    /// Yield execution for \b delay.
    inline
    void
    UContextImpl::yield_for(libport::utime_t delay) const
    {
      return yield_until(libport::utime() + delay);
    }

    template <typename T> void
    deletor(T* ptr)
    {
      delete ptr;
    }

    template <typename T> void
    UContextImpl::addCleanup(T* ptr)
    {
      addCleanup(boost::bind(&deletor<T>, ptr));
    }
  }

}
