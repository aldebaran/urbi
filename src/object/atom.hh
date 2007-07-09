/**
 ** \file object/float.hh
 ** \brief Definition of object::Float.
 */

#ifndef OBJECT_FLOAT_HH
# define OBJECT_FLOAT_HH

# include "libport/ufloat.h"
# include "object/object.hh"

namespace object
{

  /// Run time values for Urbi: the simple atoms.
  template <typename Traits>
  class Atom : public Object
  {
    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct an Atom with value \p v.
    Atom (const typename Traits::type& v);

    /// Destroy an Atom.
    virtual ~Atom ();
    /** \} */

    /// Base class.
    typedef Object super_type;

    /// The kind of Atom.
    virtual std::string kind_get () const;

    /// For debugging.
    std::ostream& special_slots_dump (std::ostream& o) const;

  private:
    /// The value.
    typename Traits::type value_;
  };


  /*--------.
  | Float.  |
  `--------*/

  struct float_traits
  {
    typedef libport::ufloat type;
    static const char kind[];
  };

  typedef Atom<float_traits> Float;

  /*----------.
  | Integer.  |
  `----------*/

  struct integer_traits
  {
    typedef int type;
    static const char kind[];
  };

  typedef Atom<integer_traits> Integer;

  /*---------.
  | String.  |
  `---------*/

  struct string_traits
  {
    typedef std::string type;
    static const char kind[];
  };

  typedef Atom<string_traits> String;

} // namespace object

# include "object/atom.hxx"

#endif // !OBJECT_FLOAT_HH
