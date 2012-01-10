/*
 * Copyright (C) 2010-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file object/list.hxx
 ** \brief Inline implementation for the Urbi object List.
 */

namespace urbi
{
  namespace object
  {
    inline
    List&
    List::operator<<(const rObject& e)
    {
      insertBack(e);
      return *this;
    }

  } // namespace object
}
