#ifndef FACTORY_UFACTORY_HH
# define FACTORY_UFACTORY_HH

# include <urbi/uobject.hh>
# include "factory.hh"

class UFactory
  : public urbi::UObject
{
public:
  UFactory(const std::string& name);
  int init(unsigned d);
  std::string assemble(std::list<std::string> components);
  int duration_set(urbi::UVar& v);

private:
  urbi::UVar duration;
  Factory* factory;
};
#endif // ! FACTORY_UFACTORY_HH
