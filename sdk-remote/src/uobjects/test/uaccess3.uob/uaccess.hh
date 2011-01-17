/*
 * Copyright (C) 2008-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef UACCESS_HH
# define UACCESS_HH

# include <urbi/uobject.hh>

class uaccess : public urbi::UObject
{
public:
  uaccess (const std::string& s);
  int init ();

  urbi::UVar val;

  urbi::UReturn newval (urbi::UVar&);
  urbi::UReturn changed (urbi::UVar&);
};

#endif
