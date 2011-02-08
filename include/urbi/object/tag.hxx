/*
 * Copyright (C) 2010-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef OBJECT_TAG_HXX
# define OBJECT_TAG_HXX

namespace urbi
{
  namespace object
  {

    inline
    const std::string&
    Tag::name() const
    {
      return value_->name_get();
    }

    inline
    void
    Tag::name_set(const std::string& s)
    {
      value_->name_set(s);
    }

    inline
    Tag::priority_type
    Tag::priority() const
    {
      return value_->prio_get();
    }


  } // namespace object
} // namespace urbi

#endif
