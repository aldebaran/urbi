/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file urbi/ubinary.hxx

namespace urbi
{

  inline
  BinaryData::BinaryData()
    : data(0), size(0)
  {}

  inline
  BinaryData::BinaryData(void *d, size_t s)
    : data(d), size(s)
  {}

  inline
  void BinaryData::clear()
  {
    free(data);
  }

} // end namespace urbi
