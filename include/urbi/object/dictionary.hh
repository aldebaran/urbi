/*
 * Copyright (C) 2009-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file object/dictionary.hh
 ** \brief Definition of the Urbi object dictionary.
 */

#ifndef OBJECT_DICTIONARY_HH
# define OBJECT_DICTIONARY_HH

# include <libport/hash.hh>
# include <urbi/object/cxx-object.hh>
# include <urbi/object/fwd.hh>
# include <urbi/object/hash.hh>

namespace urbi
{
  namespace object
  {
    struct unordered_map_hash
    {
      std::size_t operator()(rObject val) const;
    };
    struct unordered_map_equal_to
    {
      bool operator()(rObject lhs, rObject rhs) const;
    };

    typedef boost::unordered_map<rObject, rObject,
                                 unordered_map_hash,
                                 unordered_map_equal_to>
    unordered_map;

    class URBI_SDK_API Dictionary: public CxxObject
    {
    public:
      typedef unordered_map value_type;

      Dictionary();
      Dictionary(rDictionary model);
      Dictionary(const value_type& value);
      const value_type& value_get() const;
      value_type& value_get();
      void key_check(rObject key) const;

      /// Urbi methods
      rDictionary clear();
      bool empty() const;
      size_t size() const;
      /// False iff empty.
      virtual bool as_bool() const;
      rDictionary erase(rObject key);
      rObject get(rObject key);
      bool has(rObject key) const;
      rList keys();
      rDictionary set(rObject key, rObject value);

    private:
      value_type content_;
      URBI_CXX_OBJECT(Dictionary, CxxObject);
      URBI_ATTRIBUTE_ON_DEMAND_DECLARE(Event, elementAdded);
      URBI_ATTRIBUTE_ON_DEMAND_DECLARE(Event, elementChanged);
      URBI_ATTRIBUTE_ON_DEMAND_DECLARE(Event, elementRemoved);
    };
  }; // namespace object
}

#endif // !OBJECT_DICTIONARY_HH
