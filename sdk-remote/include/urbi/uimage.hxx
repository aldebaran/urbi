/*
 * Copyright (C) 2011-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file urbi/uimage.hxx

namespace urbi
{

  /*-------------.
  | UImageImpl.  |
  `-------------*/

  inline UImageImpl::operator UImage&()
  {
    return *(UImage*)this;
  }

  inline UImageImpl::operator const UImage&() const
  {
    return *(const UImage*)this;
  }

  /*---------.
  | UImage.  |
  `---------*/

  inline UImage::UImage()
  {
    init();
  }

  inline
  UImage::UImage(const UImageImpl& us)
  {
    *this = us;
  }

  inline
  UImage& UImage::operator = (const UImageImpl& us)
  {
    this->UImageImpl::operator=(us);
    return *this;
  }

} // end namespace urbi
