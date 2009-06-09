/// \file urbi/ubinary.hh

// This file is part of UObject Component Architecture
// Copyright (c) 2007-2009 Gostai S.A.S.
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
# include <iosfwd>
# include <list>
# include <string>

# include <urbi/export.hh>
# include <urbi/uimage.hh>
# include <urbi/usound.hh>

namespace urbi
{

  /*--------------.
  | UBinaryData.  |
  `--------------*/

  //internal use: unparsed binary data
  class URBI_SDK_API BinaryData
  {
  public:
    BinaryData()
      : data(0), size(0)
    {}
    BinaryData(void *d, size_t s)
      : data(d), size(s)
    {}
    void* data;
    size_t size;
  };



  /*----------.
  | UBinary.  |
  `----------*/

  enum UBinaryType
  {
    BINARY_NONE,
    BINARY_UNKNOWN,
    BINARY_IMAGE,
    BINARY_SOUND,
  };

  /// Binary data of known or unknown type.
  ///
  /// Handles its memory: the data field will be freed when the
  /// destructor is called.
  class URBI_SDK_API UBinary
  {
  public:
    UBinary();
    /// Deep copy constructor.
    UBinary(const UBinary &b, bool copy = true);
    explicit UBinary(const UImage&, bool copy = true);
    explicit UBinary(const USound&, bool copy = true);
    /// Deep copy.
    UBinary & operator = (const UBinary &b);
    /// Build message from structures.
    void buildMessage();
    /// Get message extracted from structures.
    std::string getMessage() const;
    /// Frees binary buffer.
    ~UBinary();
    /// Return true on success.
    bool parse(std::istringstream& is,
               const std::list<BinaryData>& bins,
               std::list<BinaryData>::const_iterator& binpos);
    int parse(const char* message, int pos,
	      const std::list<BinaryData>& bins,
	      std::list<BinaryData>::const_iterator& binpos);

    /// Used by UValue::print for serialization.
    std::ostream& print(std::ostream& o) const;

    UBinaryType type;
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
    /// Extra bin headers (everything after BIN <size> and before ';').
    std::string message;

  private:
    bool allocated_;
  };

  URBI_SDK_API
  std::ostream& operator<< (std::ostream& o, const UBinary& t);

} // end namespace urbi

#endif // ! URBI_UBINARY_HH
