/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file object/float.hxx
 ** \brief Inline implementation of urbi::object::Float.
 */

namespace urbi
{
  namespace object
  {

    inline
    bool
    Float::as_bool() const
    {
      return value_;
    }

    inline
    Float::integer_type
    Float::as_integer() const
    {
      return libport::numeric_cast<integer_type>(value_);
    }

    inline
    Float::value_type
    Float::inf()
    {
      return std::numeric_limits<libport::ufloat>::infinity();
    }

    inline
    bool
    Float::is_inf() const
    {
      return std::isinf(value_);
    }

    inline
    bool
    Float::is_integer() const
    {
      return libport::numeric_castable<integer_type>(value_);
    }

    inline
    bool
    Float::is_nan() const
    {
      return std::isnan(value_);
    }

    inline
    Float::value_type
    Float::nan()
    {
      return std::numeric_limits<libport::ufloat>::quiet_NaN();
    }


  } // namespace object
}
