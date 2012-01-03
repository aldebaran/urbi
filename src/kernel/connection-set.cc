/*
 * Copyright (C) 2008-2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/bind.hh>
#include <boost/lambda/lambda.hpp>
#include <boost/ptr_container/ptr_list.hpp>

#include <kernel/connection-set.hh>
#include <urbi/kernel/uconnection.hh>

namespace kernel
{

  ConnectionSet::iterator
  ConnectionSet::begin()
  {
    return connections_.begin();
  }

  ConnectionSet::iterator
  ConnectionSet::end()
  {
    return connections_.end();
  }

  void
  ConnectionSet::add(UConnection* c)
  {
    connections_.push_front(c);
  }

  void
  ConnectionSet::clear()
  {
    connections_.clear();
  }

}

