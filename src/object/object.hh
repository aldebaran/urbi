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
# include <boost/shared_ptr.hpp>

# include <libport/intrusive-ptr.hh>
# include <object/fwd.hh>
# include <object/centralized-slots.hh>
# include <urbi/export.hh>

namespace object
{

  /// Run time values for Urbi.
  class URBI_SDK_API Object: public libport::RefCounted
  {
    /// \name Ctor & dtor.
    /// \{
  public:
    /// Create a new Object.
    Object();

    /// Destroy a Object.
    virtual ~Object();
    /// \}

    /// The slots implementation
    typedef CentralizedSlots slots_implem;

    /// Type of the keys.
    typedef slots_implem::key_type key_type;

    /// Ref-couting.
    typedef libport::intrusive_ptr<Object> shared_type;

    /// Check whether this is of type \a T
    template<typename T>
    bool is_a() const;

    /// Whether \a this represents a true value. UnexpectedVoidError
    /// will be signalled if void is passed.
    virtual bool as_bool() const;

    /// Convert this to type \a T
    /** \return This seen as a \a T, or 0 if it's not of type \a T
     */
    template<typename T>
    libport::intrusive_ptr<T> as() const;
    template<typename T>
    libport::intrusive_ptr<T> as();

    /// \name The protos.
    /// \{
    /// The protos.
    typedef objects_type protos_type;

    /// Add proto.
    Object& proto_add(const rObject& p);
    /// Remove proto.
    Object& proto_remove(const rObject& p);
    /// Read only access to protos.
    const protos_type& protos_get() const;
    /// Change the whole set of protos.
    void protos_set(const rList&);
    // Urbi access to protos.
    rList urbi_protos_get();

    // Urbi's `Object'
    static rObject proto;
    /// \}

    /// \name The slots.
    /// \{
    /// Whether \a this has a \a k slot
    /// \param k         Slot name.
    /// \return Whether the \a k slot exists
    bool slot_has(const key_type& k) const;

    typedef std::pair<rObject, rSlot> location_type;
    /// Lookup field in object hierarchy.
    /// \param k         Slot name.
    /// \param fallback  Whether we agree to use the "fallback" method
    ///                  when the slot does not exist.
    /// \return the Object containing slot \a k and the slot,
    /// or (0, 0) if not found.
    location_type
    slot_locate(const key_type& k, bool fallback = true) const;

    /// Same as slot_locate, but raise LookupError if not found.
    /// \throw LookupError if the lookup fails.
    location_type
    safe_slot_locate(const key_type& k) const;

    /// Lookup field in object hierarchy.
    /// \param name The name of the slot to search
    /// \throw LookupError if the slot isn't found.
    rObject
    slot_get(const key_type& k) const;
    Slot&
    slot_get(const key_type& k);

    /// Implement copy-on-write if the owner of the scope is not this.
    /// \param r        Runner to run the updateHook.
    /// \param k	The slot to update
    /// \param o	The new value
    /// \param hook	Whether to trigger the potential updateHook
    rObject
    slot_update(const key_type& k, const rObject& o, bool hook = true);


    /// \brief Update value in slot.
    ///
    /// Set slot value in local slot.
    /// \precondition the slot does not exist in this.
    /// \return    *this.
    Object& slot_set(const key_type& k, rObject o, bool constant = false);
    Object& slot_set(const key_type& k, Slot* o);

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
    /// \return 0 if there is no such slot.
    rSlot local_slot_get(const key_type& k) const;

    /// Remove slot.
    Object& slot_remove(const key_type& k);
    /// Read only access to slots.
    const slots_implem& slots_get() const;

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
    rObject property_set(const key_type& k, const key_type& p,
			 const rObject& val);
    /// Remove property \a p from slot \a k. Returns what was removed.
    rObject property_remove(const key_type& k, const key_type& p);
    /// \}

    /// \name Printing.
    /// \{
    /// Dump the list of protos.
    /// FIXME: Should become useless when protos become Urbi lists.
    std::ostream& protos_dump(std::ostream& o) const;
    /// Report a short string describing the identity.
    std::ostream& id_dump(std::ostream& o) const;
    /// Report a slot and possibly its properties.
    std::ostream& slot_dump(std::ostream& o,
                            const CentralizedSlots::q_slot_type& s,
                            int depth_max) const;

    /// Dump the special slots if there are.
    virtual std::ostream& special_slots_dump(std::ostream& o) const;

    // Print out the value. Suitable for user interaction.
    std::ostream& print(std::ostream&) const;

    /// Report the content on \p o.  For debugging purpose.
    std::ostream& dump(std::ostream&, int depth_max) const;
    /// \}

    /// Clone, i.e., create a fresh object with this class as sole proto.
    // It is tempting to make it const, but then the list of protos
    // must be const too.
    virtual rObject clone() const;

    /// Comparison methods.
    virtual bool operator<(const Object& rhs) const;

    virtual bool valid_proto(const Object& o) const;

    virtual std::string type_name_get() const;

    rObject call(libport::Symbol name,
                 rObject arg1 = 0,
                 rObject arg2 = 0,
                 rObject arg3 = 0,
                 rObject arg4 = 0,
                 rObject arg5 = 0);

    rObject call(const std::string& name,
                 rObject arg1 = 0,
                 rObject arg2 = 0,
                 rObject arg3 = 0,
                 rObject arg4 = 0,
                 rObject arg5 = 0);

    rObject call(libport::Symbol name,
                 const objects_type& args);


    /*-------------.
    | Urbi methods |
    `-------------*/

    void urbi_createSlot(key_type k);
    rObject urbi_getSlot(key_type k);
    rObject urbi_locateSlot(key_type k);
    rDictionary urbi_properties(key_type slotName);
    rObject urbi_removeSlot(key_type k);
    rObject urbi_setSlot(key_type k, const rObject& o);
    rObject urbi_setConstSlot(key_type k, const rObject& o);
    rObject urbi_updateSlot(key_type k, const rObject& o);


  private:
    location_type
    slot_locate_(const key_type& k, bool fallback) const;

    /// The protos.
    protos_type* protos_;

    /// The protos cache at the Urbi level.
    rObject protos_cache_;

    /// The slots.
    slots_implem slots_;

    mutable int lookup_id_;
  };

  /// Call f(robj) on r and all its protos hierarchy, stop if it returns true.
  template<class F> bool for_all_protos(const rObject& r, F f);

  /// Whether \b p is present in \b c's proto hierarchy.
  URBI_SDK_API bool is_a(const rObject& c, const rObject& p);

  /// Same as above, but check first with a dynamic_cast in order to handle
  /// atoms more efficiently.
  template<typename T>
  URBI_SDK_API bool is_a(const rObject& c);

  /// Return an Urbi boolean object corresponding to \a b.
  URBI_SDK_API const rObject& to_boolean(bool b);

  // FIXME: we probably want libport::refcounted smart pointers here
  typedef boost::shared_ptr<rObject> rrObject;

  /// Pretty print an object (using the asString urbi method).
  std::ostream& operator << (std::ostream& s, Object& o);

} // namespace object

# include <object/object.hxx>

# ifndef OBJECT_FLOAT_CLASS_HH
#  include <object/slot.hh>
# endif

#endif // !OBJECT_OBJECT_HH
