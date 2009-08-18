namespace object
{
  template <typename Exact, typename Value>
  inline
  bool
  EqualityComparable<Exact, Value>::operator==(const rObject& rhs) const
  {
    return (rhs->is_a<Exact>()
            && *this == rhs->as<Exact>()->value_get());
  }

  template <typename Exact, typename Value>
  inline
  bool
  EqualityComparable<Exact, Value>::operator==(const value_type& rhs) const
  {
    return value_get() == rhs;
  }
}
