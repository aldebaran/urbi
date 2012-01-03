/*
 * Copyright (C) 2008-2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef KERNEL_CONNECTION_SET_HH
# define KERNEL_CONNECTION_SET_HH

# include <boost/ptr_container/ptr_list.hpp>

# include <urbi/kernel/uconnection.hh>

namespace kernel
{
  class ConnectionSet
  {
  public:
    void add(UConnection* c);

    void clear();

    typedef std::list<UConnection*> connections_type;

    /// Present an iterable interface for sake of foreach.
    typedef connections_type::iterator iterator;
    typedef connections_type::const_iterator const_iterator;
    iterator begin();
    iterator end();

  private:
    connections_type connections_;
    friend class UServer;
  };
}

#endif
