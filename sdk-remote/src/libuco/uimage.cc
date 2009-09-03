/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */
/// \file libuco/uimage.cc

#include <urbi/uimage.hh>

namespace urbi
{
  UImage
  UImage::make()
  {
    UImage res;
    res.data = 0;
    res.size = res.width = res.height = 0;
    res.imageFormat = IMAGE_UNKNOWN;
    return res;
  }

  const char*
  UImage::format_string() const
  {
    switch (imageFormat)
    {
    case IMAGE_RGB:
      return "rgb";
    case IMAGE_JPEG:
      return "jpeg";
    case IMAGE_YCbCr:
      return "YCbCr";
    case IMAGE_PPM:
      return "ppm";
    case IMAGE_UNKNOWN:
      return "unknown format";
    }
    // To pacify "warning: control reaches end of non-void function".
    // pabort(imageFormat);
    // FIXME: This should not abort. UImage should be initialized with IMAGE_UKNOWN.
    //        This is not done because data is stored in an union in UBinary and
    //        union members cannot have constructors.
    return "unknown format";
  }

} // namespace urbi
