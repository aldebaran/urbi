/// \file libuobject/uobject.cc

#include <iostream>
#include <sstream>
#include <list>
#include <libport/unistd.h>

#include <urbi/uobject.hh>
#include <urbi/usyncclient.hh>

//! Global definition of the starterlist
namespace urbi
{
  UObject* dummyUObject;

  std::ostream& unarmorAndSend(const char* a);

  void uobject_unarmorAndSend(const char* a)
  {
    unarmorAndSend(a);
  }

  void send(const char* a)
  {
    default_stream() << a;
  }

  void send(void* buf, int size)
  {
    default_stream().rdbuf()->sputn(static_cast<char*> (buf), size);
  }

  UObjectMode getRunningMode()
  {
    return MODE_REMOTE;
  }

  // **************************************************************************
  //  Monitoring functions

  //! Generic UVar monitoring without callback
  void
  UObject::USync(UVar&)
  {
    //UNotifyChange(v, &UObject::voidfun);
  }

  // **************************************************************************
  //! UObject constructor.
  UObject::UObject(const std::string& s)
    : __name(s)
    , classname(s)
    , derived(false)
    , gc(0)
    , remote(true)
  {
    objecthub = 0;
    autogroup = false;
    URBI_SEND_PIPED_COMMAND("class " << __name << "{}");
    URBI_SEND_PIPED_COMMAND("external object " << __name);

    // Do not replace this call to init by a `, load(s, "load")' as
    // both result in "var <__name>.load = 1", which is not valid
    // until the two above lines actually create <__name>.
    load.init(__name, "load");

    period = -1;
    // default
    load = 1;
  }

  //! Dummy UObject constructor.
  UObject::UObject(int index)
    : derived(false)
    , gc(0)
    , remote(true)
  {
    std::stringstream ss;
    ss << "dummy" << index;
    __name = ss.str();
    classname = __name;
    objecthub = 0;
    autogroup = false;
    period = -1;
  }


  //! UObject cleaner
  void
  UObject::clean()
  {
    cleanTable(monitormap(), __name);
    cleanTable(accessmap(), __name);
    cleanTable(functionmap(), __name);
    cleanTable(eventmap(), __name);
    cleanTable(eventendmap(), __name);

    if (objecthub)
      objecthub->members.remove(this);
  }

  //! UObject destructor.
  UObject::~UObject()
  {
    clean();
  }

  void
  UObject::UJoinGroup(const std::string& gpname)
  {
    std::string groupregister = "addgroup " + gpname +" { "+__name+"};";
    uobject_unarmorAndSend(groupregister.c_str());
  }

  void
  UObject::USetUpdate(ufloat t)
  {
    std::ostringstream os;
    // Forge names for callback and tag
    std::string tagName = "maintimer_" + __name;
    std::string cbName = __name + ".maintimer";
    std::string cbFullName = cbName + "__0";

    // Stop any previous update
    URBI_SEND_COMMAND("stop " << tagName);

    // Find previous update timer on this object
    std::list<UGenericCallback*>& cblist = eventmap()[cbFullName];
    std::list<UGenericCallback*>::iterator it = cblist.begin ();
    for (; it != cblist.end () && (*it)->getName () != cbFullName ; ++it)
      ;

    // Delete if found
    if (it != cblist.end ())
    {
      cblist.erase (it);
      delete *it;
    }

    // Set period value
    period = t;
    // Do nothing more if negative value given
    if (period < 0)
      return;

    // FIXME: setting update at 0 put the kernel in infinite loop
    //        and memory usage goes up to 100%
    if (period == 0)
      period = 1;

    // Create callback
    createUCallback(__name, "event",
		    this, &UObject::update, cbName, eventmap(), false);

    // Set update at given period
    URBI_SEND_COMMAND(tagName << ": every(" << period << ")"
		      "              { emit " << cbName << ";},");

    return;
  }

  int
  UObject::send (const std::string& s)
  {
    URBI(()) << s;
    return 0;
  }

  UObjectHub::~UObjectHub()
  {
  }

  void
  UObjectHub::USetUpdate(ufloat t)
  {
    period = t;
    // nothing happend in remote mode...
  }

  //! echo method
  void
  echo(const char* format, ...)
  {
    va_list arg;
    va_start(arg, format);
    vfprintf(stderr, format, arg);
    va_end(arg);
  }

  void yield()
  {
    yield_until(libport::utime());
  }
  void yield_until(libport::utime_t deadline)
  {
    // Ensure processEvents is called at least once.
    while (true)
    {
      bool processed = dynamic_cast<USyncClient*>(getDefaultClient())
        ->processEvents(0);
      if (deadline < libport::utime())
        return;
      if (!processed)
        usleep(0);
    }
  }
  void yield_until_things_changed()
  {
    while (true)
    {
      if (dynamic_cast<USyncClient*>(getDefaultClient())->processEvents(0))
        return;
      usleep(0);
    }
  }
  void side_effect_free_set(bool) {}
  bool side_effect_free_get() {return false;}
} // namespace urbi
