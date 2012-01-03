/*
 * Copyright (C) 2010-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef OBJECT_UCONNECTION_HH
# define OBJECT_UCONNECTION_HH

# include <urbi/object/cxx-object.hh>
# include <runner/runner.hh>

# include <urbi/uvalue.hh>

namespace urbi
{
  namespace object
  {
    /** class representing a link between two slots.
     *
     *  See uobject.u for the urbi part.
     */
    class UConnection: public CxxObject
    {
    public:
      UConnection();
      UConnection(libport::intrusive_ptr<UConnection> model);
      /// Call the connection, returns false if it must be unregistered.
      bool call(runner::Runner& r, rObject self);
      /// Source UVar name (cannot take rObject)
      std::string source;
      /// Target UVar name (cannot take rObject)
      std::string target;
      /// Do not trigger if !enabled
      bool enabled;
      /// Do not call faster than this period in seconds
      double minInterval;
      /// Time of last call
      double lastCall;
      /// Number of calls made
      long callCount;
      /// Total callback processing time
      double totalCallTime;
      /// Min and max call time
      double minCallTime, maxCallTime;
      /// Call asynchronously
      bool asynchronous;
    private:
      void doCall(runner::Runner* r, rObject self, rObject target);
      /// True if we are currently processing this connection.
      bool processing;
      void init_();
      friend class UVar;
      URBI_CXX_OBJECT(UConnection, CxxObject);
    };
  }
}
#endif
