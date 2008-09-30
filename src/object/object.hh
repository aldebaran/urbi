/**
 ** \file object/object.hh
 ** \brief Definition of object::Object.
 */

#ifndef OBJECT_OBJECT_HH
# define OBJECT_OBJECT_HH

# include <deque>
# include <iosfwd>
# include <set>

# include <boost/function.hpp>

# include <libport/shared-ptr.hh>

# include <object/fwd.hh>
# include <object/centralized-slots.hh>
# include <object/hash-slots.hh>
# include <object/sorted-vector-slots.hh>
# include <object/vector-slots.hh>

# include <runner/fwd.hh>

namespace object
{

  /// Run time values for Urbi.
  class Object: public libport::RefCounted
  {
    /// \name Ctor & dtor.
    /// \{
  public:
    /// Create a new Object.
    Object ();

    /// Destroy a Object.
    virtual ~Object ();
    /// \}

    /// Type of the keys.
    typedef Slots::key_type key_type;

    /// Ref-couting.
    typedef libport::shared_ptr<Object> shared_type;

    /// Check whether this is of type \a T
    template<typename T>
    bool is_a() const;

    /// Convert this to type \a T
    /** \return This seen as a \a T, or 0 if it's not of type \a T
     */
    template<typename T>
    libport::shared_ptr<T> as() const;
    template<typename T>
    libport::shared_ptr<T> as();

    /// \name The protos.
    /// \{
    /// The protos.
    typedef std::deque<rObject> protos_type;

    /// Add proto.
    Object& proto_add (const rObject& p);
    /// Remove proto.
    Object& proto_remove (const rObject& p);
    /// Read only access to protos.
    const protos_type& protos_get () const;
    /// Change the whole set of protos.
    void protos_set (const rList&);
    // Urbi access to protos.
    rList urbi_protos_get ();
    /// \}

    /// \name The slots.
    /// \{
    /// The slots implementation
    typedef CentralizedSlots slots_implem;
    /// One slot.
    typedef std::set<const Object*> objects_set_type;

    /// Whether \a this has a \a k slot
    /// \param k         Slot name.
    /// \return Whether the \a k slot exists
    bool
    slot_has(const key_type& k);

    /// Lookup field in object hierarchy.
    /// \param k         Slot name.
    /// \param fallback  Whether we agree to use the "fallback" method
    ///                  when the slot does not exist.
    /// \param value     Whether to return the owner of the slot or its value
    /// \return the Object containing slot \a k if \a value is false,
    ///         the slot value if \a value is true, 0 if not found.
    rObject
    slot_locate(const key_type& k,
                bool fallback = true, bool value = false) const;

    /// Same as slot_locate, but raise LookupError if not found.
    /// \throw LookupError if the lookup fails.
    rObject
    safe_slot_locate(const key_type& k, bool value = false) const;

    /// Lookup field in object hierarchy.
    /// \param name The name of the slot to search
    /// \throw LookupError if the slot isn't found.
    rObject
    slot_get(const key_type& k) const;

    /// Implement copy-on-write if the owner of the scope is not this.
    /// Otherwise, falls-thru to own_slot_update().
    /// \param r        Runner to run the updateHook.
    /// \param k	The slot to update
    /// \param o	The new value
    /// \param hook	Whether to trigger the potential updateHook
    rObject
    slot_update(runner::Runner& r,
                const key_type& k, const rObject& o,
                bool hook = true);

    /// Update slot \c k to \a o.
    void
    own_slot_update(const key_type& k, const rObject& v);


    /// \brief Update value in slot.
    ///
    /// Set slot value in local slot.
    /// \precondition the slot does not exist in this.
    /// \return    *this.
    Object& slot_set(const key_type& k, const rObject& o);

    /// \brief Copy another object's slot.
    ///
    /// \precondition the slot does not exist in this.
    /// \precondition the slot exists in \a from.
    /// \postcondition the slot exists in \a this.
    /// \param name The name of the slot to copy
    /// \param from The object to copy the slot from
    /// \return this
    Object& slot_copy(const key_type& name, const rObject& from);

    /// Get the object pointed to by the *local* slot.
    /// An error if the slot does not exist in this object (not its
    /// protos).
    rObject own_slot_get(const key_type& k) const;

    /// Remove slot.
    Object& slot_remove(const key_type& k);
    /// Read only access to slots.
    const slots_implem& slots_get () const;

    /// Copy another object local slots, if not already present.
    void all_slots_copy(const rObject& other);

    /// \}

    /// \name Properties.
    /// \{
    /// Return the dictionary of the properties.
    /// Returns 0 if there is no slot "properties", or if it is not
    /// a Dictionary.  Must be an "own" slot.
    rDictionary properties_get();
    /// Return the dictionary of the properties of slot \a k, or 0.
    rDictionary properties_get(const key_type& k);
    /// Return the property \a p of slot \a k, or 0.
    rObject property_get(const key_type& k, const key_type& p);
    /// Return whether slot \a k has a property \a p.
    bool property_has(const key_type& k, const key_type& p);
    /// self.k->p = val.
    /// Ensures that self.property exists.
    rObject property_set(runner::Runner& r,
                         const key_type& k, const key_type& p,
			 const rObject& val);
    /// Remove property \a p from slot \a k. Returns what was removed.
    rObject property_remove(const key_type& k, const key_type& p);
    /// \}

    /// \name Printing.
    /// \{
    /// Dump the list of protos.
    /// FIXME: Should become useless when protos become Urbi lists.
    std::ostream& protos_dump(std::ostream& o,
                              runner::Runner& r) const;
    /// Report a short string describing the identity.
    std::ostream& id_dump(std::ostream& o,
                          runner::Runner& r) const;

    /// Dump the special slots if there are.
    virtual std::ostream& special_slots_dump(std::ostream& o,
                                             runner::Runner&) const;

    // Print out the value. Suitable for user interaction.
    std::ostream& print(std::ostream&, runner::Runner&) const;

    /// Report the content on \p o.  For debugging purpose.
    std::ostream& dump(std::ostream&, runner::Runner&, int depth_max) const;
    /// \}

    /// Clone, i.e., create a fresh object with this class as sole proto.
    // It is tempting to make it const, but then the list of protos
    // must be const too.
    virtual rObject clone() const;

    /// Comparison methods.
    virtual bool operator< (const Object& rhs) const;

  private:
    rObject
    slot_locate(const key_type& k,
                bool fallback,
                bool value,
                objects_set_type& marks) const;


    /// The protos.
    protos_type* protos_;

    /// The protos cache at the Urbi level.
    rObject protos_cache_;

    /// The slots.
    slots_implem slots_;
  };

  /// Call f(robj) on r and all its protos hierarchy, stop if it returns true.
  template<class F> bool for_all_protos(const rObject& r, F f);

  /// Whether \b p is present in \b c's proto hierarchy.
  bool is_a(const rObject& c, const rObject& p);

  /// Whether \a o represents a true value. WrongArgumentType will be raised
  /// with a reference to the \a fun if void is passed.
  bool is_true(const rObject& o, const libport::Symbol& fun);

  /// Return an Urbi boolean object corresponding to \a b.
  const rObject& to_boolean(bool b);

} // namespace object

# include <object/object.hxx>

#endif // !OBJECT_OBJECT_HH
