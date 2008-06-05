/**
 ** \file parser/tweast.hxx
 ** \brief implements inline methods of parser/tweast.hh
 */

#ifndef PARSER_TWEAST_HXX
# define PARSER_TWEAST_HXX

# include <libport/foreach.hh>

# include <parser/tweast.hh>

namespace parser
{

  template <typename T>
  T&
  Tweast::append_(unsigned&, T& data) const
  {
    return data;
  }

  template <typename T>
  bool
  Tweast::must_be_unique_(const T&) const
  {
    return false;
  }

  template <typename T>
  Tweast&
  Tweast::operator<< (const T& t)
  {
    passert(t, !must_be_unique_(t) || unique_(t));
    input_ << append_(count_, t);
    return *this;
  }

  template <typename T>
  T
  Tweast::take(unsigned s) throw (std::range_error)
  {
    return MetavarMap<T>::take_(s);
  }

}

#endif // !PARSER_TWEAST_HXX
