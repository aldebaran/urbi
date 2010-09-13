/*
 * Copyright (C) 2009, 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/bind.hh>
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


    template<typename T> void
    deletor(T* ptr)
    {
      delete ptr;
    }

    template<typename T> void
    UContextImpl::addCleanup(T* ptr)
    {
      cleanup_list_->back().push_back(boost::bind(&deletor<T>, ptr));
    }

    inline void
    UContextImpl::pushCleanupStack()
    {
      CleanupList* cl = cleanup_list_.get();
      if (!cl)
      {
        cl = new CleanupList;
        cleanup_list_.reset(cl);
      }
      cl->resize(cl->size()+1);
    }

    inline void
    UContextImpl::popCleanupStack()
    {
      // I would rather not define 'foreach' in a hxx.
      BOOST_FOREACH(boost::function0<void>& f, cleanup_list_->back())
        f();
      cleanup_list_->pop_back();
    }

    inline
    UContextImpl::CleanupStack::CleanupStack(UContextImpl& owner)
    : owner_(owner)
    {
      owner_.pushCleanupStack();
    }

    inline
    UContextImpl::CleanupStack::~CleanupStack()
    {
      owner_.popCleanupStack();
    }
  }

}
