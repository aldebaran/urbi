/*
 * Copyright (C) 2011-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file ast/loc.cc
 ** \brief Implementation of ast::loc.
 */

#include <boost/algorithm/string/erase.hpp>
#include <ast/loc.hh>
#include <object/system.hh>

namespace ast
{
  ::libport::Symbol
  declare_location_file(const std::string& srcdir, std::string file)
  {
    boost::algorithm::erase_first(file, srcdir);
    boost::algorithm::erase_first(file, "/src/");
    libport::Symbol res(file);
    ::urbi::object::system_files_get().insert(res);
    return res;
  }
}
