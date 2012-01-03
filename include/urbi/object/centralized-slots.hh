/*
 * Copyright (C) 2009-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef OBJECT_CENTRALIZED_SLOTS_HH
# define OBJECT_CENTRALIZED_SLOTS_HH

// BMI includes boost/foreach.hpp, which is troublesome (read
// libport/foreach.hh).  Include this header to neutralize the issue.
# include <libport/foreach.hh>

# include <boost/multi_index_container.hpp>
# include <boost/multi_index/global_fun.hpp>
# include <boost/multi_index/mem_fun.hpp>
# include <boost/multi_index/ordered_index.hpp>
# include <boost/multi_index/hashed_index.hpp>
# include <boost/multi_index/member.hpp>

# include <urbi/object/slot.hh>

namespace urbi
{
  namespace object
  {
    using namespace boost::multi_index;

    class URBI_SDK_API CentralizedSlots
    {

      /*---------------.
      | Type aliases.  |
      `---------------*/

    public:
      /// The slot type
      typedef rSlot value_type;
      /// The key type
      typedef libport::Symbol key_type;
      /// The location of a slot
      typedef std::pair<Object*, libport::Symbol> location_type;
      /// A slot and its location
      typedef std::pair<location_type, value_type> q_slot_type;

    private:
      /// Boost multi index helper
      static Object* get_owner(const q_slot_type& slot);
      /// The boost multi index
      typedef multi_index_container<
        q_slot_type,
        indexed_by<hashed_unique<member<q_slot_type,
                                        location_type,
                                        &q_slot_type::first> >,
                   hashed_non_unique<global_fun<const q_slot_type&,
                                               Object*,
                                               get_owner> > > >
        content_type;

      /// The location-wise index type
      typedef content_type::nth_index<0>::type loc_index_type;
      /// The owner-wise index type
      typedef content_type::nth_index<1>::type obj_index_type;

    public:
      /// The iterator type
      typedef obj_index_type::iterator iterator;
      /// The const iterator type
      typedef obj_index_type::const_iterator const_iterator;


      /*------.
      | API.  |
      `------*/

    public:
      /// Get a begin iterator.
      static iterator begin(Object* owner);
      /// Get a begin const iterator.
      static const_iterator begin(const Object* owner);
      /// Dispose of the slots of \a owner.
      static void finalize(Object* owner);
      /// Get a past-the-end iterator.
      static iterator end(Object* owner);
      /// Get a past-the-end cosnt iterator.
      static const_iterator end(const Object* owner);
      /// Erase \a owner's \a key slot.
      /// @return Success status.
      ///         I.e., false if the slot was not defined (entailing failure).
      static bool erase(Object* owner, const key_type& key);
      /// Get \a owner's \a key slot's value.
      static value_type get(const Object* owner, const key_type& key);
      /// Return whether \a owner has a \a key slot.
      static bool has(Object* owner, const key_type& key);

      /// Set \a owner's \a key slot's value to \a v.
      /// @return Success status.
      ///         I.e., false if the slot was already defined (entailing failure).
      static bool set(Object* owner,
		      const key_type& key, value_type v, bool overwrite = false);


      /*----------.
      | Helpers.  |
      `----------*/

    private:
      static loc_index_type::iterator
        where(const Object* owner, const key_type& key);


      /*----------.
      | Members.  |
      `----------*/

    private:
      /// The boost multi index.
      static content_type* content_;
      /// The location-wise index.
      static loc_index_type& loc_index_;
      /// The owner-wise index.
      static obj_index_type& obj_index_;
    };
  }
}

# include <urbi/object/centralized-slots.hxx>

#endif
