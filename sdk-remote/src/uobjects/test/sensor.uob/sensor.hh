/*
 * Copyright (C) 2008-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef SENSOR_HH
# define SENSOR_HH

# include <urbi/uobject.hh>

using namespace urbi;

class sensor : public UObject
{
  public:
    sensor (std::string s);
    int	init ();

    UVar val;

    UReturn newval (UVar&);
    UReturn getval (UVar&);
    void setVal (int);
};

#endif
