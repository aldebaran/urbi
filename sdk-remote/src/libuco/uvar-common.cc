/// \file libuco/uvar-common.cc

#include <urbi/uobject.hh>
#include <urbi/uvalue.hh>
#include <urbi/uvar.hh>

namespace urbi
{
  // **************************************************************************
  //! UVar constructor: implicit object ref (using 'lastUOjbect') + varname
  UVar::UVar(const std::string& varname)
    : VAR_PROP_INIT
  {
    name = varname;
    __init();
  }

  //! UVar constructor: object reference + var name
  UVar::UVar(UObject& obj, const std::string& varname)
    : VAR_PROP_INIT
  {
    name = obj.__name + '.' + varname;
    __init();
  }

  //! UVar constructor: object name + var name
  UVar::UVar(const std::string& objname, const std::string& varname)
    : VAR_PROP_INIT
  {
    name = objname + '.' + varname;
    __init();
  }


  //! UVar initialization
  void
  UVar::init(const std::string& objname, const std::string& varname)
  {
    name = objname + '.' + varname;
    __init();
  }

  bool
  UVar::invariant() const
  {
    if (!vardata)
    {
      // FIXME: this special case should be put in doc.
      echo ("Unable to locate variable %s in hashtable. "
	    "UVars being UObject attributes must be binded to be used "
	    "in UObject C++ code. Use UBindVar. "
	    "In any other case, this can be a memory problem. "
	    "Please report bug.\n",
	     name.c_str());
      return false;
    }
    else
      return true;
  }

  /*-------.
  | UVar.  |
  `-------*/

  void
  UVar::operator= (const UValue& v)
  {
    switch (v.type)
    {
      case DATA_STRING:
	*this = *v.stringValue;
	break;
      case DATA_BINARY:
	*this = *v.binary;
	break;
      case DATA_LIST:
	*this = *v.list;
	break;
      case DATA_DOUBLE:
	*this = v.val;
	break;
      case DATA_VOID:
	//TODO: do something!
	pabort (v);
	break;
      case DATA_OBJECT:
	// Not valid currently.
	pabort (v);
	break;
    }
  }

} // namespace urbi

std::ostream&
operator<< (std::ostream& o, const urbi::UVar& u)
{
  return o << "UVar (\"" << u.get_name() << "\" = " << u.get_value() << ')';
}
