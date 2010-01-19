/*
 * Copyright (C) 2009, 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef OBJECT_EQUALITY_COMPARABLE_HH
# define OBJECT_EQUALITY_COMPARABLE_HH

namespace urbi
{
  namespace object
  {
    // I have failed to deduce value_type from Exact.
    template <typename Exact, typename Value>
    class EqualityComparable
    {
    public:
      typedef Exact exact_type;
      typedef Value value_type;

      virtual ~EqualityComparable() = 0;
      virtual const value_type& value_get() const = 0;

      bool operator==(const rObject& rhs) const;
      bool operator==(const value_type& rhs) const;
    };
  }
}

# include <urbi/object/equality-comparable.hxx>

#endif // ! OBJECT_EQUALITY_COMPARABLE_HH
