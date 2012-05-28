/*
 * Copyright (C) 2009-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef OBJECT_SLOT_HH
# define OBJECT_SLOT_HH

# include <boost/preprocessor/seq/for_each.hpp>
# include <boost/preprocessor/tuple/elem.hpp>

# include <libport/allocator-static.hh>
# include <libport/attributes.hh>
# include <libport/cassert>
# include <libport/hash.hh>
# include <libport/intrusive-ptr.hh>
# include <libport/ref-counted.hh>
# include <libport/symbol.hh>

# include <urbi/object/symbols.hh>
# include <urbi/object/fwd.hh>
# include <urbi/object/cxx-object.hh>

namespace urbi
{
  using libport::ufloat;

  namespace object
  {

    class URBI_SDK_API Slot: public CxxObject
    {
    private:
      URBI_CXX_OBJECT(Slot, CxxObject);
    public:
      /// Maximum object size for the allocator
      static const size_t allocator_static_max_size;

    public:
      typedef boost::unordered_map<libport::Symbol, rObject> properties_type;

      Slot();
      Slot(const Slot& model);
      Slot(rSlot model);
      Slot(rObject& value);
      template <typename T>
      static rSlot create(const T& value);
      ~Slot();


      /*----------.
      | Setters.  |
      `----------*/
      template <typename T>
      void set(const T& value, Object* sender=0);

      void set(rObject value, Object* sender=0);
      void set(rObject value, Object* sender, libport::utime_t timestamp);
      // Setter from an uobject. Adds loop detection system to set.
      void uobject_set(rObject value, Object* sender,
                       libport::utime_t timestamp);
      // Invoked by remote uobjects
      void update_timed(rObject value, libport::utime_t timestamp);
      template <typename T>
      const T& operator=(const T& value);

      /*----------.
      | Getters.  |
      `----------*/
      Object* operator->();
      const Object* operator->() const;
      /** Get current value, calling getters if present.
       * @arg fromUObject true if the request is made by an UObject. We need
       *      this information to know how to handle bypass mode writes.
       */
      rObject value(Object* sender = 0, bool fromUObject = false) const;
      rObject init(bool fromModel = false);
      template <typename T>
      T get(Object* sender = 0);

      /*-------------.
      | Properties.  |
      `-------------*/

      /// The \a k property for this slot, or 0 if there is none.
      rObject property_get(libport::Symbol k);
      /// Whether this slot has a \a k property
      bool property_has(libport::Symbol k) const;
      /// Set the \a k property of this slot to \a value
      bool property_set(libport::Symbol k, rObject value);
      /// Remove the \a k property of this slot. Noop if it does not
      /// exist.
      void property_remove(libport::Symbol k);
      /// The properties hash map of this slot, or NULL if there are no
      /// property at all.
      properties_type* properties_get();
      /// Whether is const.
      bool constant() const;

#define URBI_OBJECT_SLOT_CACHED_PROPERTY_STORE(Elem)                    \
        (*properties_)                                                  \
        [SYMBOL_EXPAND(BOOST_PP_TUPLE_ELEM(3, 1, Elem))]                \
        = to_urbi(val)                                                  \

      rObject changed();

      // Get normalized value.
      float normalized();
      // Set normalized value
      void normalized_set(float v);

      /** Check for push-pull inconsistency and start the push-pull loop
       * if needed. See push_pull_loop for more.
       * @param first_getter pass true if the first getter was just installed
       */
       bool push_pull_check(bool first_getter = false);

       // Write to output val in split mode.
       void set_output_value(rObject v);
       // Read input val in split mode.
       rObject get_input_value();
    protected:
      // Get value, when getter or a uvalue is present.
      rObject value_special(Object* sender = 0, bool fromUObject = false) const;

      // Changed event, created on demand.
      ATTRIBUTE_RW(rObject, changed);

      /*----------------.
      | Configuration.  |
      `----------------*/
      ATTRIBUTE_RW(bool, constant);
      // Disable copy on write for this slot if false.
      ATTRIBUTE_RW(bool, copyOnWrite);
      // If true, do not bridge input to output
      ATTRIBUTE_RW(bool, split);
      // True when we are in the getter. Used for loop detection.
      ATTRIBUTE_RW(int, in_getter, , , , mutable);
      // True when we are in setter, loop detection. Per-runner flag.
      ATTRIBUTE_RW(std::vector<void*>, in_setter);
      // Enable rtp mode
      ATTRIBUTE_R(bool, rtp);
      // True if content is a uvalue
      ATTRIBUTE_RW(bool, has_uvalue);
      void rtp_set(bool v);

      /*------------.
      | Callbacks.  |
      `------------*/
      // Slot getter hook: val slot.get()
      ATTRIBUTE_Rw(rObject, get);
      // Slot setter hook: slot.set(val)
      ATTRIBUTE_Rw(rObject, set);
      // Owner object getter hook: val obj.get(slot)
      ATTRIBUTE_Rw(rObject, oget);
      // Owner object setter hook: obj.set(val, slot)
      ATTRIBUTE_Rw(rObject, oset);
      ATTRIBUTE_RW(rObject, updateHook);

      /*--------.
      | Value.  |
      `--------*/
      // Only value, or if split, inputValue, aka the command we receive.
      ATTRIBUTE_RW(rObject, value);
      // aka 'sensor' value, what we expose to the external world
      ATTRIBUTE_RW(rObject, output_value);
      // Constrain value to this type if set
      ATTRIBUTE_RW(rObject, type);
      ATTRIBUTE_RW(ufloat, timestamp);
      ATTRIBUTE_RW(ufloat, rangemax);
      ATTRIBUTE_RW(ufloat, rangemin);

      /* UObject stuff: true if we are dead, ie our owner object is gone.
       * Needed so that all the components that may hold a ref to us can
       * know nothing more will happen and let us go.
       */
      ATTRIBUTE_RW(bool, dead);

      /*-----------------.
      | Push-pull loop.  |
      `-----------------*/

      /* Set to true if a push-pull loop is running.
       * It gets activated when a getter is present, and someone registers
       * to one of the change notification mechanisms.
       * In this case the code 'loop getSlotValue("theslot")' is periodicaly
       * called.
       */
      ATTRIBUTE_RW(bool, push_pull_loop);
      /* Check if there is a push_pull_loop. Must be called when the first
       * registration occurrs on each notify-change backend(at, connections)
       * Return true if a push-pull loop was activated.
       */
      bool push_pull_loop_check();
      /// Run the push-pull loop
      rObject push_pull_loop_run(runner::Job& r);

      /*--------------.
      | Bypass mode.  |
      `--------------*/
      // Tag used to notify tasks blocked in a read on a slot in bypass mode
      ATTRIBUTE_RW(rObject, waiter_tag, , , , mutable);
      // on-demand creation of waiter_tag_
      rTag waiter_tag() const;
      // Number of runners blocked using waiter_tag.
      ATTRIBUTE_RW(unsigned int, waiter_count, , , , mutable);
      // Check and unlock getters stuck waiting for bypass-mode write.
      void check_waiters();
    };

    typedef libport::intrusive_ptr<Slot> rSlot;
  }
}

# include <urbi/object/centralized-slots.hxx>
# include <urbi/object/cxx-object.hxx>
#endif
