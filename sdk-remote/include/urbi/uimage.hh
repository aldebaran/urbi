/*
 * Copyright (C) 2007-2011, Gostai S.A.S.
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

  /*---------------.
  | UImageFormat.  |
  `---------------*/

  enum UImageFormat
  {
    IMAGE_UNKNOWN = 0,
    IMAGE_RGB = 1,     ///< RGB 24 bit/pixel
    IMAGE_YCbCr = 2,   ///< YCbCr 24 bit/pixel
    IMAGE_YUV = 2,     ///< Alternate name for YCbCr
    IMAGE_JPEG = 3,    ///< JPEG
    IMAGE_PPM = 4,     ///< RGB with a PPM header
    IMAGE_YUV422 = 5,  ///< a.k.a YUYV,  2Bytes / pixel, Interlaced (YUYVYUYV)
    IMAGE_GREY8 = 6,   ///< Grey image, 1 byte/pixel
    IMAGE_GREY4 = 7,   ///< Grey image, two pixels per byte
    IMAGE_YUV411_PLANAR=8, ///< 4y 1u1v planar
    IMAGE_NV12=9,      ///< planar y then interleaved uv subsampled 2x2.
    IMAGE_YUV420_PLANAR=10, ///< 4y 1u1v planar
  };


  // Parse an image format string.
  URBI_SDK_API UImageFormat parse_image_format(const std::string&);

  // Conversion to string.
  URBI_SDK_API const char* format_string(UImageFormat f);



  /*---------.
  | UImage.  |
  `---------*/

  /** Class encapsulating an image.

   This class does not handle its memory: the data field msut be
   freed manualy.  */
  class URBI_SDK_API UImage
  {
  public:
    /// Return an empty UImage.
    /// Not a constructor so that we can still put it in a union.
    static UImage make();

    /// Initialize.
    /// Not a constructor so that we can still put it in a union.
    void init();

    /// Return a legible definition of imageFormat.
    const char* format_string() const;

    /// Pointer to image data.
    unsigned char* data;
    /// Image size in byte.
    size_t size;
    /// Dimensions of the image.
    size_t width, height;

    UImageFormat imageFormat;

  private:
    friend class UBinary;
    // The UBinary headers.
    std::string headers_() const;
  };

} // end namespace urbi

#endif // ! URBI_UIMAGE_HH
