/*
 * Copyright (C) 2009, 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef URBI_UCONTEXT_FACTORY_HH
# define URBI_UCONTEXT_FACTORY_HH

namespace urbi
{
  namespace impl
  {
    /// The list of modules that can be bound() into a UContext.
    URBI_SDK_API
    std::vector<std::string> listModules();

    /// The plugin context. Implemented in plugin library only.
    URBI_SDK_API
    UContextImpl* getPluginContext();

    /// A new remote context. Implemented in remote library only.
    URBI_SDK_API
    UContextImpl* makeRemoteContext(const std::string& host,
                                    const std::string& port);
    /// A remote context, only creates one context per host:port pair.
    URBI_SDK_API
    UContextImpl* getRemoteContext(const std::string& host,
                                   const std::string& port);
  }
}
#endif
