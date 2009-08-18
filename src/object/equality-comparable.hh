#ifndef OBJECT_EQUALITY_COMPARABLE_HH
# define OBJECT_EQUALITY_COMPARABLE_HH

namespace object
{
  // I have failed to deduce value_type from Exact.
  template <typename Exact, typename Value>
  class EqualityComparable
  {
  public:
    typedef Exact exact_type;
    typedef Value value_type;

    virtual const value_type& value_get() const = 0;

    bool operator==(const rObject& rhs) const;
    bool operator==(const value_type& rhs) const;
  };
}

# include <object/equality-comparable.hxx>

#endif // ! OBJECT_EQUALITY_COMPARABLE_HH
