/*
 * Copyright (C) 2009, 2010, Gostai S.A.S.
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

  /*---------------.
  | UImageFormat.  |
  `---------------*/

  static const char* formats[] =
  {
    "image_unknown",
    "rgb",
    "YCbCr",
    "jpeg",
    "ppm",
    "YUV422",
    "grey8",
    "grey4",
    "image_unknown",
  };

  const char*
  format_string(UImageFormat fmt)
  {
    int f = static_cast<int>(fmt);
    if (0 <= f && f <= int(sizeof formats / sizeof *formats))
      return formats[f];
    return "image_unknown";
  }

  /*---------.
  | UImage.  |
  `---------*/

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
    return ::urbi::format_string(imageFormat);
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
