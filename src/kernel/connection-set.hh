#ifndef KERNEL_CONNECTION_SET_HH
# define KERNEL_CONNECTION_SET_HH

# include <boost/ptr_container/ptr_list.hpp>

# include <kernel/uconnection.hh>

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
