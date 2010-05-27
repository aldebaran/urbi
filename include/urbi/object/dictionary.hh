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
 ** \file object/dictionary.hh
 ** \brief Definition of the Urbi object dictionary.
 */

#ifndef OBJECT_DICTIONARY_HH
# define OBJECT_DICTIONARY_HH

# include <libport/hash.hh>
# include <urbi/object/fwd.hh>
# include <urbi/object/cxx-object.hh>

namespace urbi
{
  namespace object
  {
    class URBI_SDK_API Dictionary: public CxxObject
    {
    public:
      typedef boost::unordered_map<libport::Symbol, rObject> value_type;

      Dictionary();
      Dictionary(rDictionary model);
      Dictionary(const value_type& value);
      const value_type& value_get() const;
      value_type& value_get();

      /// Urbi methods
      rDictionary clear();
      bool empty() const;
      size_t size() const;
      /// False iff empty.
      virtual bool as_bool() const;
      rDictionary erase(libport::Symbol key);
      rObject get(libport::Symbol key);
      bool has(libport::Symbol key) const;
      rList keys();
      rDictionary set(libport::Symbol key, rObject value);

    private:
      value_type content_;

    URBI_CXX_OBJECT_(Dictionary);
    };
  }; // namespace object
}

#endif // !OBJECT_DICTIONARY_HH
