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
# include <urbi/object/event.hh>
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
    class UConnection: public Subscription, public libport::InstanceTracker<UConnection>
    {
      URBI_CXX_OBJECT(UConnection, Subscription);
    public:
      UConnection();
      UConnection(libport::intrusive_ptr<UConnection> model);

      void call(const objects_type&);
      /// Source UVar
      rObject source;
      /// Target UVar
      rObject target;
    private:
      void init_();
    };
  }
}
#endif
