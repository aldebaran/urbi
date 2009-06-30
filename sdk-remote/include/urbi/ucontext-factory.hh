#ifndef URBI_UCONTEXT_FACTORY_HH
# define URBI_UCONTEXT_FACTORY_HH

namespace urbi
{
  namespace impl
  {
    /// Return the list of modules that can be bound() into a UContext.
    URBI_SDK_API std::vector<std::string> listModules();
    /// Return the plugin context. Implemented in plugin library only.
    URBI_SDK_API UContextImpl* getPluginContext();
    /// Return a new remote context. Implemented in remote library only.
    URBI_SDK_API UContextImpl* makeRemoteContext(const std::string& host,
                                                 const std::string& port);
    /// Return a remote context, only creates one context per host:port pair.
    URBI_SDK_API UContextImpl* getRemoteContext(const std::string& host,
                                                const std::string& port);
  }
}
#endif
