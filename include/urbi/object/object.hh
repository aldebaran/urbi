/*
 * Copyright (C) 2009-2012, Gostai S.A.S.
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
# include <typeinfo>

# include <boost/function.hpp>
# include <boost/shared_ptr.hpp>

# include <libport/allocator-static.hh>
# include <libport/attributes.hh>
# include <libport/compiler.hh>
# include <libport/intrusive-ptr.hh>

# include <urbi/object/fwd.hh>
# include <urbi/object/centralized-slots.hh>
# include <urbi/export.hh>

# define URBI_ATTRIBUTE_ON_DEMAND_DECLARE(Type, Name)   \
  private:                                              \
  rObject Name##_;                                      \
public:                                                 \
libport::intrusive_ptr<Type> Name##_get() const;        \
  void Name();                                          \

# define URBI_ATTRIBUTE_ON_DEMAND_IMPL(Class, Type, Name)               \
  libport::intrusive_ptr<Type> Class::Name##_get() const                \
  {                                                                     \
    if (! Name##_)                                                      \
    {                                                                   \
      const_cast<Class*>(this)->Name##_ = new Type;                     \
    }                                                                   \
    return reinterpret_cast<Type*>(Name##_.get());                      \
  }                                                                     \
                                                                        \
  void Class::Name()                                                    \
  {                                                                     \
    if (Name##_)                                                        \
      reinterpret_cast<Type*>(Name##_.get())->call(SYMBOL(emit));       \
  }                                                                     \


namespace urbi
{
  namespace object
  {
    /// Run time values for Urbi.
    class URBI_SDK_API Object
      : public libport::RefCounted
      , public libport::StaticallyAllocated<Object, 1024 * 4>
    {
    public:
      /// Maximum object size for the allocator
      static const size_t allocator_static_max_size;

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

      /// Whether \a this represents a true value. UnexpectedVoidError
      /// will be signalled if void is passed.
      virtual bool as_bool() const;

      /// This should not be overriden.
      /// To specialized, redefine as_string.
      /// Conversion to string, using asString in Urbi, if defined.
      rString asString() const;

      /// Conversion to string, using asString in Urbi, if defined.
      virtual std::string as_string() const;

    /*--------------.
    | Conversions.  |
    `--------------*/

    public:
      /// Convert this to type \a T
      /** \return This seen as a \a T, or 0 if it's not of type \a T
       */
      template<typename T>
        ATTRIBUTE_ALWAYS_INLINE
        libport::intrusive_ptr<T> as() const;
      template<typename T>
        ATTRIBUTE_ALWAYS_INLINE
        libport::intrusive_ptr<T> as();

        /// Convert this to type \a T
      /** \return This seen as a \a T, or raise if it's not of type \a T
       */
      template<typename T>
        ATTRIBUTE_ALWAYS_INLINE
        libport::intrusive_ptr<T> as_checked() const;
      template<typename T>
        ATTRIBUTE_ALWAYS_INLINE
        libport::intrusive_ptr<T> as_checked();
    protected:
      virtual void* as_dispatch_(const std::type_info* requested);
      ATTRIBUTE_ALWAYS_INLINE
      bool as_check_(const std::type_info* requested);


    /*---------.
    | Protos.  |
    `---------*/

    public:
      /// \name The protos.
      /// \{
      /// The protos.
      typedef objects_type protos_type;

      /// Add proto.
      Object& proto_add(const rObject& p);
      /// Add proto without performing any check.
      Object& unsafe_proto_add(const rObject& v);
      /// Remove proto.
      Object& proto_remove(const rObject& p);
      /// Returns true if we have at least one proto.
      bool protos_has() const;
      /// Change the whole set of protos.
      void protos_set(const rList&);
      /// Remove all protos and set this unique object instead
      void proto_set(const rObject& o);
      /// Return the first proto
      rObject protos_get_first() const;

      // Use for_all_protos to access the list of protos in an efficient way.

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

      typedef std::pair<Object*, rObject> location_type;
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
      rObject slot_get(key_type k, bool throwOnFailure = true) const;
      rObject slot_get_value(key_type k, bool throwOnFailure = true) const;

      /// Implement low-level copy-on-write.
      /// \param name	The slot to update.
      /// \param slot	The original slot.
      /// \return  The newly created slot.
      Slot& slot_copy_on_write(key_type name, Slot& slot);

      /// Implement copy-on-write if the owner of the scope is not this.
      /// \param k	The slot to update
      /// \param o	The new value
      /// \param hook	Whether to trigger the potential updateHook
      rObject slot_update(key_type k, const rObject& o, bool hook = true);
      /// Similar to slot_update(), but with knowledge of the slot location
      rObject slot_update_with_cow(key_type k, const rObject& o,
        bool hook, location_type& r);

      /// \brief Update value in slot.
      ///
      /// Set slot value in local slot.
      /// \precondition the slot does not exist in this.
      /// \return    *this.
      Object& slot_set_value(key_type k, rObject o, bool constant = false);
      Object& slot_set(key_type k, rObject o);

      /// Create a slot with a getter and a setter.
      Object& slot_set(key_type slot, rObject getter, rObject setter);

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
      rObject local_slot_get(key_type k) const;

      /// Get the value in the *local* slot.
      /// \return 0 if there is no such slot.
      rObject local_slot_get_value(key_type k) const;

      /// Remove (local) slot.
      /// \return  Whether there was such a slot.
      bool slot_remove(key_type k);
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

      /*---------.
      | Events.  |
      `---------*/

      URBI_ATTRIBUTE_ON_DEMAND_DECLARE(Event, slotAdded);
      URBI_ATTRIBUTE_ON_DEMAND_DECLARE(Event, slotRemoved);

      /*---------------.
      | Urbi methods.  |
      `---------------*/

    public:
      void urbi_createSlot(key_type k);
      rObject getSlot(key_type k);
      // Convenience overload because Symbols don't cast to strings.
      rObject getSlot(const std::string& k);
      /// Reproduce the lookup algorithm in the implicit target case.
      rSlot findSlot(const std::string& k);
      /// Return the associated value.
      /// \throw Exception.Lookup if not available locally.
      rObject getLocalSlotValue(key_type k);
      rObject getLocalSlot(key_type k);
      rObject getProperty(const std::string& slot, const std::string& prop);
      bool hasSlot(const std::string& k);
      bool hasLocalSlot(const std::string& k);
      rObject new_(const objects_type& args);
      rObject urbi_locateSlot(key_type k);
      rDictionary urbi_properties(key_type slotName);

      /// Raise an error if k is not a local slot name.
      rObject urbi_removeLocalSlot(key_type k);

      /// Obsolete.  Same as urbi_removeLocalSlot, but make a warning
      /// instead of an error.
      rObject urbi_removeSlot(key_type k);
      rObject setProperty(const std::string& slot,
                          const std::string& prop, const rObject& value);
      rObject setSlot(key_type slot, const rObject& value);
      // Convenience overload because Symbols don't cast to strings.
      rObject setSlot(const std::string& slot, const rObject& value);
      rObject setSlotValue(const std::string& slot, const rObject& value);
      rObject getSlotValue(const std::string& slot);
      rObject urbi_setConstSlotValue(key_type k, const rObject& o);
      rObject urbi_updateSlot(key_type k, const rObject& o);
      rObject asPrintable() const;
      rObject asTopLevelPrintable() const;
      rHash   hash() const;
      rObject addProto(rObject proto);
      rObject removeProto(rObject proto);
      std::string uid() const;

      /*--------.
      | Package |
      `--------*/
    public:
      /// Get 'Package' object, where lookup starts for 'import' exps.
      static rObject package_root_get();
      /// Get the 'Lang' package, a.k.a. Package.Lang
      static rObject package_lang_get();

      /*--------.
      | Binding |
      `--------*/
    public:
      template <typename T>
      void bind(const std::string& name, T);
      typedef boost::function1<rObject, const objects_type&> function1_type;
      void bind_variadic(const std::string& name,
                         const function1_type& val);
      template <typename Return, typename Self>
      void bind_variadic(const std::string& name,
                         const boost::function2<Return, Self*,
                         const objects_type&>& val);
      template <typename Return, typename Self>
      void bind_variadic(const std::string& name,
                         Return (Self::*val)(const objects_type&));
      // Bind with a T ThisType::getter() and a void ThisType::setter(T)
      // Make the getter/setter free template to allow variations
      // like const or not, and return by const reference.
      // You can pass '0' to either the setter or getter.
      template <typename G, typename S>
      void bind(const std::string& name,
                G g,
                S s);
    private:
      template <bool mem, typename T>
      friend struct DispatchBind_;
      template <typename T>
      void bindfun_(const std::string& name, T);
      template <typename Self, typename T>
      void bindvar_(const std::string& name, T (Self::*));


      location_type slot_locate_(key_type k) const;

      /// Our proto as long as we only have one, ie protos_ = 0.
      rObject proto_;

      /// The protos.
      protos_type* protos_;

      /// The protos cache at the Urbi level.
      rObject protos_cache_;

      /// The slots.
      slots_implem slots_;

      mutable int lookup_id_;

    public:
      typedef boost::unordered_set<rObject> objects_set_type;
      template<class F> friend bool
      for_all_protos(const rObject& r, F& f, objects_set_type& objects);
      friend class CentralizedSlots;
    };

    /// Call f(robj) on r and all its protos hierarchy, stop if it returns true.
    template<class F> bool for_all_protos(const rObject& r, F f);

    /// Whether \b p is present in \b c's proto hierarchy.
    URBI_SDK_API bool is_a(const rObject& c, const rObject& p);

    /// Return an Urbi boolean object corresponding to \a b.
    URBI_SDK_API const rObject& to_boolean(bool b);

    // FIXME: we probably want libport::refcounted smart pointers here
    typedef boost::shared_ptr<rObject> rrObject;

    /// Pretty print an object (using the asString urbi method).
    std::ostream& operator<<(std::ostream& s, const Object& o);

  } // namespace object
}

