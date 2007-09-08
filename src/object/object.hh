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
    /// \}


    /// \name The parents.
    /// \{
    /// The refs to parents.
    typedef rObject parent_type;
    /// The parents.
    typedef std::list<parent_type> parents_type;

    /// Add parent.
    Object& parent_add (const parent_type& p);
    /// Remove parent.
    Object& parent_remove (const parent_type& p);
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
    /// The slot set for lookup.
    typedef std::set<const Object*> lookup_set_type;

    /// R/w access to the slots. Returns same slot as \c lookup,
    //  or create slot if \c lookup fails.
    rObject& operator[] (const key_type& k);
    /// Lookup field in object hierarchy.
    const rObject& lookup (const key_type& k) const;
    rObject& lookup (const key_type& k);
    /// Update value in slot returned by \c lookup.
    Object& slot_update (const key_type& k, rObject o);
    /// Set slot value in local slot. Create slot if necessary.
    Object& slot_set (const key_type& k, rObject o);
    /// Remove slot.
    Object& slot_remove (const key_type& k);
    /// Read only access to slots.
    const slots_type& slots_get () const;
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

  private:
    /// Lookup field in object hierarchy.
    const rObject& lookup (const key_type& k, lookup_set_type& lu) const;

    /// The parents.
    parents_type parents_;
    /// The slots.
    slots_type slots_;
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

# include "object/object.hxx"

#endif // !OBJECT_OBJECT_HH
