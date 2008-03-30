/**
 ** \file object/object.hh
 ** \brief Definition of object::Object.
 */

#ifndef OBJECT_OBJECT_HH
# define OBJECT_OBJECT_HH

# include <iosfwd>
# include <set>
# include <list>

# include <boost/function.hpp>
# include <boost/optional.hpp>

# include "libport/hash.hh"
# include "libport/shared-ptr.hh"
# include "libport/symbol.hh"
# include "libport/weak-ptr.hh"
# include "object/fwd.hh"
# include "runner/fwd.hh"

namespace object
{

  /// Run time values for Urbi.
  class Object
  {
    /// \name Ctor & dtor.
    /// \{
  public:
    /// Create a new object.
    static rObject fresh();

    /// Get a smart pointer to this
    rObject self() const;

    /// Destroy a Object.
    virtual ~Object ();
    /// \}

    /// Ref-couting.
    typedef libport::shared_ptr<Object> shared_type;

    /// \name Kind
    /// \{
    /// The kinds of primitive objects.
    enum kind_type
      {
#define CASE(What, Name) kind_ ## What,
	APPLY_ON_ALL_PRIMITIVES(CASE)
#undef CASE
      };

    /// Return the kind of this Object.
    virtual kind_type kind_get () const;

    /// Return the kind as a string.  Used by dump.
    static const char* string_of (kind_type k);

    /// Whether kind == \a k.
    bool kind_is(Object::kind_type k) const;

    /// Whether \a Type has the same kind as \a this.
    /// Very similar to testting via dynamic_cast, might not be faster.
    template <typename Type> bool type_is() const;
    /// \}


    /// \name The protos.
    /// \{
    /// The protos.
    typedef std::list<rObject> protos_type;

    /// Add proto.
    Object& proto_add (const rObject& p);
    /// Remove proto.
    Object& proto_remove (const rObject& p);
    /// Read only access to protos.
    const protos_type& protos_get () const;
    /// \}

    /// \name The slots.
    /// \{
    /// The keys to the slots.
    typedef libport::Symbol key_type;
    /// The slots.
    typedef libport::hash_map<key_type, rObject> slots_type;
    /// One slot.
    typedef std::pair<const key_type, rObject> slot_type;
    typedef std::set<const Object*> objects_set_type;

    /// Abstract lookup traversal
    ///
    /// Traverse the inheritance hierarchy, calling \a action on each
    /// encountered object. If the lookup is successful, \a action
    /// should return the result. Otherwise, it should return an empty
    /// boost::optional, so as the lookup continues. This enables to
    /// perform different kind of searches (slot_locate, target
    /// lookup) without duplicating the traversal algorithm.
    ///
    /// \param action The search to apply on each encountered object.
    /// \return The result returned by \a action, or an empty
    /// boost::optional if the lookup failed.
    template <typename R>
    boost::optional<R>
    lookup(boost::function1<boost::optional<R>,
                            rObject> action) const;

    /// Lookup helper, with a mark table
    template <typename R>
    boost::optional<R>
    lookup(boost::function1<boost::optional<R>,
                            rObject> action,
           objects_set_type& marks) const;

    /// Lookup field in object hierarchy.
    /// \return the Object containing slot \b k, or 0 if not found.
    rObject slot_locate (const Object::key_type& k) const;

    /// Same as slot_locate, but raise LookupError if not found.
    /// \throw LookupError if the lookup fails.
    rObject safe_slot_locate (const Object::key_type& k) const;


    /// Lookup field in object hierarchy.
    const rObject& slot_get (const key_type& k) const;
    rObject& slot_get (const key_type& k);

    /// \brief Update value in slot.
    ///
    /// Set slot value in local slot.
    /// \precondition the slot does not exist in this.
    Object& slot_set (const key_type& k, rObject o);

    /// Get the object pointed to by the *local* slot.
    /// An error if the slot does not exist in this object (not its
    /// protos).
    const rObject& own_slot_get (const key_type& k) const;
    rObject& own_slot_get (const key_type& k);

    /// Get the object pointed to by the *local* slot
    /// or \a def if it does not exist locally.
    rObject own_slot_get (const key_type& k, rObject def);

    /// Remove slot.
    Object& slot_remove (const key_type& k);
    /// Read only access to slots.
    const slots_type& slots_get () const;
    /// \}

    /// \name Properties.
    /// \{
    /// Whether this is an anynomous object containing local variables.
    bool locals_get () const;
    /// Change whether is a locals object.
    /// \return this
    Object& locals_set (bool b);
    /// \}

