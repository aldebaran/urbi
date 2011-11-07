/*
 * Copyright (C) 2009-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file libuvalue/usound.cc

#include <sstream>
#include <libport/cassert>
#include <libport/cstring>
#include <libport/debug.hh>
#include <libport/format.hh>

#include <urbi/usound.hh>

#define cardinality_of(Array) (sizeof (Array) / sizeof (*(Array)))

GD_CATEGORY(Urbi.UValue);

namespace urbi
{

  /*---------------.
  | USoundFormat.  |
  `---------------*/

  static const char* formats[] =
  {
    "raw",
    "wav",
    "mp3",
    "ogg",
    "unknown format",
  };

  const char*
  format_string(USoundFormat f)
  {
    if (f < 0 || int(cardinality_of(formats)) <= f)
    {
      GD_FERROR("invalid USoundFormat value: %d", f);
      f = SOUND_UNKNOWN;
    }
    return formats[f];
  }

  USoundFormat
  parse_sound_format(const std::string& s)
  {
    for (unsigned i = 0; i < cardinality_of(formats); ++i)
      if (s == formats[i])
        return static_cast<USoundFormat>(i);
    GD_FINFO_TRACE("unknown sound format: %s", s);
    return SOUND_UNKNOWN;
  }


  /*---------------------.
  | USoundSampleFormat.  |
  `---------------------*/

  std::istream&
  operator>> (std::istream& is, USoundSampleFormat& f)
  {
    int v = 0;
    is >> v;
    f = USoundSampleFormat(v);
    return is;
  }

  static
  std::ostream&
  operator<<(std::ostream& o, USoundSampleFormat f)
  {
    switch (f)
    {
    case SAMPLE_SIGNED:
      return o << "signed";
    case SAMPLE_UNSIGNED:
      return o << "unsigned";
    default:
      return o << "unknown[" << (int)f << "]";
    }
    unreachable();
  }


  /*---------.
  | USound.  |
  `---------*/

  void
  USound::init()
  {
    data = 0;
    size = sampleSize = channels = rate = 0;
    soundFormat = SOUND_UNKNOWN;
    sampleFormat = SAMPLE_UNSIGNED;
  }

  USound
  USound::make()
  {
    USound res;
    res.init();
    return res;
  }

  bool
  USound::operator==(const USound &b) const
  {
    return !memcmp(this, &b, sizeof(USound));
  }

  const char*
  USound::format_string() const
  {
    return ::urbi::format_string(soundFormat);
  }

  std::string
  USound::headers_() const
  {
    return libport::format("%s %s %s %s %d",
                           format_string(),
                           channels, rate,
                           sampleSize, int(sampleFormat));
  }

  std::ostream&
  USound::dump(std::ostream& o) const
  {
    return o <<  "sound(format: " << format_string() << ", "
             <<          "size: " << size << ", "
             <<      "channels: " << channels << ", "
             <<          "rate: " << rate << ", "
             <<   "sample size: " << sampleSize << ", "
             << "sample format: " << sampleFormat
             << ")";
  }

  std::ostream&
  operator<< (std::ostream& o, const USound& s)
  {
    return s.dump(o);
  }

}
