/**
 ** \file object/atom.hh
 ** \brief Definition of atomic objects.
 */

#ifndef OBJECT_ATOM_HH
# define OBJECT_ATOM_HH

# include <boost/any.hpp>
# include <boost/tuple/tuple.hpp>
# include <boost/type_traits/add_reference.hpp>

# include <ast/code.hh>

namespace ast
{
  class Function;
}

# include <kernel/fwd.hh>

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
    static shared_type fresh(const value_type v, bool add_proto = true);

    /// Get a smart pointer to this
    shared_type self() const;


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
    std::ostream& special_slots_dump (std::ostream& o, runner::Runner&) const;

  protected:
    /// Protected constructor to force proper self_ initialization.
    Atom (const value_type v, bool add_proto = true);


  private:
    /// The value.
    value_type value_;
  };


  /*--------.
  | Alien.  |
  `--------*/

  // Aliens allow to store any kind of value.  For debugging purpose,
  // we also store its C++ type, hence the tuple.
  struct alien_traits
  {
    typedef boost::tuple<boost::any, std::string> type;
    enum { kind = object_kind_alien };
  };


  /*-------.
  | Code.  |
  `-------*/

  // Functions that are written in Urbi, detached from the AST
  // returned by the parser.
  struct code_traits
  {
    struct type
    {
      type(ast::rConstCode a)
        : ast(a)
      {}
      ast::rConstCode ast;
      std::vector<rObject> captures;
    };
    enum { kind = object_kind_code };
  };


  /*----------.
  | Delegate. |
  `----------*/

  struct delegate_traits
  {
    typedef IDelegate* type;
    enum { kind = object_kind_delegate };
  };


  /*------------.
  | Dictionary. |
  `------------*/

  struct dictionary_traits
  {
    typedef libport::hash_map<libport::Symbol, rObject> type;
    enum { kind = object_kind_dictionary };
  };


  /*--------.
  | Float.  |
  `--------*/

  // The sole numerical value really supported for the time being.
  struct float_traits
  {
    typedef libport::ufloat type;
    enum { kind = object_kind_float };
  };


  /*----------.
  | Integer.  |
  `----------*/

  // The future.  But requires intelligence to mix with other
  // numerical values, so not used currently.
  struct integer_traits
  {
    typedef int type;
    enum { kind = object_kind_integer };
  };


  /*-------.
  | List.  |
  `-------*/

  // Lists, not arrays.
  struct list_traits
  {
    typedef objects_type type;
    enum { kind = object_kind_list };
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
 | Primitive.  |
 `------------*/

  // Code written in C++.
  struct primitive_traits
  {
    // The type of the primitives.
    typedef rObject (*type) (runner::Runner&, objects_type);
    enum { kind = object_kind_primitive };
  };


  /*---------.
  | String.  |
  `---------*/

  // Internalized strings.  We should probably make them real
  // std::string in the future, because currently any constructed
  // string is kept endlessly.
  struct string_traits
  {
    typedef libport::Symbol type;
    enum { kind = object_kind_string };
  };

} // namespace object

#endif // !OBJECT_ATOM_HH
