/*
 * Copyright (C) 2010, 2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef URBI_OBJECT_HASH_HH
# define URBI_OBJECT_HASH_HH

# include <urbi/object/cxx-object.hh>

namespace urbi
{
  namespace object
  {
    class URBI_SDK_API Hash: public CxxObject
    {
    public:
      typedef std::size_t value_type;
      Hash(value_type val);
      Hash(rHash model);
      value_type value() const;
      bool operator== (rHash other) const;
      Float::value_type asFloat() const;
      rHash combine(rObject o);
      static std::size_t hash(rObject o);

    private:
      value_type val_;
      URBI_CXX_OBJECT(Hash, CxxObject);
    };

    std::size_t hash_value(const Object& o);
  }
}

#endif
