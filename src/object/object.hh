/**
 ** \file object/object.hh
 ** \brief Definition of object::Object.
 */

#ifndef OBJECT_OBJECT_HH
# define OBJECT_OBJECT_HH

# include <iosfwd>
# include <set>
# include <list>
# include <algorithm>
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
	kind_object,
	kind_float,
	kind_integer,
	kind_string,
      };

    /// Return the kind of this Object.
    virtual kind_type kind_get () const;

    /// Return the kind as a string.  Used by dump.
    static const char* string_of (kind_type k);
    /// \}


    /// \name The parents.
    /// \{
    /// The parent type.
    typedef Object parent_type;
    /// The refs to parents.
    typedef rObject parent_rtype;
    /// The parents.
    typedef std::list<parent_rtype> parents_type;

    /// Add parent.
    Object& parent_add (const parent_rtype& p);
    /// Remove parent.
    Object& parent_remove (const parent_rtype& p);
    /// \}

    /// \name The slots.
    /// \{
    /// The keys to the slots.
    typedef libport::Symbol key_type;
    /// The slots.
    typedef libport::hash_map<key_type, rObject> slots_type;
    /// The slot set for lookup.
    typedef std::set<Object*> lookup_set_type;

    /// R/w access to the slots. Returns same slot as \c lookup,
    //  or create slot if \c lookup fails.
    rObject& operator[] (const key_type& k);
    /// Lookup field in object hierarchy.
    rObject& lookup (const key_type& k);
    /// Update value in slot returned by \c lookup.
    Object& update_slot (const key_type& k, rObject& o);
    /// Set slot value in local slot. Create slot if necessary.
    Object& set_slot (const key_type& k, rObject& o);
    /// Remove slot.
    Object& remove_slot (const key_type& k);
    /// \}

    /// \name Printing.
    /// \{
    /// Report the content on \p o.  For debugging purpose.
    virtual std::ostream& dump (std::ostream& o) const;

    /// Dump the special slots if there are.
    virtual std::ostream& special_slots_dump (std::ostream& o) const;
    /// \}

  private:
    /// Lookup field in object hierarchy.
    rObject& lookup (const key_type& k, lookup_set_type& lu);

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
  

  /// Report \p v on \p o.  For debugging purpose.
  std::ostream& operator<< (std::ostream& o, const Object& v);

  extern rObject object_class;
  extern rObject float_class;
  extern rObject integer_class;
  extern rObject string_class;

} // namespace object

# include "object/object.hxx"

#endif // !OBJECT_OBJECT_HH
