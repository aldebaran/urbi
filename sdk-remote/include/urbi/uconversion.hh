/*! \file uconversion.hh
****************************************************************************
 * $Id: $
 *
 * Conversion of sounds and images.
 *
 * Copyright (C) 2004, 2006 Jean-Christophe Baillie.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
**********************************************************************/

#ifndef URBI_UCONVERSION_HH
# define URBI_UCONVERSION_HH
# include <urbi/uobject.hh>

namespace urbi
{
  typedef unsigned char byte;

  /// Image format conversion functions.
  USDK_API int convertYCrCbtoYCbCr(const byte* source, int sourcelen,
                                   byte* dest);
  USDK_API int convertRGBtoYCrCb  (const byte* source, int sourcelen,
                                   byte* dest);
  USDK_API int convertYCrCbtoRGB  (const byte* source, int sourcelen,
                                   byte* dest);
  USDK_API int convertJPEGtoYCrCb (const byte* source, int sourcelen,
                                   byte* dest, int &size);
  USDK_API int convertJPEGtoRGB   (const byte* source, int sourcelen,
                                   byte* dest, int &size);

  USDK_API int convertRGBtoJPEG(const byte* source, int w, int h, byte* dest,
		       int &size, int quality);
  USDK_API int convertYCrCbtoJPEG(const byte* source, int w, int h, byte* dest,
			 int &size, int quality);

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
    int datalength;
  };

  //sound format conversion functions.
  USDK_API int convert(const USound &source, USound &destination);


  //image format conversion. JPEG compression not impletmented.
  USDK_API int convert(const UImage &source, UImage & destination);

} // namespace urbi
#endif
