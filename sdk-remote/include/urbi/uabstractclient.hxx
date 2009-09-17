/*! \file urbi/uabstractclient.hxx
 ****************************************************************************
 *
 * Definition of the URBI interface class
 *
 * Copyright (C) 2004, 2006, 2007, 2008, 2009 Jean-Christophe Baillie.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 **********************************************************************/

#ifndef URBI_UABSTRACTCLIENT_HXX
# define URBI_UABSTRACTCLIENT_HXX

namespace urbi
{

  /*----------------.
  | UCallbackInfo.  |
  `----------------*/

  inline
  UCallbackInfo::UCallbackInfo(UCallbackWrapper &w)
    : callback(w)
  {}

  inline
  bool
  UCallbackInfo::operator==(UCallbackID id) const
  {
    return this->id == id;
  }

  /*------------------.
  | UAbstractClient.  |
  `------------------*/

  // This constant is declared as an inlined function instead of a
  // simple value because of Windows.  Indeed, libuco uses the
  // default_host value at various places, but it does not include
  // UAbstractClient.o.  So in the end, our libraries depend on values
  // that will be provided by those who link against these libraries.
  // This is not portable.
  //
  // So use an inline function.  This probably means there are several
  // instances of this value, one per library.  But who would compare
  // the pointers instead of the values?
  inline
  const char*
  UAbstractClient::default_host()
  {
    // When using Boost.Asio, "localhost" on OS X is IPv6, and nothing
    // works as expected.  Make sure we run in IPv4.
    // FIXME: Find out why we run in IPv6 by default.
    return "127.0.0.1";
  }

  inline
  bool
  UAbstractClient::init() const
  {
    return init_;
  }

  inline
  UAbstractClient::error_type
  UAbstractClient::error() const
  {
    return rc;
  }

  inline
  const std::string&
  UAbstractClient::getServerName() const
  {
    return host_;
  }

  /// Return the server port.
  inline
  unsigned
  UAbstractClient::getServerPort() const
  {
    return port_;
  }

  inline
  std::ostream&
  UAbstractClient::stream_get()
  {
    return *stream_;
  }

  inline
  int
  UAbstractClient::kernelMajor() const
  {
    waitForKernelVersion();
    return kernelMajor_;
  }

  inline
  int
  UAbstractClient::kernelMinor() const
  {
    waitForKernelVersion();
    return kernelMinor_;
  }

  inline
  const std::string&
  UAbstractClient::kernelVersion() const
  {
    waitForKernelVersion();
    return kernelVersion_;
  }

  inline
  UAbstractClient::error_type
  UAbstractClient::onClose()
  {
    closed_ = true;
    return 0;
  };

} // namespace urbi

#endif // URBI_UABSTRACTCLIENT_HXX
