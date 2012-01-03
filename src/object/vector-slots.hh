/*
 * Copyright (C) 2008-2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef OBJECT_VECTOR_SLOTS_HH
# define OBJECT_VECTOR_SLOTS_HH

# include <object/slots.hh>

namespace object
{
  class VectorSlots: public Slots
  {
    public:
      typedef std::vector<std::pair<key_type, value_type> > content_type;
      typedef content_type::iterator iterator;
      typedef content_type::const_iterator const_iterator;

      inline bool set(const key_type& key, value_type v);
      inline void update(const key_type& key, value_type v);
      inline value_type get(const key_type& key) const;
      inline void erase(const key_type& key);
      inline bool has(const key_type& key) const;
      inline void where(const key_type& key) const;
      inline const content_type& container() const;

    private:
      content_type content_;
      mutable content_type::iterator it_;
      mutable bool found_;
  };
}

# include <object/vector-slots.hxx>

#endif
