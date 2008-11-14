/// \file urbi/umessage.hh

#ifndef URBI_UMESSAGE_HH
# define URBI_UMESSAGE_HH

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
  class USDK_API UMessage
  {
  public:
    /// Connection from which originated the message.
    UAbstractClient& client;
    /// Server-side timestamp.
    int timestamp;
    /// Associated tag.
    std::string	tag;

    UMessageType type;

    /// Set only if the message type is MESSAGE_DATA.
    urbi::UValue* value;
    /// Set only if the message type is MESSAGE_SYSTEM or MESSAGE_ERROR.
    std::string	message;
    /// Raw message without the binary data.
    std::string	rawMessage;

    /// Default ctor
    UMessage(UAbstractClient& client);

    /// Parser constructor
    UMessage(UAbstractClient& client, int timestamp,
	     const char* tag, const char* message,
	     std::list<urbi::BinaryData> bins);
    /// If alocate is true, everything is copied, eles pointers are stolen
    UMessage(const UMessage& source);

    /// Free everything if data was copied, doesn't free anything otherwise
    ~UMessage();

    operator urbi::UValue& () {return *value;}

  };

  USDK_API std::ostream& operator<<(std::ostream& s, const UMessage& m);

} // namespace urbi

#endif // URBI_UMESSAGE_HH
