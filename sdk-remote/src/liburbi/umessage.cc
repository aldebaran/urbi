/*
 * Copyright (C) 2008-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file liburbi/umessage.cc

#include <libport/cstdlib>
#include <iomanip>
#include <iostream>

#include <libport/debug.hh>
#include <libport/escape.hh>

#include <urbi/umessage.hh>
#include <urbi/uvalue.hh>

GD_CATEGORY(Urbi.UMessage);

namespace urbi
{

  UMessage::UMessage(UAbstractClient& client)
    : client(client)
    , value(0)
  {
  }

  void
  UMessage::init_(const binaries_type& bins)
  {
    const char* msg = rawMessage.c_str();
    GD_FINFO_DUMP("new: \"%s\", client: %p",
                  libport::escape(msg), &client);
    while (msg[0] == ' ')
      ++msg;

    // System and error messages.
    if (msg[0] == '*' || msg[0] == '!')
    {
      type = msg[0] == '*' ? MESSAGE_SYSTEM : MESSAGE_ERROR;
      if (4 <= strlen(msg))
	message = msg + 4;
      return;
    }

    // value.
    type = MESSAGE_DATA;
    value = new UValue();
    binaries_type::const_iterator iter = bins.begin();
    int p = value->parse(msg, 0, bins, iter);
    if (0 <= p)
      while (msg[p] == ' ')
	++p;
    /* no assertion can be made on message[p] because there is no terminator
     * for binaries */
    if (p < 0 || /*message[p] ||*/ iter != bins.end())
      GD_FERROR("parse error in `%s' at %s", msg, abs(p));
  }

  UMessage::UMessage(UAbstractClient& client, int timestamp,
		     const char* tag, const char* msg,
		     const binaries_type& bins)
    : client(client)
    , timestamp(timestamp)
    , tag(tag)
    , value(0)
    , rawMessage(msg)
  {
    init_(bins);
  }

  UMessage::UMessage(UAbstractClient& client, int timestamp,
		     const std::string& tag, const std::string& msg,
		     const binaries_type& bins)
    : client(client)
    , timestamp(timestamp)
    , tag(tag)
    , value(0)
    , rawMessage(msg)
  {
    init_(bins);
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
        o << "*** " << message;
        break;
      case MESSAGE_ERROR:
	o << "!!! " << message;
	break;
    }
    return o;
  }


} // namespace urbi