/*-------.
| BIND.  |
`-------*/

// N-ary macro to bind functions.
//
// For example:
//
// BIND(foo)
//   Binds urbiscript name foo to C++ function foo.
//
//      BIND(init);
//
// BIND(foo, foo_impl)
//   Same but C++ function foo_impl.
//
//      BIND(data, data_get);
//
// BIND(foo, foo_impl, foo_type)
//   Same but static_cast C++ function foo_impl to foo_type (to
//   resolve overloading).
//
//      BIND(PERCENT, operator%, std::string(self_type::*)(const Type&) const);
//
// BIND(foo, foo_impl, foo_return, foo_args)
//   Same byt cast foo_impl as a function with foo_args (possibly
//   ending with "const") and returning type foo_return.
//
//        BIND(PERCENT, operator%, std::string, (const Type&) const);
//        BIND(dump, dump, Logger*, ())
# define BIND(...)                                                      \
  LIBPORT_CAT(BIND,                                                     \
              LIBPORT_LIST_SIZE(LIBPORT_LIST(__VA_ARGS__)))(__VA_ARGS__)

# define BIND0(Name)                            \
  BIND1(Name, Name)

# define BIND1(Name, Cxx)                       \
  bind(SYMBOL_(Name), &self_type::Cxx)

# define BIND2(Name, Cxx, Cast)                 \
  bind(SYMBOL_(Name), static_cast<Cast>(&self_type::Cxx))

