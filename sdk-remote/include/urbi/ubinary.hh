/*
 * Copyright (C) 2007-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file urbi/ubinary.hh
#ifndef URBI_UBINARY_HH
# define URBI_UBINARY_HH

# include <libport/cstring>
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

  // Internal use: unparsed binary data.
  class URBI_SDK_API BinaryData
  {
  public:
    BinaryData();
    BinaryData(void *d, size_t s);
    /// Reclaim data.
    void clear();
    void* data;
    size_t size;
  };

  /// List of the binaries.
  typedef std::list<BinaryData> binaries_type;


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
    UBinary(const UBinary &b, bool copy = true, bool temp = false);
    explicit UBinary(const UImage&, bool copy = true);
    explicit UBinary(const USound&, bool copy = true);

    /// Deep copy.
    UBinary& operator= (const UBinary &b);

    /// Store the result of getMessage() in member \a message.
    void buildMessage();
    /// Get header.
    std::string getMessage() const;

    /// Clear all the buffers that were allocated by the system.
    void clear();

    /// Frees binary buffer.
    ~UBinary();
    /// Return true on success.
    bool parse(std::istringstream& is,
               const binaries_type& bins,
               binaries_type::const_iterator& binpos, bool copy = true);
    int parse(const char* message, int pos,
	      const binaries_type& bins,
	      binaries_type::const_iterator& binpos, bool copy = true);

    /// Used by UValue::print for serialization.
    std::ostream& print(std::ostream& o, int kernelMajor = 2) const;

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
    /// Headers (everything after "BIN theSize" and before ';' or \n).
    std::string message;

    /// Whether the memory (common.data) is managed by this object, or
    /// by the user.
    bool allocated_;
    /** If true, content is a temporary value that can be stolen when copied.
     * Assigning from a temporary will mark the target as temporary too.
     * So the temporary_ property must be reset to false manually at
     * some point.
     */
    bool temporary_;
  };

  URBI_SDK_API
  std::ostream& operator<< (std::ostream& o, const UBinary& t);

} // end namespace urbi

# include <urbi/ubinary.hxx>

#endif // ! URBI_UBINARY_HH
