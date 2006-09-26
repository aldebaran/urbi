/*! \file uconversion.h
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

#ifndef UCONVERSION_H
# define UCONVERSION_H
# include "uobject.h"

namespace urbi
{
  typedef unsigned char byte;

  /// Image format conversion functions.
  int convertRGBtoYCrCb  (const byte* source, int sourcelen, byte* dest);
  int convertYCrCbtoRGB  (const byte* source, int sourcelen, byte* dest);
  int convertJPEGtoYCrCb (const byte* source, int sourcelen, byte* dest,
			  int &size);
  int convertJPEGtoRGB   (const byte* source, int sourcelen, byte* dest,
			  int &size);

  int convertRGBtoJPEG(const byte* source, int w, int h, byte* dest,
		       int &size, int quality);
  int convertYCrCbtoJPEG(const byte* source, int w, int h, byte* dest,
			 int &size, int quality);


  //sound format conversion functions.
  int convert(const USound &source, USound &destination);


  //image format conversion. JPEG compression not impletmented.
  int convert(const UImage &source, UImage & destination);

} // namespace urbi
#endif
