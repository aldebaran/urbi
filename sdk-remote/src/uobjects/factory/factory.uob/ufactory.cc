#include "ufactory.hh"

UStart(UFactory);

UFactory::UFactory(const std::string& name)
  : urbi::UObject(name)
  , factory(0)
{
  UBindFunction(UFactory, init);
}

int
UFactory::init(unsigned d)
{
  UBindVar(UFactory, duration);
  UBindFunction(UFactory, assemble);

  duration = d;
  factory = new Factory(d);

  UNotifyChange(duration, &UFactory::duration_set);
  return 0;
}

int
UFactory::duration_set(urbi::UVar& v)
{
  assert(factory);
  factory->duration = static_cast<int>(v);
  return 0;
}


std::string
UFactory::assemble(std::list<std::string> components)
{
  assert(factory);
  return (*factory)(components);
}
