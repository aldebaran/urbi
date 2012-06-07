/*
 * Copyright (C) 2011-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file urbi/usound.hxx
namespace urbi
{

  /*-------------.
  | USoundImpl.  |
  `-------------*/

  inline
  USound::USound(const USoundImpl& us)
  {
    *this = us;
  }

  inline
  USound& USound::operator = (const USoundImpl& us)
  {
    this->USoundImpl::operator=(us);
    return *this;
  }

  /*---------.
  | USound.  |
  `---------*/

  inline USound::USound()
  {
    init();
  }

  inline USoundImpl::operator const USound&() const
  {
    return *(const USound*)this;
  }

  inline USoundImpl::operator USound&()
  {
    return *(USound*)this;
  }

} // end namespace urbi
