#include "ufactory.hh"

// Register the UFactory UObject in the Urbi world.
UStart(UFactory);

// Bouncing the name to the UObject constructor is mandatory.
UFactory::UFactory(const std::string& name)
  : urbi::UObject(name)
  , factory(0)
{
  // Register the Urbi constructor.  This is the only mandatory
  // part of the C++ constructor.
  UBindFunction(UFactory, init);
}

int
UFactory::init(ufloat d)
{
  // Failure on invalid arguments.
  if (d < 0)
    return 1;

  // Bind the UVars before using them.
  UBindVar(UFactory, duration);
  // Bind the functions, i.e., declare them to the Urbi world.
  UBindFunction(UFactory, assemble);

  // Set the duration.
  duration = d;
  // Build the factory.
  factory = new Factory(d);

  // Require that duration_set be invoked each time duration is
  // changed.  Declared after the above "duration = d" since we
  // don't want it to be triggered for this first assignment.
  UNotifyChange(duration, &UFactory::duration_set);

  // Success.
  return 0;
}

int
UFactory::duration_set(urbi::UVar& v)
{
  assert(factory);
  ufloat d = static_cast<ufloat>(v);
  if (0 <= d)
  {
    // Valid value.
    factory->duration = d;
    return 0;
  }
  else
    // Report failure.
    return 1;
}


std::string
UFactory::assemble(std::list<std::string> components)
{
  assert(factory);
  // Bounce to Factory::operator().
  return (*factory)(components);
}
