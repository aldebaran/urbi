/**
 ** \file object/object.hh
 ** \brief Definition of object::Object.
 */

#ifndef OBJECT_OBJECT_HH
# define OBJECT_OBJECT_HH

# include <iosfwd>
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

    /// Report the content on \p o.  For debugging purpose.
    virtual std::ostream& print (std::ostream& o) const;

    /// \name The slots.
    /// \{
    /// The keys to the slots.
    typedef libport::Symbol key_type;
    /// The slots.
    typedef libport::hash_map<key_type, rObject> slots_type;

    /// R/w access to the slots.
    rObject& operator[] (const key_type& k);
    /// \}

  private:
    /// The slots.
    slots_type slots_;
  };

  /// Report \p v on \p o.  For debugging purpose.
  std::ostream& operator<< (std::ostream& o, const Object& v);

} // namespace object

# include "object/object.hxx"

#endif // !OBJECT_OBJECT_HH
