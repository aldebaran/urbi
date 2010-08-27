/*
 * Copyright (C) 2009, 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file urbi/object/object.hh
 ** \brief Definition of object::Object.
 */

#ifndef OBJECT_OBJECT_HH
# define OBJECT_OBJECT_HH

# include <deque>
# include <iosfwd>
# include <set>

# include <boost/function.hpp>
# include <boost/shared_ptr.hpp>

# include <libport/attributes.hh>
# include <libport/intrusive-ptr.hh>

# include <urbi/object/fwd.hh>
# include <urbi/object/centralized-slots.hh>
# include <urbi/export.hh>

namespace urbi
{
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

      /// This should not be overriden.
      /// To specialized, redefine as_string.
      /// Conversion to string, using asString in Urbi, if defined.
      rString asString() const;

      /// Conversion to string, using asString in Urbi, if defined.
      virtual std::string as_string() const;

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
      bool slot_has(key_type k) const;

      typedef std::pair<Object*, rSlot> location_type;
      /// Lookup field in object hierarchy.
      /// \param k         Slot name.
      /// \param fallback  Whether we agree to use the "fallback" method
      ///                  when the slot does not exist.
      /// \return the Object containing slot \a k and the slot,
      /// or (0, 0) if not found.
      location_type slot_locate(key_type k, bool fallback = true) const;

      /// Same as slot_locate, but raise Exception.Lookup if not found.
      /// \throw Exception.Lookup if the lookup fails.
      location_type safe_slot_locate(key_type k) const;

      /// Lookup field in object hierarchy.
      /// \param name The name of the slot to search
      /// \throw Exception.Lookup if the slot isn't found.
      rObject slot_get(key_type k) const;
      Slot& slot_get(key_type k);

      /// Implement copy-on-write if the owner of the scope is not this.
      /// \param r        Runner to run the updateHook.
      /// \param k	The slot to update
      /// \param o	The new value
      /// \param hook	Whether to trigger the potential updateHook
      rObject slot_update(key_type k, const rObject& o, bool hook = true);


      /// \brief Update value in slot.
      ///
      /// Set slot value in local slot.
      /// \precondition the slot does not exist in this.
      /// \return    *this.
      Object& slot_set(key_type k, rObject o, bool constant = false);
      Object& slot_set(key_type k, Slot* o);

      /// \brief Copy another object's slot.
      ///
      /// \precondition the slot does not exist in this.
      /// \precondition the slot exists in \a from.
      /// \postcondition the slot exists in \a this.
      /// \param name The name of the slot to copy
      /// \param from The object to copy the slot from
      /// \return this
      Object& slot_copy(key_type name, const rObject& from);

      /// Get the object pointed to by the *local* slot.
      /// \return 0 if there is no such slot.
      rSlot local_slot_get(key_type k) const;

      /// Remove slot.
      Object& slot_remove(key_type k);
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
      rDictionary properties_get(key_type k);
      /// Return the property \a p of slot \a k, or 0.
      rObject property_get(key_type k, key_type p);
      /// Return whether slot \a k has a property \a p.
      bool property_has(key_type k, key_type p);
      /// self.k->p = val.
      /// Ensures that self.property exists.
      rObject property_set(key_type k, key_type p,
                           const rObject& val);
      /// Remove property \a p from slot \a k. Returns what was removed.
      rObject property_remove(key_type k, key_type p);
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

      /// Invoke \a name on \a this, passing additional arguments.
      /// Pushes arg0 (this) before the other arguments.
      rObject call(libport::Symbol name,
                   rObject arg1 = 0,
                   rObject arg2 = 0,
                   rObject arg3 = 0,
                   rObject arg4 = 0,
                   rObject arg5 = 0);

      /// Invoke \a name on \a this, passing additional arguments.
      /// Pushes arg0 (this) before the other arguments.
      rObject call(const std::string& name,
                   rObject arg1 = 0,
                   rObject arg2 = 0,
                   rObject arg3 = 0,
                   rObject arg4 = 0,
                   rObject arg5 = 0);

      // /// Invoke \a name on \a this, passing additional arguments.
      rObject call(libport::Symbol name,
                   const objects_type& args);

      /// Invoke \a name on \a this, passing additional arguments.
      /// \a this is used to look for name, but it is not passed
      /// as arg0.  UrbiScript's "this" must be args[0].
      rObject call_with_this(libport::Symbol name, const objects_type& args);

      /*------------------.
      | 'changed' event.  |
      `------------------*/

    public:
      /// Fire the 'changed' event.
      void changed();

    private:
      ATTRIBUTE_r(rEvent, changed);

      /*---------------.
      | Urbi methods.  |
      `---------------*/

    public:
      void urbi_createSlot(key_type k);
      rObject getSlot(key_type k);
      // Convenience overload because Symbols don't cast to strings.
      rObject getSlot(const std::string& k);
      /// Return the associated value.
      /// \throw Exception.Lookup if not available locally.
      rObject getLocalSlot(key_type k);
      rObject getProperty(const std::string& slot, const std::string& prop);
      rObject urbi_locateSlot(key_type k);
      rDictionary urbi_properties(key_type slotName);
      rObject urbi_removeSlot(key_type k);
      rObject setProperty(const std::string& slot, const std::string& prop, const rObject& value);
      rObject setSlot(key_type slot, const rObject& value);
      // Convenience overload because Symbols don't cast to strings.
      rObject setSlot(const std::string& slot, const rObject& value);
      rObject urbi_setConstSlot(key_type k, const rObject& o);
      rObject urbi_updateSlot(key_type k, const rObject& o);
      rObject asPrintable() const;
      rObject asToplevelPrintable() const;

      /*--------.
      | Binding |
      `--------*/
    public:
      template <typename T>
      void bind(const std::string& name, T);
      template <typename F1, typename F2>
      void bind(const std::string& getter_name, F1 getter, const std::string& setter_name, F2 setter);
    private:
      template <bool mem, typename T>
      friend class DispatchBind_;
      template <typename T>
      void bindfun_(const std::string& name, T);
      template <typename Self, typename T>
      void bindvar_(const std::string& name, T (Self::*));


      location_type slot_locate_(key_type k, bool fallback) const;

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
    std::ostream& operator<<(std::ostream& s, const Object& o);

  } // namespace object
}

# include <urbi/object/object.hxx>

# ifndef OBJECT_FLOAT_HH
#  include <urbi/object/slot.hh>
# endif

#endif // !OBJECT_OBJECT_HH
