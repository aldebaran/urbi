/*
 * Copyright (C) 2004-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef URBI_QT_UMAIN_HH
# define URBI_QT_UMAIN_HH

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

#endif /* !URBI_QT_UMAIN_HH */
