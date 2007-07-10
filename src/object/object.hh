/**
 ** \file object/object.hh
 ** \brief Definition of object::Object.
 */

#ifndef OBJECT_OBJECT_HH
# define OBJECT_OBJECT_HH

# include <iosfwd>
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
    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct a Object.
    Object ();

    /// Destroy a Object.
    virtual ~Object ();
    /** \} */

    /// Return the kind of this Object.  Used by dump.
    virtual std::string kind_get () const;

    /// Report the content on \p o.  For debugging purpose.
    virtual std::ostream& dump (std::ostream& o) const;

    /// Dump the special slots if there are.
    virtual std::ostream& special_slots_dump (std::ostream& o) const;

    /// \name The slots.
    /// \{
    /// The keys to the slots.
    typedef libport::Symbol key_type;
    /// The slots.
    typedef libport::hash_map<key_type, rObject> slots_type;

    /// R/w access to the slots.
    rObject& operator[] (const key_type& k);
    /// Lookup field in object hierarchy.
    rObject& lookup (const key_type& k);
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

  private:
    /// The slots.
    slots_type slots_;
    /// The parents.
    parents_type parents_;
  };

  /// Report \p v on \p o.  For debugging purpose.
  std::ostream& operator<< (std::ostream& o, const Object& v);

} // namespace object

# include "object/object.hxx"

#endif // !OBJECT_OBJECT_HH
