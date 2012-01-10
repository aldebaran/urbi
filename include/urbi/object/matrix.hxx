/*
 * Copyright (C) 2011-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef URBI_OBJECT_MATRIX_HXX
# define URBI_OBJECT_MATRIX_HXX

# include <urbi/object/matrix.hh>

namespace urbi
{
  namespace object
  {
    inline
    const Matrix::value_type&
    Matrix::value_get() const
    {
      return value_;
    }

    inline
    Matrix::value_type&
    Matrix::value_get()
    {
      return value_;
    }

    inline size_t
    Matrix::size1() const
    {
      return value_.size1();
    }

    inline size_t
    Matrix::size2() const
    {
      return value_.size2();
    }

    inline size_t
    Matrix::index1(int i) const
    {
      int res = i;
      if (res < 0)
        res += size1();
      if (res < 0 || size1() <= size_t(res))
        FRAISE("invalid row: %s", i);
      return res;
    }

    inline size_t
    Matrix::index2(int j) const
    {
      int res = j;
      if (res < 0)
        res += size2();
      if (res < 0 || size2() <= size_t(res))
        FRAISE("invalid column: %s", j);
      return res;
    }
  }
}

#endif
