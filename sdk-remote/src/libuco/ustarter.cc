/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/foreach.hh>
#include <urbi/ustarter.hh>

namespace urbi
{

  /*------------------.
  | baseURBIStarter.  |
  `------------------*/


  baseURBIStarter::list_type&
  baseURBIStarter::list()
  {
    static list_type list;
    return list;
  }

  /*---------------------.
  | baseURBIStarterHub.  |
  `---------------------*/

  baseURBIStarterHub::list_type&
  baseURBIStarterHub::list()
  {
    static list_type list;
    return list;
  }

}
