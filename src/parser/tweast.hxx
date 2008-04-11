/**
 ** \file parser/tweast.hxx
 ** \brief implements inline methods of parser/tweast.hh
 */

#ifndef PARSER_TWEAST_HXX
# define PARSER_TWEAST_HXX

# include <libport/foreach.hh>

# include "parser/tweast.hh"

namespace parser
{

  template <typename T>
  T&
  Tweast::append_ (unsigned&, T& data) const
  {
    return data;
  }

  template <typename T>
  Tweast&
  Tweast::operator<< (const T& t)
  {
    input_ << append_ (count_, t);
    return *this;
  }

  template <typename T>
  T*
  Tweast::take (unsigned s) throw (std::range_error)
  {
    return MetavarMap<T>::take_ (s);
    // T* t = 0;
    //    try
    //      {
    //	t = MetavarMap<T>::take_ (s);
    //      }
    //    catch (std::range_error& e)
    //      {
    //	// Print the contents of the Tweast before dying.
    //	misc::error error;
    //	error << e.what () << std::endl;
    //	error << *this;
    //	error.ice_here ();
    //      }
    // return t;
  }

}

#endif // !PARSER_TWEAST_HXX
