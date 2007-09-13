/**
 ** \file object/atom.hh
 ** \brief Definition of atomic objects (code, float, string).
 */

#ifndef OBJECT_FLOAT_HH
# define OBJECT_FLOAT_HH

# include "ast/fwd.hh"

# include "kernel/fwd.hh"

# include "libport/ufloat.h"
# include "libport/select-ref.hh"
# include "libport/symbol.hh"
# include "object/object.hh"
# include "object/state.hh"

namespace object
{

  /// Run time values for Urbi: the simple atoms.
  template <typename Traits>
  class Atom : public Object
  {
    /** \name Ctor & dtor.
     ** \{ */
  public:

    /// Give access to Traits. Useful in macros.
    typedef Traits traits;

    /// Construct an Atom with value \p v.
    Atom (const typename Traits::type v);

    /// Destroy an Atom.
    virtual ~Atom ();
    /** \} */

    /// Base class.
    typedef Object super_type;

    /// The kind of Atom.
    virtual kind_type kind_get () const;

    /// The value held.
    const typename Traits::type value_get () const;

    /// Comparison methods
    /// \{
    bool operator< (const Atom& rhs) const;
    virtual bool operator< (const Object& rhs) const;
    /// \}

    /// The held value.
    typename libport::ref_traits<typename Traits::type>::type value_get ();

    /// For debugging.
    std::ostream& special_slots_dump (std::ostream& o) const;

    std::ostream& print(std::ostream& out) const;

  private:
    /// The value.
    typename Traits::type value_;
  };


  /*-------.
  | Code.  |
  `-------*/

  struct code_traits
  {
    typedef ast::Function& type;
    enum { kind = Object::kind_code };
  };


  /*----------.
  | Context.  |
  `----------*/

  struct context_traits
  {
    typedef State type;
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


  /*-------.
  | List.  |
  `-------*/

  struct list_traits
  {
    typedef std::list<rObject> type;
    enum { kind = Object::kind_list };
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
    typedef libport::Symbol type;
    enum { kind = Object::kind_string };
  };

} // namespace object

# include "object/atom.hxx"

#endif // !OBJECT_FLOAT_HH
