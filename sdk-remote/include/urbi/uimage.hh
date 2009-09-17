/*
 * Copyright (C) 2007-2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file urbi/uimage.hh
#ifndef URBI_UIMAGE_HH
# define URBI_UIMAGE_HH

# include <libport/cstdlib>
# include <urbi/export.hh>

namespace urbi
{

  enum UImageFormat
  {
    IMAGE_RGB=1,     ///< RGB 24 bit/pixel
    IMAGE_YCbCr=2,   ///< YCbCr 24 bit/pixel
    IMAGE_JPEG=3,    ///< JPEG
    IMAGE_PPM=4,     ///< RGB with a PPM header
    IMAGE_UNKNOWN
  };

  /** Class encapsulating an image.

   This class does not handle its memory: the data field msut be
   freed manualy.  */
  class URBI_SDK_API UImage
  {
  public:
    /// Return an empty UImage.
    /// Not a constructor so that we can still put it in a union.
    static UImage make();

    /// Return a legible definition of imageFormat.
    const char* format_string() const;

    /// Pointer to image data.
    unsigned char* data;
    /// Image size in byte.
    size_t size;
    /// Dimensions of the image.
    size_t width, height;

    UImageFormat imageFormat;
  };


} // end namespace urbi

#endif // ! URBI_UIMAGE_HH