    /// \name Printing.
    /// \{
    /// Report a short string describing the identity.
    std::ostream& id_dump (const rObject& self,
                           std::ostream& o,
                           runner::Runner& r);

    /// Dump the special slots if there are.
    virtual std::ostream& special_slots_dump (runner::Runner&, rObject r,
					      std::ostream& o) const;
    /// \}

    /// Clone, i.e., create a fresh object with this class as sole proto.
    // It is tempting to make it const, but then the list of protos
    // must be const too.
    // Not a member function because we want the shared_ptr, which
    // is not available via this.
    virtual rObject clone () const;

    /// Comparison methods.
    virtual bool operator< (const Object& rhs) const;

    /// Create a fresh scope
    /**
     ** \param parent The parent scope (optional, in which case lobby
     **               becomes the parent)
     ** \return The new scope
     */
    static rObject make_scope(const rObject& parent);

    /// Make a 'do ... {}' scope
    /**
     ** Similar to \see make_method_scope, but make the scope forward more
     ** messages to 'self'.
     ** \return The new scope
     **/
    static rObject make_do_scope(const rObject& parent, const rObject& self);

    /// Make a method outer scope
    /**
     ** Similar to \see make_scope, but make the scope forward
     ** messages to self when relevant
     ** \return The new scope
     **/
    static rObject make_method_scope(const rObject& self);

  protected:
    /// Protected constructor to force proper self_ initialization.
    Object ();
    /// Weak pointer to be able to retrieve smart pointer to this
    libport::weak_ptr<Object> self_;

  private:
    typedef std::pair<bool, rObject> locate_type;
    /// Lookup field in object hierarchy.
    /// \param k   the key looked up for
    /// \param os  the objects already looked up for, to break infinite
    ///            recursions
    /// \return (false,0) if k does not exist, (true,0) if k is in this,
    ///          (true, ptr) if k is in ptr.
    locate_type slot_locate (const key_type& k, objects_set_type& os) const;

  private:
    /// The protos.
    protos_type protos_;
    /// The slots.
    slots_type slots_;
    /// Whether is a locals object.
    bool locals_;

    friend void slot_update(runner::Runner& r,
			    rObject& ref,
			    const Object::key_type& k,
			    rObject o);
    friend std::ostream& dump(runner::Runner&, rObject, std::ostream&);
  };


  /// Target lookup
  rObject target(rObject where, const libport::Symbol& name);


  /// Helpers to call Urbi functions from C++
  rObject urbi_call(runner::Runner& r,
                    rObject& self,
                    libport::Symbol msg);
  rObject urbi_call(runner::Runner& r,
                    rObject& self,
                    libport::Symbol msg,
                    objects_type& args);

  /// If the target is a "real" object, then updating means the same
  /// as slot_set: one never updates a proto.  If the target is a
  /// "locals" object, then updating really means updating the
  /// existing slot, not creating a new slot in the inner scope.
  /// Except if the existing source slot is a "real" object, in which case
  /// updating means creating the slot in the result of "self" evaluation.
  void slot_update (runner::Runner& r,
		    rObject& ref,
		    const Object::key_type& k,
		    rObject o);

  // Print out the value. Suitable for user interaction.
  std::ostream& print(runner::Runner&, rObject, std::ostream&);

  /// Report the content on \p o.  For debugging purpose.
  std::ostream& dump(runner::Runner&, rObject, std::ostream&);


  /// Call f(robj) on r and all its protos hierarchy, stop if it returns true.
  template<class F> bool for_all_protos(rObject& r, F& f);

  /// Checks in \b c's proto hierarchy if \b p is present.
  bool is_a(const rObject& c, const rObject& p);

} // namespace object

/// Return the value of the shared_ptr \a Obj as an Atom<Type>.
// FIXME: Bad name (space).
// A macro, not a function, because I want to work on the ref-pointer.
// But I might be wrong: the problem is that when I have to different
// pointer types to the same object, they should share the ref counter.
// But is it really the case with our ref ptr?  Be careful, be very
// careful :(  To be checked thoroughly.
#define VALUE(Obj, Type)			\
  ((Obj).unsafe_cast<Type>()->value_get())

/// Whether the truth value of \a Obj is true.
// We don't have Booleans currently: 0 is false, everything else
// is true.  The day we have a Nil, it should evaluate to false too.
// FIXME: Rounding errors on 0?
#define IS_TRUE(Obj)					\
  (!(Obj)->type_is<object::Float>()			\
   || (Obj).unsafe_cast<object::Float>()->value_get())


# include "object/object.hxx"

#endif // !OBJECT_OBJECT_HH
