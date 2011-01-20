/*
 * Copyright (C) 2009, 2010, 2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file urbi/umessage.hh

#ifndef URBI_UMESSAGE_HH
# define URBI_UMESSAGE_HH

# include <iosfwd>
# include <list>
# include <string>

# include <urbi/export.hh>
# include <urbi/fwd.hh>
# include <urbi/ubinary.hh>

namespace urbi
{

  enum UMessageType
  {
    MESSAGE_SYSTEM, ///< Messages prefixed by ***.
    MESSAGE_ERROR,  ///< Messages prefixed by !!!.
    MESSAGE_DATA    ///< All other messages.
  };

  /// Class containing all informations related to an URBI message.
  class URBI_SDK_API UMessage
  {
  public:
    UMessage(UAbstractClient& client);

    /// Parser constructor.
    /// Keeps a copy of tag and message.
    UMessage(UAbstractClient& client, int timestamp,
	     const char* tag, const char* message,
	     const binaries_type& bins = binaries_type());
    /// Likewise.
    UMessage(UAbstractClient& client, int timestamp,
	     const std::string& tag, const std::string& message,
	     const binaries_type& bins = binaries_type());

    UMessage(const UMessage& source);

    /// Free everything if data was copied, doesn't free anything otherwise
    ~UMessage();

    /// Return the message.
    operator urbi::UValue&();

    /// Format as "[TIMESTAMP:TAG] (!!!|***)? CONTENTS".
    std::ostream& print(std::ostream& o) const;

    /// Connection from which originated the message.
    UAbstractClient& client;
    /// Server-side timestamp.
    int timestamp;
    /// Associated tag.
    std::string	tag;

    /// The type of this message.
    UMessageType type;

    /// Set only if the message type is MESSAGE_DATA.
    urbi::UValue* value;
    /// Set only if the message type is MESSAGE_SYSTEM or MESSAGE_ERROR.
    std::string	message;
    /// Raw message without the binary data.
    std::string	rawMessage;

  private:
    /// Factor common code between ctors.
    /// Works on rawMessage.
    void init_(const binaries_type& bins);
  };

  /// For debugging purpose.
  std::ostream& operator<<(std::ostream& s, const UMessage& m);

  /// Set \a val to the value of \a m.
  /// \return 1 on success, 0 on failure.
  /// An error message is printed on error.
  template <typename T>
  int getValue(UMessage* m, T& val);

} // namespace urbi



# include <urbi/umessage.hxx>

#endif // URBI_UMESSAGE_HH
