/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file libuvalue/ubinary.cc

#include <iostream>
#include <libport/debug.hh>
#include <urbi/ubinary.hh>

GD_ADD_CATEGORY(UBinary);

namespace urbi
{

  UBinary::UBinary()
    : type(BINARY_NONE)
    , allocated_(true)
  {
    common.data = 0;
    common.size = 0;
  }

  UBinary::UBinary(const UBinary& b, bool copy)
    : type(BINARY_NONE)
    , allocated_(copy)
  {
    common.data = 0;
    if (copy)
      *this = b;
    else
    {
      // Be safe, do not try to guess which is bigger.
      image = b.image;
      sound = b.sound;
      message = b.message;
      type = b.type;
    }
  }

  UBinary::UBinary(const UImage& i, bool copy)
    : type(BINARY_IMAGE)
    , image(i)
    , allocated_(copy)
  {
    if (copy)
    {
      image.data = static_cast<unsigned char*> (malloc (image.size));
      memcpy(image.data, i.data, image.size);
    }
  }

  UBinary::UBinary(const USound& i, bool copy)
    : type(BINARY_SOUND)
    , sound(i)
    , allocated_(copy)
  {
    if (copy)
    {
      sound.data = static_cast<char*> (malloc (sound.size));
      memcpy(sound.data, i.data, sound.size);
    }
  }

  UBinary::~UBinary()
  {
    if (common.data && allocated_)
      free(common.data);
  }

  UBinary& UBinary::operator= (const UBinary& b)
  {
    if (this == &b)
      return *this;

    free(common.data);

    type = b.type;
    message = b.message;
    common.size = b.common.size;
    switch(type)
    {
      case BINARY_IMAGE:
	image = b.image;
	break;
      case BINARY_SOUND:
	sound = b.sound;
	break;
      case BINARY_NONE:
      case BINARY_UNKNOWN:
	break;
    }
    common.data = malloc(common.size);
    memcpy(common.data, b.common.data, b.common.size);
    return *this;
  }

  int
  UBinary::parse(const char* message, int pos,
		 const std::list<BinaryData>& bins,
		 std::list<BinaryData>::const_iterator& binpos)
  {
    std::istringstream is(message + pos);
    bool ok = parse(is, bins, binpos);
    // tellg() might be -1 if we encountered an error.
    int endpos = is.tellg();
    if (endpos == -1)
      endpos = strlen(message) - pos;
    return (ok ? 1:-1) * (pos + endpos);
  }

  bool
  UBinary::parse(std::istringstream& is,
		 const std::list<BinaryData>& bins,
		 std::list<BinaryData>::const_iterator& binpos)

  {
    GD_CATEGORY(UBinary);
    // LIBPORT_ECHO("Parsing: {" << is.str() << "}");
    if (binpos == bins.end())
    {
      GD_ERROR("no binary data available");
      return false;
    }

    // Validate size.
    size_t psize;
    is >> psize;
    if (is.fail())
    {
      GD_FERROR("cannot read bin size: %s (%s)", (is.str())(psize));
      return false;
    }
    if (psize != binpos->size)
    {
      GD_FERROR("bin size inconsistency: %s != %s", (psize)(binpos->size));
      return false;
    }
    common.size = psize;
    common.data = malloc(psize);
    memcpy(common.data, binpos->data, common.size);
    ++binpos;

    // Skip spaces.
    while (is.peek() == ' ')
      is.ignore();

    // Get the headers.
    std::stringbuf sb;
    is.get(sb);
    message = sb.str();

    // The contents is after the header (and the end of line:\r\n or \n).
    if (is.peek() == '\r')
      is.ignore();
    is.ignore();

    // Analyse the header to decode know UBinary types.
    // Header stream.
    std::istringstream hs(message);

    // Parse the optional type.  Don't check hs.fail, since the type
    // hs optional, in which case t remains empty.
    std::string t;
    hs >> t;
    if (t == "jpeg" || t == "YCbCr" || t == "rgb" || t.find("image_")==0)
    {
      type = BINARY_IMAGE;
      image.size = common.size;
      hs >> image.width >> image.height;
      image.imageFormat =
        t == "jpeg" ? IMAGE_JPEG
        : t == "YCbCr" ? IMAGE_YCbCr
        : t == "rgb" ? IMAGE_RGB
        : IMAGE_UNKNOWN;
    }
    else if (t == "raw" || t == "wav")
    {
      type = BINARY_SOUND;
      sound.soundFormat =
        t == "raw" ? SOUND_RAW
        : t == "wav" ? SOUND_WAV
        : SOUND_UNKNOWN;
      sound.size = common.size;
      hs >> sound.channels
         >> sound.rate
         >> sound.sampleSize >> sound.sampleFormat;
    }
    else
    {
      // GD_FWARN("unknown binary type: %s", (t));
      type = BINARY_UNKNOWN;
    }

    return true;
  }

  void UBinary::buildMessage()
  {
    message = getMessage();
  }

  std::string UBinary::getMessage() const
  {
    std::ostringstream o;
    switch (type)
    {
      case BINARY_IMAGE:
	o << image.format_string()
	  << ' ' << image.width << ' ' << image.height;
	break;

      case BINARY_SOUND:
	o << sound.format_string()
	  << ' ' << sound.channels
	  << ' ' << sound.rate
	  << ' ' << sound.sampleSize
	  << ' ' << sound.sampleFormat;
	break;

      case BINARY_UNKNOWN:
	o << message;
	break;

      case BINARY_NONE:
	break;
    }
    return o.str();
  }

  std::ostream&
  UBinary::print(std::ostream& o) const
  {
    o << "BIN "<< common.size << ' ' << getMessage() << ';';
    o.write((char*) common.data, common.size);
    return o;
  }

  std::ostream&
  operator<< (std::ostream& o, const UBinary& t)
  {
    return t.print(o);
  }

} // namespace urbi
