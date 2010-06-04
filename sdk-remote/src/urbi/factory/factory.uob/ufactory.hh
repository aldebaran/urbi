/*
 * Copyright (C) 2009-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef FACTORY_UFACTORY_HH
# define FACTORY_UFACTORY_HH

// Include the UObject declarations.
# include <urbi/uobject.hh>

// We wrap factories.
# include "factory.hh"

/// A UObject wrapping a factory object.
class UFactory
  : public urbi::UObject
{
public:
  /// C++ contructor.
  /// \param name  name given to the instance.
  UFactory(const std::string& name);

  /// Urbi constructor.
  /// \param d  the duration of the assembly process.
  ///           Must be positive.
  /// \return 0 on success.
  int init(ufloat d);

  /// Wrapper around Factory::operator().
  std::string assemble(std::list<std::string> components);

  /// Function notified when the duration is changed.
  /// \param v   the UVar being modified (i.e., UFactory::duration).
  /// \return 0  on success.
  int duration_set(urbi::UVar& v);

private:
  /// The duration of the assembly process.
  urbi::UVar duration;

  /// The actual factory, wrapped in this UObject.
//  Factory* factory;
};
#endif // ! FACTORY_UFACTORY_HH
