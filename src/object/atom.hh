/**
 ** \file object/atom.hh
 ** \brief Definition of atomic objects.
 */

#ifndef OBJECT_ATOM_HH
# define OBJECT_ATOM_HH

# include <boost/scoped_ptr.hpp>
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

    /// What we store.
    typedef typename Traits::type value_type;

    /// What we store, as a ref (for arguments and return values).
    typedef typename libport::ref_traits<value_type>::type value_ref_type;

    /// Ref-couting.
    typedef libport::shared_ptr< Atom<Traits> > shared_type;

    /// Construct an Atom with value \p v.
    Atom (const value_type v);

    /// Destroy an Atom.
    virtual ~Atom ();
    /** \} */

    /// Base class.
    typedef Object super_type;

    /// The kind of Atom.
    enum { kind = Traits::kind };
    virtual kind_type kind_get () const;

    /// The value held.
    const value_type value_get () const;
    value_ref_type value_get ();

    void value_set (value_ref_type v);

    /// Comparison methods
    /// \{
    bool operator< (const Atom& rhs) const;
    virtual bool operator< (const Object& rhs) const;
    /// \}

    /// For debugging.
    std::ostream& special_slots_dump (std::ostream& o) const;

    std::ostream& print(std::ostream& out) const;

  private:
    /// The value.
    value_type value_;
  };


  /*--------------------------.
  | Free standing functions.  |
  `--------------------------*/

  /// Clone, i.e., create a fresh object with this class as sole parent.
  // Overloads the definition for rObject.
  // Not a member function because we want the shared_ptr, which
  // is not available via this.
  template <typename Traits>
  typename Atom<Traits>::shared_type
  clone (typename Atom<Traits>::shared_type ref);

  /*----------.
  | Delegate. |
  `----------*/

  struct delegate_traits
  {
    typedef IDelegate* type;
    enum { kind = Object::kind_delegate };
  };

  /*-------.
  | Code.  |
  `-------*/

  struct code_traits
  {
    typedef ast::Function& type;
    enum { kind = Object::kind_code };
  };


  /*-------.
  | Lobby.  |
  `-------*/

  struct lobby_traits
  {
    typedef State type;
    enum { kind = Object::kind_lobby };
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

#endif // !OBJECT_ATOM_HH
