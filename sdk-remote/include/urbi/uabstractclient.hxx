/*
 * Copyright (C) 2004-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef URBI_UABSTRACTCLIENT_HXX
# define URBI_UABSTRACTCLIENT_HXX

# include <libport/debug.hh>
# include <libport/escape.hh>

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
  UAbstractClient::effective_send(const void* buffer, size_t size)
  {
    GD_CATEGORY(Urbi.Client.Abstract);
    GD_FINFO_DUMP("Sending: \"%s\"",
                  libport::escape(std::string(static_cast<const char*>(buffer),
                                              size)));
    return effectiveSend(buffer, size);
  }

  inline
  UAbstractClient::error_type
  UAbstractClient::effective_send(const std::string& s)
  {
    return effective_send(s.c_str(), s.size());
  }

  inline
  UAbstractClient::error_type
  UAbstractClient::effective_send(const char* cp)
  {
    return effective_send(cp, strlen(cp));
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
  int
  UAbstractClient::kernelMajor() const
  {
    waitForKernelVersion();
    libport::BlockLock bl(sendBufferLock);
    return kernelMajor_;
  }

  inline
  int
  UAbstractClient::kernelMinor() const
  {
    waitForKernelVersion();
    libport::BlockLock bl(sendBufferLock);
    return kernelMinor_;
  }

  inline
  const std::string&
  UAbstractClient::kernelVersion() const
  {
    waitForKernelVersion();
    libport::BlockLock bl(sendBufferLock);
    return kernelVersion_;
  }

  inline
  std::ostream&
  UAbstractClient::stream_get()
  {
    return *stream_;
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
