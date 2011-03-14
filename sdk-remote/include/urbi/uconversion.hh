/*
 * Copyright (C) 2004, 2006, 2008, 2009, 2010, 2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file urbi/uconversion.hh
/// \brief Conversion of sounds and images.

#ifndef URBI_UCONVERSION_HH
# define URBI_UCONVERSION_HH

# include <urbi/export.hh>
# include <urbi/uvalue.hh>

namespace urbi
{
  typedef unsigned char byte;

  /// Image format conversion functions.

  /// Convert a buffer \a in, which contains an YCrCb, image to
  /// a buffer for the \a out, which will contain a YCbCr image.
  /// This function is its own reverse operation and can be used to convert
  /// YCbCr image into YCrCb.
  ///
  /// The \a in and \a out are expected to be pointers
  /// to a valid memory area of size equal to \a bufferSize.
  URBI_SDK_API int
  convertYCrCbtoYCbCr(const byte* source, size_t sourcelen,
                      byte* dest);

  /// Convert a buffer \a in containing a source image, (an RGB
  /// image), to a buffer \a out for the destinationImage, which will
  /// contain a YCbCr (YUV) image.
  ///
  /// The \a sourceImage and \a destinationImage are expected to be
  /// pointers to a valid memory area of size equal to \a bufferSize.
  URBI_SDK_API int
  convertRGBtoYCbCr(const byte* source, size_t sourcelen,
                    byte* dest);

  /// Convert a buffer \a in, which contains an YCrCb, image to a
  /// buffer for the \a out, which will contain a RGB image.
  ///
  /// The \a in and \a out are expected to be pointers
  /// to a valid memory area of size equal to \a bufferSize.
  URBI_SDK_API int
  convertYCbCrtoRGB(const byte* source, size_t sourcelen,
                    byte* dest);
  URBI_SDK_API int
  convertJPEGtoYCrCb(const byte* source, size_t sourcelen,
                     byte** dest, size_t& size,
                     size_t& w, size_t& h);
  URBI_SDK_API int
  convertJPEGtoRGB(const byte* source, size_t sourcelen,
                   byte** dest, size_t& size,
                   size_t& w, size_t& h);

  /// Convert a buffer \a source, which contains an RGB image, to a
  /// buffer for the \a dest, which will contain a JPEG image.
  ///
  /// The \a source and \a dest are expected to be pointers to a valid
  /// memory area of size equal to \a size.  The \a size argument is
  /// modified to represent the size of the JPEG data inside the \a dest
  /// buffer.
  ///
  /// Arguments \a w, \a h and \a quality are used to respectively define
  /// the width, the height and the quality of the compressed image.
  URBI_SDK_API int
  convertRGBtoJPEG(const byte* source,
                   size_t w, size_t h, byte* dest,
                   size_t& size, int quality);

  /// Convert a buffer \a source, which contains an YCrCb image, to a
  /// buffer for the \a dest, which will contain a JPEG image.
  ///
  /// The \a source and \a dest are expected to be pointers to a valid
  /// memory area of size equal to \a size.  The \a size argument is
  /// modified to represent the size of the JPEG data inside the \a dest
  /// buffer.
  ///
  /// Arguments \a w, \a h and \a quality are used to respectively define
  /// the width, the height and the quality of the compressed image.
  URBI_SDK_API int
  convertYCrCbtoJPEG(const byte* source,
                     size_t w, size_t h, byte* dest,
                     size_t& size, int quality);

  /// Convert a buffer \a in containing a source image, (an RGB image), to a
  /// buffer \a out for the destination Image, which will contain a Grey8
  /// image with the formula defined in recommendation 601. ( see
  /// http://fr.wikipedia.org/wiki/Niveau_de_gris )
  ///
  /// The \a sourceImage and \a destinationImage are expected to be pointers
  /// to a valid memory area of size equal to \a bufferSize for the input
  /// image and equal to the third for the output image.
  URBI_SDK_API int
  convertRGBtoGrey8_601(const byte* in, size_t bufferSize,
                        byte* out);


  struct wavheader
  {
    char riff[4];
    int length;
    char wave[4];
    char fmt[4];
    int lnginfo;
    short one;
    short channels;
    int freqechant;
    int bytespersec;
    short bytesperechant;
    short bitperchannel;
    char data[4];
    size_t datalength;
  };

  /// Conversion between various sound formats.
  ///
  /// Supported sound formats are RAW format and WAV format.  The \a dest
  /// sound must have its sound format defined and any other zero properties
  /// (channel, sampleSize, rate, sampleFormat) are copied from the \a
  /// source sound.
  ///
  /// The function handles memory reallocation of the destination data if
  /// the size is to small to contains the converted sound.
  URBI_SDK_API int convert(const USound& source, USound& destination);


  /// Convert the image \a src to the image \a dest.
  ///
  /// The image format of the destination has to be initialized.  If other
  /// fields are empty, they are supposed to be identical to the source
  /// image.
  ///
  /// The destination image can only have one of the following type rgb, ppm,
  /// YCbCr or jpeg.
  ///
  /// JPEG compression not implemented.
  URBI_SDK_API int convert(const UImage& source, UImage& destination);

} // namespace urbi
#endif
