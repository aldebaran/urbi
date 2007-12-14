#ifndef UBINDER_HXX
# define UBINDER_HXX

# include "ubinder.hh"

template <typename S>
void
removeMonitor(HMbindertab& t, S c)
{
  std::list<HMbindertab::iterator> deletelist;
  for (HMbindertab::iterator i = t.begin(); i != t.end(); ++i)
    if (i->second->removeMonitor(c))
      deletelist.push_back(i);

  BOOST_FOREACH (HMbindertab::iterator i, deletelist)
    t.erase(i);
  deletelist.clear();
}

#endif
