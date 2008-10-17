/// \file libuobject/ucallbacks.cc

#include <iostream>
#include <sstream>
#include <list>

#include <libport/program-name.hh>

#include <urbi/ucallbacks.hh>
#include <urbi/usyncclient.hh>
#include <urbi/uexternal.hh>
#include <urbi/uobject.hh>

namespace urbi
{

  //! UGenericCallback constructor.
  UGenericCallback::UGenericCallback(const std::string& objname,
				     const std::string& type,
				     const std::string& name,
				     int size, UTable &, bool)
    : objname (objname), name (name)
  {
    nbparam = size;

    if (type == "function" || type== "event" || type == "eventend")
    {
      std::ostringstream oss;
      oss << size;
      this->name += "__" + oss.str();
    }

    std::cerr << libport::program_name
	      << ": Registering " << type << " " << name << " " << size
	      << " into " << this->name
	      << " from " << objname
	      << std::endl;

    if (type == "var")
      URBI_SEND_COMMAND("external " << type << " "
			<< name << " from " << objname);

    if (type == "event" || type == "function")
      URBI_SEND_COMMAND("external " << type << "(" << size << ") "
			<< name << " from " << objname);

    if (type == "varaccess")
      echo("Warning: NotifyAccess facility is not available for modules in "
	   "remote mode.\n");
  }

  //! UGenericCallback constructor.
  UGenericCallback::UGenericCallback(const std::string& objname,
				     const std::string& type,
				     const std::string& name, UTable&)
    : objname(objname), name(name)
  {
    URBI_SEND_COMMAND("external " << type << " " << name);
  }

  UGenericCallback::~UGenericCallback()
  {
  }

  void
  UGenericCallback::registerCallback(UTable& t)
  {
     t[this->name].push_back(this);
  }


} // namespace urbi
