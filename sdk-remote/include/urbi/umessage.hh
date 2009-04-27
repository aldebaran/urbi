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
    /// List of the binaries.
    typedef std::list<urbi::BinaryData> binaries_type;

    UMessage(UAbstractClient& client);

    /// Parser constructor.
    UMessage(UAbstractClient& client, int timestamp,
	     const char* tag, const char* message,
	     const binaries_type& bins = binaries_type());

    UMessage(const UMessage& source);

    /// Free everything if data was copied, doesn't free anything otherwise
    ~UMessage();

    /// Return the message.
    operator urbi::UValue&();

    /// Format as "[TIMESTAMP:TAG] CONTENTS".
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
  };

  std::ostream& operator<<(std::ostream& s, const UMessage& m);

} // namespace urbi

# include <urbi/umessage.hxx>

#endif // URBI_UMESSAGE_HH
