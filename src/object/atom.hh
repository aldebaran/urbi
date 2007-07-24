/**
 ** \file object/float.hh
 ** \brief Definition of object::Float.
 */

#ifndef OBJECT_FLOAT_HH
# define OBJECT_FLOAT_HH

# include "kernel/fwd.hh"
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
    virtual kind_type kind_get () const;

    /// The held value.
    const typename Traits::type value_get () const;
    typename Traits::type value_get ();

    /// For debugging.
    std::ostream& special_slots_dump (std::ostream& o) const;

  private:
    /// The value.
    typename Traits::type value_;
  };


  /*-------.
  | Code.  |
  `-------*/

  struct code_traits
  {
    // FIXME: Can be ref?
    typedef const ast::Exp* type;
    enum { kind = Object::kind_code };
  };


  /*----------.
  | Context.  |
  `----------*/

  struct context_traits
  {
    // FIXME: Can be ref?
    typedef const UConnection* type;
    enum { kind = Object::kind_context };
  };


  /*--------.
  | Float.  |
  `--------*/

  struct float_traits
  {
    typedef libport::ufloat type;
    enum { kind = Object::kind_float };
  };


  /*----------.
  | Integer.  |
  `----------*/

  struct integer_traits
  {
    typedef int type;
    enum { kind = Object::kind_integer };
  };


  /*------------.
  | Primitive.  |
  `------------*/

  struct primitive_traits
  {
    typedef primitive_type type;
    enum { kind = Object::kind_primitive };
  };


  /*---------.
  | String.  |
  `---------*/

  struct string_traits
  {
    typedef std::string type;
    enum { kind = Object::kind_string };
  };

} // namespace object

# include "object/atom.hxx"

#endif // !OBJECT_FLOAT_HH
