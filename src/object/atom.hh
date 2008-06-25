/**
 ** \file object/atom.hh
 ** \brief Definition of atomic objects.
 */

#ifndef OBJECT_ATOM_HH
# define OBJECT_ATOM_HH

# include <deque>

# include <boost/any.hpp>
# include <boost/type_traits/add_reference.hpp>

# include <ast/code.hh>

namespace ast
{
  class Function;
}

# include <kernel/fwd.hh>
# include <scheduler/fwd.hh>

# include <libport/hash.hh>
# include <libport/ufloat.h>
# include <libport/symbol.hh>
# include <object/fwd.hh>
# include <object/object.hh>
# include <object/state.hh>

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
    typedef typename boost::add_reference<value_type>::type value_ref_type;

    /// Ref-couting.
    typedef libport::shared_ptr< Atom<Traits>, true > shared_type;

    /// Construct an Atom with value \p v. If \p add_proto is \a true,
    /// the appropriate prototype will be added.
    Atom (const value_type v, bool add_proto = true);

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

    virtual rObject clone () const;

    /// Comparison methods
    /// \{
    bool operator< (const Atom& rhs) const;
    virtual bool operator< (const Object& rhs) const;
    /// \}

    /// For debugging.
    std::ostream& special_slots_dump(std::ostream& o, runner::Runner&) const;

    static const std::string type_name;

  private:
    /// The value.
    value_type value_;
  };


  /*--------.
  | Alien.  |
  `--------*/

  // Aliens allow to store any kind of value.  For debugging purpose,
  // we also store its C++ type, hence the pair.
  struct alien_traits
  {
    typedef std::pair<boost::any, std::string> type;
    enum { kind = object_kind_alien };
  };


  /*----------.
  | Delegate. |
  `----------*/

  struct delegate_traits
  {
    typedef IDelegate* type;
    enum { kind = object_kind_delegate };
  };


  /*-------.
  | Lobby.  |
  `-------*/

  // The object inside which user interaction (via the connections) is
  // evaluated.
  struct lobby_traits
  {
    typedef State type;
    enum { kind = object_kind_lobby };
  };


  /*------------.
  | Semaphore.  |
  `------------*/

  // Urbi semaphores.
  struct semaphore_traits
  {
    typedef std::pair< int, std::deque<scheduler::rJob> > type;
    enum { kind = object_kind_semaphore };
  };

} // namespace object

#endif // !OBJECT_ATOM_HH
