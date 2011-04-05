/*
 * Copyright (C) 2011, Gostai S.A.S.
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
    Matrix::index1(int i) const
    {
      if (i < 0)
        i += value_.size1();
      if (i < 0 || value_.size1() <= size_t(i))
        FRAISE("invalid row: %s", i);
      return i;
    }

    inline size_t
    Matrix::index2(int j) const
    {
      if (j < 0)
        j += value_.size2();
      if (j < 0 || value_.size2() <= size_t(j))
        FRAISE("invalid column: %s", j);
      return j;
    }
  }
}

#endif
