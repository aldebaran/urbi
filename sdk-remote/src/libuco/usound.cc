/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */
#include <sstream>
#include <libport/cstring>

#include <urbi/usound.hh>

namespace urbi
{
  USound
  USound::make()
  {
    USound res;
    res.data = 0;
    res.size = res.sampleSize = res.channels = res.rate = 0;
    res.soundFormat = SOUND_UNKNOWN;
    res.sampleFormat = SAMPLE_UNSIGNED;
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
    switch (soundFormat)
    {
      case SOUND_RAW:
	return "raw";
      case SOUND_WAV:
	return "wav";
      case SOUND_MP3:
	return "mp3";
      case SOUND_OGG:
	return "ogg";
      case SOUND_UNKNOWN:
	return "unknown format";
    }
    // To pacify "warning: control reaches end of non-void function".
    // FIXME: This should not abort. UImage should be initialized with IMAGE_UKNOWN.
    //        This is not done because data is stored in an union in UBinary and
    //        union members cannot have constructors.
    return "unknown format";
  }

  USound::operator std::string() const
  {
    std::ostringstream o;
    o << "sound(format: " << format_string() << ", "
      << "size: " << size << ", "
      << "channels: " << channels << ", "
      << "rate: " << rate << ", "
      << "sample size: " << sampleSize << ", "
      << "sample format: ";
    switch (sampleFormat)
    {
    case SAMPLE_SIGNED:   o << "signed";  break;
    case SAMPLE_UNSIGNED: o << "unsigned"; break;
    default:              o << "unknown[" << (int)sampleFormat << "]";
    }
    o << ")";
    return o.str();
  }

  std::istream&
  operator>> (std::istream& is, USoundSampleFormat& f)
  {
    int v = 0;
    is >> v;
    f = USoundSampleFormat(v);
    return is;
  }

}
