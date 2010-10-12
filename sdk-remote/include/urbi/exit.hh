/*
 * Copyright (C) 2009, 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef URBI_EXIT_HH
# define URBI_EXIT_HH

# include <exception>
# include <string>

# include <urbi/export.hh>

namespace urbi
{
  class URBI_SDK_API Exit: public std::exception
  {
  public:
    Exit(int error, const std::string& message);
    virtual ~Exit() throw ();
    virtual const char* what() const throw ();
    int error_get() const;

  private:
    int error_;
    std::string msg_;
  };
}

#endif // !URBI_EXIT_HH
