#include <libport/foreach.hh>

#include "factory.hh"

Factory::Factory(unsigned d)
  : duration(d)
{}

std::string
Factory::operator()(const std::list<std::string>& components) const
{
  sleep(duration);
  std::string res;
  foreach (const std::string& s, components)
    res += s;
  return res;
}
