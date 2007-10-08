/**
 ** \file object/object.hh
 ** \brief Definition of object::Object.
 */

#ifndef OBJECT_OBJECT_HH
# define OBJECT_OBJECT_HH

# include <iosfwd>
# include <set>
# include <list>

# include "libport/hash.hh"
# include "libport/symbol.hh"
# include "object/fwd.hh"

namespace object
{

  /// Run time values for Urbi.
  class Object
  {
    /// \name Ctor & dtor.
    /// \{
  public:
    /// Construct a Object.
    Object ();

    /// Destroy a Object.
    virtual ~Object ();
    /// \}


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


    /// \name The parents.
    /// \{
    /// The parents.
    typedef std::list<rObject> parents_type;

    /// Add parent.
    Object& parent_add (const rObject& p);
    /// Remove parent.
    Object& parent_remove (const rObject& p);
    /// Read only access to parents.
    const parents_type& parents_get () const;
    /// \}

    /// \name The slots.
    /// \{
    /// The keys to the slots.
    typedef libport::Symbol key_type;
    /// The slots.
    typedef libport::hash_map<key_type, rObject> slots_type;
    /// One slot.
    typedef std::pair<const key_type, rObject> slot_type;
    /// A set of objects.
    typedef std::set<const Object*> objects_type;

    /// Find the Object which defines k.
    /// \return 0 if none found.
    const Object* which (const key_type& k) const;

    /// Lookup field in object hierarchy.
    const rObject& slot_get (const key_type& k) const;
    rObject& slot_get (const key_type& k);

    /// \brief Update value in slot.
    ///
    /// If the target is a "real" object, then updating means the same
    /// as slot_set: one never updates a parent.  If the target is a
    /// "locals" object, then updating really means updating the
    /// existing slot, not creating a new slot in the inner scope.
    Object& slot_update (const key_type& k, rObject o);

    /// Set slot value in local slot.
    /// \precondition the slot does not exist in this.
    Object& slot_set (const key_type& k, rObject o);

    /// Get the object pointed to by the *local* slot.
    /// An error if the slot does not exist in this object (not its
    /// parents).
    const rObject& own_slot_get (const key_type& k) const;
    rObject& own_slot_get (const key_type& k);

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
    std::ostream& id_dump (std::ostream& o) const;

    /// Report the content on \p o.  For debugging purpose.
    virtual std::ostream& dump (std::ostream& o) const;

    /// Print out the value. Suitable for user interaction.
    virtual std::ostream& print (std::ostream& o) const;

    /// Dump the special slots if there are.
    virtual std::ostream& special_slots_dump (std::ostream& o) const;
    /// \}

    /// Comparison methods.
    virtual bool operator< (const Object& rhs) const;

  private:
    /// Lookup field in object hierarchy.
    /// \param k   the key looked up for
    /// \param os  the objects already looked up for, to break infinite
    ///            recursions
    /// \return 0  if k does not exist.
    const Object* which (const key_type& k, objects_type& os) const;

    /// The parents.
    parents_type parents_;
    /// The slots.
    slots_type slots_;
    /// Whether is a locals object.
    bool locals_;
  };

  /// Clone, i.e., create a fresh object with this class as sole parent.
  // It is tempting to make it const, but then the list of parents
  // must be const too.
  // Not a member function because we want the shared_ptr, which
  // is not available via this.
  rObject clone (rObject ref);

  /// Report Object \p v on \p o.  For debugging purpose.
  std::ostream& operator<< (std::ostream& o, const Object& v);

  /// Report slot \p s on \p o.  For debugging purpose.
  std::ostream& operator<< (std::ostream& o, const Object::slot_type& s);

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
