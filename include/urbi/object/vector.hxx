/*
 * Copyright (C) 2011-2012, Gostai S.A.S.
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

#define OP(Op)                                                  \
    inline                                                      \
    Vector::value_type                                          \
    Vector::operator Op(const value_type& b) const              \
    {                                                           \
      size_t s1 = size();                                       \
      size_t s2 = b.size();                                     \
      if (s1 != s2)                                             \
        FRAISE("incompatible vector sizes: %s, %s", s1, s2);    \
      return value_ Op b;                                       \
    }

    OP(+)
    OP(-)
#undef OP

#define OP(Op)                                  \
    inline                                      \
    Vector::value_type                          \
    Vector::operator Op(ufloat v) const         \
    {                                           \
      value_type res(value_);                   \
      res Op##=  v;                             \
      return res;                               \
    }

    OP(*)
    OP(/)
#undef OP

#define OP(Op)                                  \
    inline                                      \
    Vector::value_type                          \
    Vector::operator Op(ufloat f) const         \
    {                                           \
      value_type res(value_);                   \
      size_t size = res.size();                 \
      for(unsigned i = 0; i < size; ++i)        \
        res(i) Op##= f;                         \
      return res;                               \
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

    inline
    Vector*
    Vector::operator +()
    {
      return this;
    }

    inline Vector::value_type
    Vector::operator -()
    {
      return -value_;
    }

    inline bool
    Vector::operator<(const Object& that) const
    {
      return *this < that.as<Vector>()->value_;
    }

  }
}

#endif
