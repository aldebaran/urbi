/// \file liburbi/umessage.cc

#include <cstdlib>
#include <iomanip>
#include <iostream>

#include <urbi/umessage.hh>
#include <urbi/uvalue.hh>

namespace urbi
{

  UMessage::UMessage(UAbstractClient& client)
    : client(client)
    , value(0)
  {
  }

  UMessage::UMessage(UAbstractClient& client, int timestamp,
		     const char* tag, const char* message,
		     const binaries_type& bins)
    : client(client)
    , timestamp(timestamp)
    , tag(tag)
    , value(0)
  {
    rawMessage = std::string(message);
    while (message[0] == ' ')
      ++message;

    // System and error messages.
    if (message[0] == '*' || message[0] == '!')
    {
      type = message[0] == '*' ? MESSAGE_SYSTEM : MESSAGE_ERROR;
      if (4 <= strlen(message))
	this->message = message + 4;
      return;
    }

    // value.
    type = MESSAGE_DATA;
    value = new UValue();
    binaries_type::const_iterator iter = bins.begin();
    int p = value->parse(message, 0, bins, iter);
    if (0 <= p)
      while (message[p] == ' ')
	++p;
    /* no assertion can be made on message[p] because there is no terminator
     * for binaries */
    if (p < 0 || /*message[p] ||*/ iter != bins.end())
      std::cerr << "PARSE ERROR in " << message << "at " << abs(p) << std::endl;
  }

  UMessage::UMessage(const UMessage& b)
    : client(b.client)
    , timestamp(b.timestamp)
    , tag(b.tag)
    , type(b.type)
    , value(0)
    , rawMessage(b.rawMessage)
  {
    switch (type)
    {
      case MESSAGE_SYSTEM:
      case MESSAGE_ERROR:
	message = b.message;
	break;
      default:
	value = new UValue(*b.value);
	break;
    }
  }


  UMessage::~UMessage()
  {
    if (type != MESSAGE_SYSTEM && type != MESSAGE_ERROR && value)
      delete value;
  }

  std::ostream&
  UMessage::print(std::ostream &o) const
  {
    char fill = o.fill('0');
    o << '[' << std::setw(8) << timestamp;
    o.fill(fill);
    if (!tag.empty())
      o << ":" << tag;
    o << "] ";
    switch (type)
    {
      case MESSAGE_DATA:
	o << *value;
	break;
      case MESSAGE_SYSTEM:
      case MESSAGE_ERROR:
	o << message;
	break;
    }
    return o;
  }


} // namespace urbi
