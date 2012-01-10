/*
 * Copyright (C) 2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <urbi/object/symbols.hh>
#include <urbi/object/hash.hh>

namespace urbi
{
  namespace object
  {
    Hash::Hash(value_type val)
      : val_(val)
    {
      proto_add(proto);
    }

    Hash::Hash(rHash model)
      : val_(model->val_)
    {
      proto_add(model);
    }

    Hash::value_type
    Hash::value() const
    {
      return val_;
    }

    bool
    Hash::operator== (rHash other) const
    {
      return val_ == other->val_;
    }

    Float::value_type
    Hash::asFloat() const
    {
      return val_;
    }

    std::size_t
    hash_value(const Object& o)
    {
      return from_urbi<rHash>(const_cast<Object&>(o).call("hash"))->value();
    }

    rHash
    Hash::combine(rObject o)
    {
      boost::hash_combine(val_, *o);
      return this;
    }

    URBI_CXX_OBJECT_INIT(Hash)
      : val_(boost::hash_value(this))
    {
      bind(SYMBOL(EQ_EQ),   &Hash::operator ==);
      bind(SYMBOL(asFloat), &Hash::asFloat);
      bind(SYMBOL(combine), &Hash::combine);
    }
  }
}

