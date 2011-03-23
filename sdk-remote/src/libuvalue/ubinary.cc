/*
 * Copyright (C) 2009-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file libuvalue/ubinary.cc

#include <iostream>
#include <sstream>
#include <libport/debug.hh>
#include <libport/escape.hh>
#include <libport/foreach.hh>
#include <libport/format.hh>
#include <boost/algorithm/string/trim.hpp>
#include <urbi/ubinary.hh>
#include <urbi/uvalue.hh> // kernelMajor.

GD_CATEGORY(Urbi.UValue);

namespace urbi
{

  UBinary::UBinary()
    : type(BINARY_NONE)
    , allocated_(true)
    , temporary_(false)
  {
    common.data = 0;
    common.size = 0;
  }

  UBinary::UBinary(const UBinary& b, bool copy, bool temp)
    : type(BINARY_NONE)
    , allocated_(copy)
    , temporary_(temp)
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
    , temporary_(false)
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
    , temporary_(false)
  {
    if (copy)
    {
      sound.data = static_cast<char*> (malloc (sound.size));
      memcpy(sound.data, i.data, sound.size);
    }
  }

  void
  UBinary::clear()
  {
    if (allocated_)
    {
      free(common.data);
      common.data = 0;
      common.size = 0;
    }
  }

  UBinary::~UBinary()
  {
    clear();
  }

  UBinary& UBinary::operator= (const UBinary& b)
  {
    if (this == &b)
      return *this;

    clear();
    if (b.temporary_)
    {
      // Be safe, do not try to guess which is bigger.
      image = b.image;
      sound = b.sound;
      message = b.message;
      type = b.type;
      UBinary& bb = const_cast<UBinary&>(b);
      bb.common.data = 0;
      bb.type = BINARY_NONE;
      temporary_ = true;
      return *this;
    }
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
		 const binaries_type& bins,
		 binaries_type::const_iterator& binpos, bool copy)
  {
    std::istringstream is(message + pos);
    bool ok = parse(is, bins, binpos, copy);
    // tellg() might be -1 if we encountered an error.
    int endpos = is.tellg();
    if (endpos == -1)
      endpos = strlen(message) - pos;
    return (ok ? 1:-1) * (pos + endpos);
  }

  namespace
  {
    /// Return everything up to the next "\n" or "\n\r" or ";", not included.
    /// Leave \a i after that delimiter.
    /// Return empty string on errors.
    static
    std::string
    headers_get(std::istringstream& i)
    {
      std::string res;
      int c = 0;
      while (!i.eof()
             && (c = i.get()) && c != '\n' && c != ';')
        res.append(1, c);
      if (i.eof())
        GD_ERROR("unexpected end of file while parsing UBinary headers");
      else
      {
        // Skip the delimiter.
        if (c == '\n')
        {
          if (i.peek() == '\r')
            i.ignore();
        }
      }
      // Remove leading/trailing spaces.
      boost::algorithm::trim(res);
      return res;
    }
  }


  bool
  UBinary::parse(std::istringstream& is,
		 const binaries_type& bins,
		 binaries_type::const_iterator& binpos, bool copy)

  {
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
      GD_FERROR("cannot read bin size: %s (%s)", is.str(), psize);
      return false;
    }
    if (psize != binpos->size)
    {
      GD_FERROR("bin size inconsistency: %s != %s", psize, binpos->size);
      return false;
    }
    common.size = psize;
    if (copy)
    {
      common.data = malloc(common.size);
      memcpy(common.data, binpos->data, common.size);
    }
    else
    {
      common.data = binpos->data;
      this->allocated_ = false;
    }
    ++binpos;

    // Skip spaces.
    while (is.peek() == ' ')
      is.ignore();

    // Get the headers.
    message = headers_get(is);

    // Analyse the header to decode know UBinary types.
    // Header stream.
    std::istringstream hs(message);

    // Parse the optional type.  Don't check hs.fail, since the type
    // is optional, in which case t remains empty.
    std::string t;
    hs >> t;
    UImageFormat image_format = parse_image_format(t);
    if (image_format != IMAGE_UNKNOWN || t.find("image_")==0)
    {
      type = BINARY_IMAGE;
      image.size = common.size;
      // In some cases (jpeg source), image size is not present in headers.
      image.width = image.height = 0;
      hs >> image.width >> image.height;
      image.imageFormat = image_format;
    }
    else if (t == "raw" || t == "wav")
    {
      type = BINARY_SOUND;
      sound.soundFormat = parse_sound_format(t);
      sound.size = common.size;
      hs >> sound.channels
         >> sound.rate
         >> sound.sampleSize >> sound.sampleFormat;
    }
    else
    {
      // GD_FWARN("unknown binary type: %s", t);
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
    switch (type)
    {
    case BINARY_IMAGE:
      return image.headers_();
    case BINARY_SOUND:
      return sound.headers_();
    case BINARY_UNKNOWN:
      {
        bool warned = false;
        std::string res = message;
        foreach (char& c, res)
          if (c == '\0' || c == '\n' || c == ';')
          {
            if (!warned)
            {
              GD_FERROR("invalid UBinary header: "
                        "prohibited `\\n', `\\0' and `;' will be "
                        "smashed to space: %s",
                        libport::escape(message));
              warned = true;
            }
            c = ' ';
          }
        // Remove leading/trailing spaces.
        boost::algorithm::trim(res);
        return res;
      }
    case BINARY_NONE:
      return "";
    }
    unreachable();
  }

  std::ostream&
  UBinary::print(std::ostream& o, int kernelMajor) const
  {
    if (2 <= kernelMajor)
    {
      o << libport::format("Global.Binary.new(\"%s\", \"\\B(%s)(",
                           getMessage(), common.size);
      o.write((char*) common.data, common.size);
      o << ")\")";
    }
    else
    {
      // Format for the Kernel, which wants ';' as header terminator.
      o << "BIN " << common.size;
      const std::string h = getMessage();
      if (!h.empty())
        o << ' ' << h;
      o << ';';
      o.write((char*) common.data, common.size);
    }
    return o;
  }

  std::ostream&
  operator<< (std::ostream& o, const UBinary& t)
  {
    return t.print(o, ::urbi::kernelMajor(o));
  }

} // namespace urbi
