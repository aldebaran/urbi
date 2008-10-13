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

  void
  ConnectionSet::remove(UConnection* c)
  {
    connections_.erase_if(&boost::lambda::_1 == c);
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

