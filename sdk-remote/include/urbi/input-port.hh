/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef URBI_INPUT_PORT_HH
# define URBI_INPUT_PORT_HH

# include <urbi/uvar.hh>

namespace urbi
{
  /** An InputPort is a slot that can be connected to other UVars from
   * urbiscript.
   *
   */
  class URBI_SDK_API InputPort: private UVar
  {
  public:
    InputPort();
    InputPort(const std::string& objname, const std::string& name,
              impl::UContextImpl* = 0);
    InputPort(UObject* owner, const std::string& name, impl::UContextImpl* = 0);
    InputPort(const InputPort& b);
    InputPort& operator=(const InputPort& b);
    void init(UObject* owner, const std::string& name, impl::UContextImpl* = 0);
    void init(const std::string& obj, const std::string& name,
              impl::UContextImpl* = 0);
    /// Remove all notifies setup on this port.
    void unnotify();
  private:
    friend class UObject;
  };
}
#include <urbi/input-port.hxx>
#endif
