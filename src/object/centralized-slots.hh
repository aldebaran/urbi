#ifndef OBJECT_CENTRALIZED_SLOTS_HH
# define OBJECT_CENTRALIZED_SLOTS_HH

# include <boost/multi_index_container.hpp>
# include <boost/multi_index/global_fun.hpp>
# include <boost/multi_index/mem_fun.hpp>
# include <boost/multi_index/ordered_index.hpp>
# include <boost/multi_index/hashed_index.hpp>
# include <boost/multi_index/member.hpp>

# include <object/slots.hh>


namespace object
{

  using namespace boost::multi_index;

  class URBI_SDK_API CentralizedSlots: public Slots
  {
  public:
    typedef std::pair<Object*, libport::Symbol> location_type;
    typedef std::pair<location_type, value_type> q_slot_type;

    static Object* get_owner(const q_slot_type& slot);

    typedef multi_index_container<
      q_slot_type,
      indexed_by<

        hashed_unique<member<q_slot_type,
                             location_type,
                             &q_slot_type::first>
                      >,

                      hashed_non_unique<global_fun<const q_slot_type&,
                                                   Object*,
                                                   get_owner>
                      >

      >
    > content_type;

  typedef content_type::nth_index<0>::type loc_index_type;
  typedef content_type::nth_index<1>::type obj_index_type;

  typedef obj_index_type::iterator iterator;
  typedef obj_index_type::const_iterator const_iterator;

  static loc_index_type::iterator
  where(const Object* owner, const key_type& key);
  static void finalize(Object* owner);
  static bool set(Object* owner, const key_type& key, value_type v);
  static void update(Object* owner, const key_type& key, value_type v);
  static value_type get(const Object* owner, const key_type& key);
  static void erase(Object* owner, const key_type& key);
  static bool has(Object* owner, const key_type& key);
  static iterator begin(Object* owner);
  static iterator end(Object* owner);
  static const_iterator begin(const Object* owner);
  static const_iterator end(const Object* owner);

private:

  static content_type* content_;

  static loc_index_type& loc_index_;

  static obj_index_type& obj_index_;
};
}

# include <object/centralized-slots.hxx>

#endif
