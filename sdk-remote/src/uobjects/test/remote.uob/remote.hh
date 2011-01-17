/*
 * Copyright (C) 2008-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

# include <urbi/uobject.hh>

using namespace urbi;

class remote : public UObject
{
public:
  remote (const std::string& s);
  int	init ();
  int foo (int x);

  UVar* val;
  UVar toto;

  UReturn newval (UVar&);
};
