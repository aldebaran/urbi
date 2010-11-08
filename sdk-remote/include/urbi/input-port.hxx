/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */


#ifndef URBI_INPUT_PORT_HXX
# define URBI_INPUT_PORT_HXX

namespace urbi
{
  inline InputPort::InputPort()
  {
  }

  inline InputPort::InputPort(const std::string& objname,
                              const std::string& name,
                              impl::UContextImpl* ctx)
  :UVar(objname, name, ctx)
  {
    impl_->setInputPort(true);
  }

  inline InputPort::InputPort(UObject* owner, const std::string& name,
                              impl::UContextImpl* ctx)
  :UVar(*owner, name, ctx)
  {
    impl_->setInputPort(true);
  }

  inline InputPort::InputPort(const InputPort& b)
  :UVar(b.get_name())
  {
    impl_->setInputPort(true);
  }

  inline InputPort& InputPort::operator=(const InputPort& b)
  {
    UVar::init(b.get_name(), b.ctx_);
    impl_->setInputPort(true);
    return *this;
  }

  inline void InputPort::unnotify()
  {
    UVar::unnotify();
  }
}

#endif
