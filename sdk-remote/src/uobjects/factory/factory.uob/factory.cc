// A wrapper around Boost.Foreach.
#include <libport/foreach.hh>

#include "factory.hh"

Factory::Factory(float d)
  : duration(d)
{
  assert(0 <= d);
}

std::string
Factory::operator()(const strings& components) const
{
  // Waiting for duration seconds.
  useconds_t one_second = 1000 * 1000;
  usleep(duration * one_second);

  // Iterate over the list of strings (using Boost.Foreach), and
  // concatenate them in res.
  std::string res;
  foreach (const std::string& s, components)
    res += s;
  return res;
}
