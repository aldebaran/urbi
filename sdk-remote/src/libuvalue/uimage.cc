/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file libuvalue/uimage.cc

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

  static const char* formats[] = {
      "rgb",
      "YCbCr",
      "jpeg",
      "ppm",
      "YUV422",
      "grey8",
      "grey4",
      "image_unknown",
      ""
    };

  const char*
  UImage::format_string() const
  {
    int f = static_cast<int>(imageFormat);
    if (f<=0 || f> IMAGE_UNKNOWN)
      return "image_unknown";
    return formats[f-1];
  }

  UImageFormat
  parse_image_format(const std::string& s)
  {
    for (unsigned i = 0; formats[i][0]; ++i)
      if (s == formats[i])
        return static_cast<UImageFormat>(i+1);
    return IMAGE_UNKNOWN;
  }
} // namespace urbi
