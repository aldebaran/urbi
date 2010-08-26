/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file ast/error.cc
 ** \brief Implementation of ast::Error.
 */

#include <iostream>
#include <libport/cassert>
#include <libport/format.hh>
#include <libport/indent.hh>

#include <ast/error.hh>
#include <ast/nary.hh>

namespace ast
{

  Error::Error()
    : messages_()
    , reported_(false)
  {}

  Error::~Error()
  {
    passert(*this, reported_ || empty());
  }

  bool
  Error::empty() const
  {
    return messages_get().empty();
  }

  bool
  Error::good() const
  {
    foreach (const message_type& m, messages_get())
      if (m.error)
        return false;
    return true;
  }

  void
  Error::message_(bool error, const loc& l, const std::string& msg)
  {
    reported_ = false;
    messages_ << message_type(error, l, msg);
  }

  void
  Error::error(const loc& l, const std::string& msg)
  {
    message_(true, l, msg);
  }

  void
  Error::warn(const loc& l, const std::string& msg)
  {
    message_(false, l, msg);
  }

  const Error::messages_type&
  Error::messages_get() const
  {
    return messages_;
  }

  Error::messages_type&
  Error::messages_get()
  {
    return messages_;
  }

  namespace
  {
    static
    std::ostream&
    operator<<(std::ostream& o,
               const Error::messages_type& ms)
    {
      foreach (const Error::message_type& m, ms)
        o << (m.error ? "error: " : "warning: ")
          << m.location << ": " << m.message << std::endl;
      return o;
    }
  }


  std::ostream&
  Error::dump(std::ostream& o) const
  {
    reported_ = true;
    return o
      << "Messages:"
      << libport::incendl << messages_ << libport::decendl;
  }

  void
  Error::process_errors(ast::Nary& target)
  {
    reported_ = true;
    foreach(const message_type& e, messages_)
      target.message_push(e.location, e.message,
                          e.error ? "error" : "warning");
    messages_.clear();
  }


} // namespace ast
