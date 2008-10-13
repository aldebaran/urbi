#ifndef KERNEL_CONNECTION_SET_HH
# define KERNEL_CONNECTION_SET_HH

# include <boost/ptr_container/ptr_list.hpp>

# include <kernel/uconnection.hh>

namespace kernel
{
  struct ConnectionSet
  {
    void add(UConnection* c);
    void remove(UConnection* c);
    void remove_closing();
    void clear();

    typedef boost::ptr_list<UConnection> connections_type;

    /// Present an iterable interface for sake of foreach.
    typedef connections_type::iterator iterator;
    typedef connections_type::const_iterator const_iterator;
    iterator begin();
    iterator end();

    connections_type connections_;
  };
}

#endif
