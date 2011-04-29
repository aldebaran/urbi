/*
 * Copyright (C) 2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file urbi/details.hh
/// \brief Implementation details, do not rely on this file.

#ifndef URBI_DETAILS_HH
# define URBI_DETAILS_HH

# include <string>

namespace urbi
{
  namespace uobjects
  {
    // Auxiliary routines to extract components of an UObject/UVar
    // name.
    typedef std::pair<std::string, std::string> StringPair;

    /// Split an UVar full name into its two components.
    /// Issue a simple warning if there is no ".".
    StringPair
    uname_split(const std::string& name);

    /// Split an UVar full name into its two components.
    /// Throw if there is no ".".
    /// \param name  what's to split.
    /// \param ctx   some optional name about where the error occurred.
    StringPair
    uname_xsplit(const std::string& name, const std::string& ctx = "");
  }
}

#endif // !URBI_DETAILS_HH
