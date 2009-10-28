/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

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

    template<typename T> void
    deletor(T* ptr)
    {
      delete ptr;
    }

    template<typename T> void
    UContextImpl::addCleanup(T* ptr)
    {
      cleanup_list_.push_back(boost::bind(&deletor<T>, ptr));
    }

    inline void
    UContextImpl::cleanup()
    {
      // I would rather not define 'foreach' in a hxx.
      BOOST_FOREACH(boost::function0<void>& f, cleanup_list_)
        f();
      cleanup_list_.clear();
    }
  }

}
