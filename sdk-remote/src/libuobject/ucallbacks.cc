/// \file libuobject/ucallbacks.cc

#include <iostream>
#include <sstream>
#include <list>

#include <libport/program-name.hh>
#include <libport/lexical-cast.hh>

#include <urbi/ucallbacks.hh>
#include <urbi/usyncclient.hh>
#include <urbi/uexternal.hh>
#include <urbi/uobject.hh>

namespace urbi
{

  namespace
  {
    static
    std::string
    callback_name(const std::string& name, const std::string& type,
                  int size)
    {
      std::string res = name;
      if (type == "function" || type == "event" || type == "eventend")
        res += "__" + string_cast(size);
      return res;
    }
  }

  //! UGenericCallback constructor.
  UGenericCallback::UGenericCallback(const std::string& objname,
				     const std::string& type,
				     const std::string& name,
				     int size, UTable &, bool)
    : nbparam(size)
    , objname(objname)
    , name(callback_name(name, type, size))
  {
    std::cerr << libport::program_name()
	      << ": Registering " << type << " " << name << " " << size
	      << " into " << this->name
	      << " from " << objname
	      << std::endl;

    if (type == "var")
      URBI_SEND_PIPED_COMMAND("external " << type << " "
                              << name << " from " << objname);
    else if (type == "event" || type == "function")
      URBI_SEND_PIPED_COMMAND("external " << type << "(" << size << ") "
                              << name << " from " << objname);
    else if (type == "varaccess")
      echo("Warning: NotifyAccess facility is not available for modules in "
	   "remote mode.\n");
  }

  //! UGenericCallback constructor.
  UGenericCallback::UGenericCallback(const std::string& objname,
				     const std::string& type,
				     const std::string& name, UTable&)
    : objname(objname), name(name)
  {
    URBI_SEND_PIPED_COMMAND("external " << type << " " << name);
  }

  UGenericCallback::~UGenericCallback()
  {
  }

  void
  UGenericCallback::registerCallback(UTable& t)
  {
    std::cerr << "Pushing " << name << "in " << &t << std::endl;
    t[name].push_back(this);
  }


} // namespace urbi
