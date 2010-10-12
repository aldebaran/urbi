
/*! \file urbi/usyncclient.hh
****************************************************************************
*
* Definition of the URBI interface class
*
* Copyright (C) 2004, 2006, 2007 Jean-Christophe Baillie.  All rights reserved.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
**********************************************************************/

#ifndef URBI_UMAIN_HH
# define URBI_UMAIN_HH

# include <urbi/usyncclient.hh>
# include <urbi/uobject.hh>
# include <libport/windows.hh>
# ifndef WIN32
#  include <libport/unistd.h>
# endif

# ifdef URBI_ENV_REMOTE

#  include <QApplication>
#  include <QObject>

  /**
   *
   * This class creates a Qt timer which each 3 ms calls
   * the procedure timerEvent
   *
   */

#  define UMAIN()				\
  namespace urbi				\
  {						\
						\
  class ProcessUrbiEvent:			\
    public ::QObject				\
  {						\
  public:					\
						\
    ProcessUrbiEvent ( QObject* parent = 0 )	\
      : QObject (parent)			\
    {						\
      startTimer(3);				\
						\
      cl = dynamic_cast<urbi::USyncClient*>	\
	(urbi::getDefaultClient ());		\
      cl->stopCallbackThread ();		\
    }						\
						\
  protected:					\
						\
    void					\
      timerEvent ( QTimerEvent* )		\
    {						\
      if (cl)					\
	cl->processEvents(3 * 1000);		\
    }						\
						\
  protected:					\
    urbi::USyncClient* cl;			\
						\
  };						\
}						\
						\
 int main(int argc, char *argv[])		\
 {						\
   urbi::main(argc, (const char**)argv, false);	\
						\
   QApplication app (argc, argv) ;		\
   urbi::ProcessUrbiEvent obj;			\
						\
   return app.exec();				\
 }
# else
#  define UMAIN()
# endif /* !URBI_ENV_REMOTE */

#endif /* !URBI_UMAIN_HH */
