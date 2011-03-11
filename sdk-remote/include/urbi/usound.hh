/*
 * Copyright (C) 2007-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file urbi/usound.hh
#ifndef URBI_USOUND_HH
# define URBI_USOUND_HH

# include <iosfwd>

# include <urbi/export.hh>

namespace urbi
{

  /*---------------.
  | USoundFormat.  |
  `---------------*/
  enum USoundFormat
  {
    SOUND_RAW = 0,
    SOUND_WAV = 1,
    SOUND_MP3 = 2,
    SOUND_OGG = 3,
    SOUND_UNKNOWN = 4,
  };

  /// Parse a sound format string.
  URBI_SDK_API USoundFormat parse_sound_format(const std::string&);

  /// Conversion to string.
  URBI_SDK_API const char* format_string(USoundFormat f);


  /*---------------------.
  | USoundSampleFormat.  |
  `---------------------*/
  enum USoundSampleFormat
  {
    SAMPLE_SIGNED = 1,
    SAMPLE_UNSIGNED = 2,
    SAMPLE_FLOAT = 3,
  };

  std::istream& operator>> (std::istream& is, USoundSampleFormat& f);


  /*---------.
  | USound.  |
  `---------*/

  /** Class encapsulating sound information.

   This class does not handle its memory: the data field must be
   freed manualy.  */
  class URBI_SDK_API USound
  {
  public:
    /// Return an empty instance.
    /// Not a constructor so that we can still put it in a union.
    static USound make();

    /// Initialize.
    /// Not a constructor so that we can still put it in a union.
    void init();

    /// Return a legible definition of imageFormat.
    const char* format_string() const;

    bool operator==(const USound &b) const;

    // For debugging.
    std::ostream& dump(std::ostream& o) const;

    /// Pointer to sound data.
    char* data;
    /// Total size in byte.
    size_t size;
    /// Number of audio channels.
    size_t channels;
    /// Rate in Hertz.
    size_t rate;
    /// Sample size in bit.
    size_t sampleSize;

    /// Format of the sound data.
    USoundFormat soundFormat;

    /// Sample format.
    USoundSampleFormat sampleFormat;

  private:
    friend class UBinary;
    // The UBinary headers.
    std::string headers_() const;
  };

  // Bounce to USound::dump.
  std::ostream& operator<< (std::ostream& o, const USound& s);


} // end namespace urbi

#endif // ! URBI_USOUND_HH
