/*
 * Copyright (C) 2009, Gostai S.A.S.
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
#include <sstream>
#include <libport/assert.hh>
#include <libport/indent.hh>

#include <ast/error.hh>
#include <ast/nary.hh>

namespace ast
{

  Error::Error()
    : errors_()
    , warnings_()
    , reported_(false)
  {}

  Error::~Error()
  {
    passert(*this, reported_ || empty());
  }

  bool
  Error::empty() const
  {
    return errors_.empty() && warnings_.empty();
  }

  bool
  Error::good() const
  {
    return errors_.empty();
  }

  void
  Error::message_(messages_type& ms, const loc& l, const std::string& msg)
  {
    reported_ = false;
    std::ostringstream o;
    o << "!!! " << l << ": " << msg;
    ms.push_back(o.str());
  }

  void
  Error::error(const loc& l, const std::string& msg)
  {
    message_(errors_, l, msg);
  }

  void
  Error::warn(const loc& l, const std::string& msg)
  {
    message_(warnings_, l, msg);
  }

  namespace
  {
    static
    std::ostream&
    operator<<(std::ostream& o,
               const std::list<std::string>& ms)
    {
      std::copy(ms.begin(), ms.end(),
                std::ostream_iterator<std::string>(o, "\n"));
      return o;
    }
  }


  std::ostream&
  Error::dump(std::ostream& o) const
  {
    reported_ = true;
    return o
      << "Errors:"
      << libport::incendl << errors_ << libport::decendl
      << "Warnings:"
      << libport::incendl << warnings_ << libport::decindent
      ;
  }

  void
  Error::process_errors(ast::Nary& target)
  {
    reported_ = true;

    foreach(const std::string& e, warnings_)
      target.message_push(e, "warning");
    warnings_.clear();

    foreach(const std::string& e, errors_)
      target.message_push(e, "error");
    errors_.clear();
  }


} // namespace ast
