/*
 * Copyright (C) 2010, 2011, Gostai S.A.S.
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
# include <runner/job.hh>

# include <urbi/uvalue.hh>

namespace urbi
{
  namespace object
  {
    /** A link between two slots.
     *
     *  See uobject.u for the urbi part.
     */
    class UConnection: public CxxObject
    {
    public:
      typedef UConnection self_type;

      UConnection();
      UConnection(libport::intrusive_ptr<UConnection> model);

      /// Call the connection, returns false if it must be unregistered.
      bool call(runner::Job& r, rObject self);
      /// Source UVar name (cannot take rObject).
      std::string source;
      /// Target UVar name (cannot take rObject).
      std::string target;
      /// Trigger only if enabled.
      bool enabled;
      /// Do not call faster than this period in seconds.
      double minInterval;
      /// Time of last call.
      double lastCall;
      /// Number of calls made.
      long callCount;
      /// Total callback processing time.
      double totalCallTime;
      /// Min and max call time.
      double minCallTime, maxCallTime;
      /// Call asynchronously.
      bool asynchronous;
    private:
      rObject doCall(runner::Job& r, rObject self, rObject target);
      /// True if we are currently processing this connection.
      bool processing;
      void init_();
      friend class UVar;
      URBI_CXX_OBJECT(UConnection, CxxObject);
    };
  }
}
#endif
