#include <boost/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/ptr_container/ptr_list.hpp>

#include <kernel/connection-set.hh>
#include <kernel/uconnection.hh>

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

  static bool
  uconnection_compare(UConnection* lhs, UConnection& rhs)
  {
    return lhs == &rhs;
  }

  void
  ConnectionSet::remove(UConnection* c)
  {
    connections_.erase_if(boost::bind(uconnection_compare, c, _1));
  }

  void
  ConnectionSet::remove_closing()
  {
    connections_.erase_if(boost::bind(&UConnection::closing_get, _1));
  }

  void
  ConnectionSet::clear()
  {
    connections_.clear();
  }

}

