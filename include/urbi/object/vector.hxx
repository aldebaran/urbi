/*
 * Copyright (C) 2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef URBI_OBJECT_VECTOR_HXX
# define URBI_OBJECT_VECTOR_HXX

# include <urbi/object/vector.hh>

namespace urbi
{
  namespace object
  {

#define OP(Op)                                          \
    inline                                              \
    rVector Vector::operator Op(const rVector& b) const \
    {                                                   \
      return new Vector(value_ Op b->value_);           \
    }

    OP(+)
    OP(-)
#undef OP

#define OP(Op)                                  \
    inline                                      \
    rVector Vector::operator Op(ufloat v) const \
    {                                           \
      rVector res(new Vector(value_));          \
      res->value_ Op##=  v;                     \
      return res;                               \
    }

    OP(*)
    OP(/)
#undef OP

#define OP(Op)                                             \
    inline                                                 \
    rVector Vector::operator Op(ufloat f) const            \
    {                                                      \
      rVector res(new Vector(value_));                     \
      Vector::value_type& v = res->value_get();            \
      size_t size = v.size();                              \
      for(unsigned i = 0; i < size; ++i)                   \
        v(i) Op##= f;                                      \
      return res;                                          \
    }

    OP(+)
    OP(-)
#undef OP

    inline
    const Vector::value_type&
    Vector::value_get() const
    {
      return value_;
    }

    inline
    Vector::value_type&
    Vector::value_get()
    {
      return value_;
    }

    inline rVector
    Vector::operator +()
    {
      return this;
    }

    inline rVector
    Vector::operator -()
    {
      return new Vector(-value_);
    }

    inline bool
    Vector::operator<(const Object& bb) const
    {
      // FIXME: BOGUS! asymetrical!
      const rVector b = bb.as<Vector>();
      if (!b)
        return this->Object::operator<(bb);
      if (value_.size() != b->value_.size())
        return value_.size() < b->value_.size();
      for (unsigned int i=0; i<value_.size(); ++i)
        if (value_(i) != b->value_(i))
          return value_(i) < b->value_(i);
      return false;
    }
  }
}

#endif
