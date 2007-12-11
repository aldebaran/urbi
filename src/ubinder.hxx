#ifndef UBINDER_HXX
# define UBINDER_HXX

# include "ubinder.hh"

template <typename S>
void
remove_monitor(HMbindertab& t, S c)
{
  std::list<HMbindertab::iterator> deletelist;
  for (HMbindertab::iterator i = t.begin(); i != t.end(); ++i)
    if (i->second->removeMonitor(c))
      deletelist.push_back(i);

  BOOST_FOREACH (HMbindertab::iterator i, deletelist)
    t.erase(i);
  deletelist.clear();
}

template <typename S, typename T>
void
unbind_monitor(libport::hash_map<const char*, S>& table, T c)
{
  typedef libport::hash_map<const char*, S> table_type;
  BOOST_FOREACH (typename table_type::value_type i, table)
    if (i.second->binder
	&& i.second->binder->removeMonitor(c))
    {
      delete i.second->binder;
      i.second->binder = 0;
    }
}

#endif