# define BIND3(Name, Cxx, Return, Args)                          \
  BIND2(Name, Cxx, Return (self_type::*)Args)

# define BINDG(...)                                                      \
  LIBPORT_CAT(BINDG,                                                     \
              LIBPORT_LIST_SIZE(LIBPORT_LIST(__VA_ARGS__)))(__VA_ARGS__)

# define BINDG0(Name)                            \
  BINDG1(Name, Name)

# define BINDG1(Name, Cxx)                       \
  bind(SYMBOL_(Name), &self_type::Cxx, 0)

# define BINDG2(Name, Cxx, Cast)                 \
  bind(SYMBOL_(Name), static_cast<Cast>(&self_type::Cxx), 0)

# define BINDG3(Name, Cxx, Return, Args)                          \
  BINDG2(Name, Cxx, Return (self_type::*)Args)

/*----------------.
| BIND_VARIADIC.  |
`----------------*/

# define BIND_VARIADIC(...)                                             \
  LIBPORT_CAT(BIND_VARIADIC,                                            \
              LIBPORT_LIST_SIZE(LIBPORT_LIST(__VA_ARGS__)))(__VA_ARGS__)

# define BIND_VARIADIC0(Name)                   \
  BIND_VARIADIC1(Name, Name)

# define BIND_VARIADIC1(Name, Cxx)                       \
  bind_variadic(SYMBOL_(Name), &self_type::Cxx)

# include <urbi/object/object.hxx>

#endif // !OBJECT_OBJECT_HH
