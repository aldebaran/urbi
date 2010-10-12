/*
 * Copyright (C) 2004, 2006, 2008, 2009, 2010, Gostai S.A.S.
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
  URBI_SDK_API int convertYCrCbtoYCbCr(const byte* source, size_t sourcelen,
                                       byte* dest);
  URBI_SDK_API int convertRGBtoYCrCb(const byte* source, size_t sourcelen,
                                     byte* dest);
  URBI_SDK_API int convertYCrCbtoRGB(const byte* source, size_t sourcelen,
                                     byte* dest);
  URBI_SDK_API int convertJPEGtoYCrCb(const byte* source, size_t sourcelen,
                                      byte** dest, size_t& size,
                                      size_t& w, size_t& h);
  URBI_SDK_API int convertJPEGtoRGB(const byte* source, size_t sourcelen,
                                    byte** dest, size_t& size,
                                    size_t& w, size_t& h);

  URBI_SDK_API int convertRGBtoJPEG(const byte* source,
                                    size_t w, size_t h, byte* dest,
                                    size_t& size, int quality);
  URBI_SDK_API int convertYCrCbtoJPEG(const byte* source,
                                      size_t w, size_t h, byte* dest,
                                      size_t& size, int quality);

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

  // Sound format conversion.
  URBI_SDK_API int convert(const USound& source, USound& destination);


  // Image format conversion. JPEG compression not implemented.
  URBI_SDK_API int convert(const UImage& source, UImage& destination);

} // namespace urbi
#endif
