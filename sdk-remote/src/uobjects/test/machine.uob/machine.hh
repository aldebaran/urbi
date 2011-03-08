/*
 * Copyright (C) 2009-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef MACHINE_MACHINE_HH
# define MACHINE_MACHINE_HH

# include <urbi/uobject.hh>

class Machine
{
public:
  /// Construction.
  /// \param duration  how long the assembly process takes.
  ///                  In seconds.
  Machine(float duration);

  /// Lists of strings.
  typedef std::list<std::string> strings;

  /// Assemble the raw components into a product.
  std::string operator()(const strings& components) const;

  /// The duration of the assembly process, in seconds.
  /// Must be positive.
  float duration;
};

#endif // ! MACHINE_MACHINE_HH
