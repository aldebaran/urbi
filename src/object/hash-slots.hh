#ifndef OBJECT_HASH_SLOTS_HH
# define OBJECT_HASH_SLOTS_HH

# include <libport/hash.hh>

# include "object/slots.hh"

namespace object
{
  class HashSlots: public Slots
  {
    public:
      typedef libport::hash_map<key_type, rObject> content_type;
      typedef content_type::iterator iterator;
      typedef content_type::const_iterator const_iterator;

      inline bool set(const key_type& key, value_type v);
      inline void update(const key_type& key, value_type v);
      inline value_type get(const key_type& key) const;
      inline void erase(const key_type& key);
      inline bool has(const key_type& key) const;
      inline std::pair<bool, iterator> where(const key_type& key);
      inline const content_type& container() const;

    private:
      content_type content_;
  };
}

# include "hash-slots.hxx"

#endif
