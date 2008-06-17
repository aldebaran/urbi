/// \file urbi/ubinary.hh

// This file is part of UObject Component Architecture
// Copyright (c) 2007, 2008 Gostai S.A.S.
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//
// For more information, comments, bug reports: http://www.urbiforge.com

#ifndef URBI_UBINARY_HH
# define URBI_UBINARY_HH

# include <cstring>
# include <list>
# include <string>

namespace urbi
{

  /** Backward compatibility **/
  enum USoundFormat
  {
    SOUND_RAW,
    SOUND_WAV,
    SOUND_MP3,
    SOUND_OGG,
    SOUND_UNKNOWN
  };

  enum USoundSampleFormat
  {
    SAMPLE_SIGNED=1,
    SAMPLE_UNSIGNED=2
  };

  enum UImageFormat
  {
    IMAGE_RGB=1,     ///< RGB 24 bit/pixel
    IMAGE_YCbCr=2,   ///< YCbCr 24 bit/pixel
    IMAGE_JPEG=3,    ///< JPEG
    IMAGE_PPM=4,     ///< RGB with a PPM header
    IMAGE_UNKNOWN
  };

  enum UBinaryType
  {
    BINARY_NONE,
    BINARY_UNKNOWN,
    BINARY_IMAGE,
    BINARY_SOUND
  };


  /*---------.
  | USound.  |
  `---------*/

  /** Class encapsulating sound information.

   This class does not handle its memory: the data field msut be
   freed manualy.  */
  class USound
  {
  public:
    char                  *data;            ///< pointer to sound data
    size_t size;             ///< total size in byte
    int                   channels;         ///< number of audio channels
    int                   rate;             ///< rate in Hertz
    int                   sampleSize;       ///< sample size in bit

    USoundFormat          soundFormat;      ///< format of the sound data
    /// Return a legible definition of imageFormat.
    const char* format_string () const;

    enum SampleFormat
    {
      SAMPLE_SIGNED=1,
      SAMPLE_UNSIGNED=2
    };
    USoundSampleFormat    sampleFormat;     ///< sample format

    bool operator ==(const USound &b) const
    {
      return !memcmp(this, &b, sizeof(USound));
    }
  };


  /*---------.
  | UImage.  |
  `---------*/

  /** Class encapsulating an image.

   This class does not handle its memory: the data field msut be
   freed manualy.  */
  class UImage
  {
  public:
    unsigned char         *data;            ///< pointer to image data
    size_t size;             ///< image size in byte
    size_t width, height;    ///< size of the image

    UImageFormat          imageFormat;

    /// Return a legible definition of imageFormat.
    const char* format_string () const;
  };


  /*--------------.
  | UBinaryData.  |
  `--------------*/

  //internal use: unparsed binary data
  class BinaryData
  {
  public:
    BinaryData()
      : data (0), size (0)
    {}
    BinaryData(void *d, int s)
      : data(d), size(s)
    {}
    void* data;
    size_t size;
  };



  /*----------.
  | UBinary.  |
  `----------*/

  /** Class containing binary data of known or unknown type.
   Handles its memory: the data field will be freed when the destructor is called.
   */
  class UBinary
  {
  public:

    UBinaryType             type;
    union
    {
      struct
      {
	void* data;             ///< binary data
	size_t size;
      } common;
      UImage image;
      USound sound;
    };
    /// Extra bin headers(everything after BIN <size> and before ';'.
    std::string message;

    UBinary();
    /// Deep copy constructor.
    UBinary(const UBinary &b);
    explicit UBinary(const UImage &);
    explicit UBinary(const USound &);
    /// Deep copy.
    UBinary & operator = (const UBinary &b);
    /// Build message from structures.
    void buildMessage();
    /// Get message extracted from structures.
    std::string getMessage() const;
    /// Frees binary buffer.
    ~UBinary();
    int parse(const char* message, int pos,
	      std::list<BinaryData> &bins,
	      std::list<BinaryData>::iterator &binpos);
  };

} // end namespace urbi

#endif // ! URBI_UBINARY_HH
