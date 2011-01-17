/*
 * Copyright (C) 2008-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef UCHANGE_HH
# define UCHANGE_HH

# include <urbi/uobject.hh>

class uchange : public urbi::UObject
{
public:
  uchange (const std::string& s);
  int init ();

  urbi::UVar* val;

  urbi::UReturn newval (urbi::UVar&);
};

#endif
