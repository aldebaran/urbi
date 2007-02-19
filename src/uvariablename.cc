/*! \file uvariablename.cc
 *******************************************************************************

 File: uvariablename.cc\n
 Implementation of the UVariableName class.

 This file is part of
 %URBI Kernel, version __kernelversion__\n
 (c) Jean-Christophe Baillie, 2004-2005.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.net

 **************************************************************************** */
// #define ENABLE_DEBUG_TRACES
#include "libport/compiler.hh"

#include "libport/cstdio"
#include <cmath>
#include <sstream>

#include "libport/containers.hh"

#include "ucallid.hh"
#include "ucommand.hh"
#include "uconnection.hh"
#include "ucopy.hh"
#include "ueventhandler.hh"
#include "unamedparameters.hh"
#include "uobj.hh"
#include "urbi/uobject.hh"
#include "userver.hh"
#include "uvalue.hh"
#include "uvariable.hh"
#include "uvariablename.hh"

namespace
{
  /// Return the part after the first `.', or the whole string if there is none.
  const char*
  suffix (const char* name)
  {
    if (const char* p = strchr(name, '.'))
      return p + 1;
    else
      return name;
  }

  /// Return the part before the `.', or an empty string.
  std::string
  prefix (const char* name)
  {
    if (const char* p = strchr(name, '.'))
      return std::string(name, p - name);
    else
      return "";
  }
}



MEMORY_MANAGER_INIT(UVariableName);


// **************************************************************************
//! UVariableName constructor for variable of the type device.id[...][...]...
UVariableName::UVariableName(UString* device,
			     UString *id,
			     bool rooted,
			     UNamedParameters *index)
  : device    (device),
    id        (id),
    method    (0),
    rooted    (rooted),
    index     (index),
    index_obj (0),
    str       (0),
    isstatic  (false),
    isnormalized (false),
    deriv     (UNODERIV),
    varerror  (false),
    varin     (false),
    variable  (0),
    function  (0),
    fromGroup (false),
    firsttime (true),
    nostruct  (false),
    local_scope  (false),
    doublecolon (false),
    // Protected.
    fullname_ (0),
    localFunction (device && *device == "__Funct__"),
    selfFunction (device && *device == "self"),
    cached    (false)
{
  ADDOBJ(UVariableName);
}

//! UVariableName constructor for variable of the type device[...][...].id[...][...]...
UVariableName::UVariableName(UString* objname,
			     UNamedParameters *index_obj,
			     UString* attribute,
			     UNamedParameters *index_att)
  : device    (objname),
    id        (attribute),
    method    (0),
    rooted    (true),
    index     (index_att),
    index_obj (index_obj),
    str       (0),
    isstatic  (false),
    isnormalized (false),
    deriv     (UNODERIV),
    varerror  (false),
    varin     (false),
    variable  (0),
    function  (0),
    fromGroup (false),
    firsttime (true),
    nostruct  (false),
    local_scope  (false),
    doublecolon (false),
    fullname_ (0),
    localFunction (device && *device == "__Funct__"),
    selfFunction (device && *device == "self"),
    cached    (false)
{
  ADDOBJ(UVariableName);
}


//! UVariableName constructor for string based variables: $("...")
UVariableName::UVariableName(UExpression* str, bool rooted)
  : device    (0),
    id        (0),
    method    (0),
    rooted    (rooted),
    index     (0),
    index_obj (0),
    str       (str),
    isstatic  (false),
    isnormalized (false),
    deriv     (UNODERIV),
    varerror  (false),
    varin     (false),
    variable  (0),
    function  (0),
    firsttime (true),
    nostruct  (false),
    local_scope  (false),
    doublecolon (false),
    fullname_ (0),
    localFunction   (false),
    selfFunction    (false),
    cached    (false)
{
  ADDOBJ(UVariableName);
}

//! UVariableName destructor.
UVariableName::~UVariableName()
{
  FREEOBJ(UVariableName);
  delete device;
  delete id;
  delete method;
  delete fullname_;
  delete str;
  delete index;
}


//! UVariableName reset cache access
void
UVariableName::resetCache()
{
  cached = false;
  variable = 0;
  delete fullname_;
  fullname_ = 0;
}

UString*
UVariableName::set_fullname (const char* s)
{
  UString* res = update (fullname_, s);
#if 0
  if (device && id
      && *res != std::string (device->str()) + "." + std::string(id->str()))
    std::cerr << "Warning, \"" << *res << "\" != \""
	      << *device << "\".\"" << *id << "\"" << std::endl;
#endif
  return res;
}


//! UVariableName access to variable (with cache)
/*! If variable is not null, it means that the variable name is
 constant and that the access to the variable hash table has been done
 already. This access is then cached to limitate the number of calls to
 the hash table.
 */
UVariable*
UVariableName::getVariable (UCommand* command, UConnection* connection)
{
  if (variable)
    return variable->toDelete ? 0 : variable;

  PING();
  if (!fullname_ || !cached)
    buildFullname(command, connection);

  if (fullname_)
    ECHO(*fullname_);
  else
    ECHO("<NULL>");

  if (!fullname_)
    return 0;

  HMvariabletab::iterator i;
  if (nostruct &&
      libport::mhas(::urbiserver->objtab, getMethod()->str()))
    i = ::urbiserver->variabletab.find(getMethod()->str());
  else
    i = ::urbiserver->variabletab.find(fullname_->str());

  UVariable *tmpvar = (i != ::urbiserver->variabletab.end()) ? i->second : 0;

  if (cached)
    variable = tmpvar;

  return tmpvar;
}

//! UVariableName access to function (with cache)
/*! If function is not null, it means that the function name is
 constant and that the access to the function hash table has been done
 already. This access is then cached to limitate the number of calls to
 the hash table.
 */
UFunction*
UVariableName::getFunction(UCommand *command, UConnection *connection)
{
  if (function)
    return function;

  if (!fullname_ || !cached)
    buildFullname(command, connection);

  if (!fullname_)
    return 0;

  UFunction* f = 0;
  HMfunctiontab::iterator i = ::urbiserver->functiontab.find(fullname_->str());
  if (i != ::urbiserver->functiontab.end())
    f = i->second;

  if (cached)
    function = f;

  return f;
}

//! UVariableName test to know if there is a function with that name
bool
UVariableName::isFunction(UCommand *command, UConnection *connection)
{
  if (getFunction(command, connection))
    return true;
  if (!fullname_)
    return false;
  return (libport::mhas (*urbi::functionmap, fullname_->str())
	  || libport::mhas (::urbiserver->functionbindertab, fullname_->str()));
}


//! UVariableName access to method (with cache)
UString*
UVariableName::getMethod()
{
  if (method)
    return method;

  if (!fullname_)
    return 0;

  return method = new UString(suffix(fullname_->str()));
}

//! UVariableName access to device (with cache)
UString*
UVariableName::getDevice()
{
  if (device)
    return device;

  if (!fullname_)
    return 0;
  if (strchr(fullname_->str(), '.'))
    return device = new UString(prefix(fullname_->str()));
  else
    return fullname_;
}

bool
UVariableName::update_array_mangling (UCommand* cmd,
				      UConnection* cn,
				      UString* s, UNamedParameters* ps)
{
  // index on object name
  if (ps != 0)
  {
    // rebuilding name based on index
    std::ostringstream o;
    o << s->str ();

    for (UNamedParameters* p = ps; p; p = p->next)
    {
      UValue* e1 = p->expression->eval(cmd, cn);
      if (e1==0)
      {
	send_error(cn, cmd, "array index evaluation failed");
	delete fullname_;
	fullname_ = 0;
	return false;
      }
      if (e1->dataType == DATA_NUM)
	o << "__" << (int)e1->val;
      else if (e1->dataType == DATA_STRING)
	o << "__" << e1->str->str();
      else
      {
	delete e1;
	send_error(cn, cmd, "invalid array index type");
	delete fullname_;
	fullname_ = 0;
	return false;
      }

      // Suppress this to make index non static by default
      // if (!p->expression->isconst) cached = false;
      delete e1;
    }
    *s = o.str().c_str();
  }
  return true;
}

bool
UVariableName::build_from_str(UCommand* command, UConnection* connection)
{
  assert(str);
  UValue*e1 = str->eval(command, connection);
  cached = str->isconst;

  if (e1==0 || e1->str==0 || e1->dataType != DATA_STRING)
  {
    send_error (connection, command,
		"dynamic variable evaluation failed");
    delete e1;
    delete fullname_;
    fullname_ = 0;
    return false;
  }

  // The name is composed of two parts: PREFIX.SUFFIX.
  const char* cp = e1->str->str();
  if (strchr(cp, '.'))
  {
    update(device, prefix(cp).c_str());
    update(id, suffix(cp));
  }
  else
  {
    nostruct = true;
    if (connection->stack.empty())
      update (device, connection->connectionTag->str());
    else
    {
      update(device, "__Funct__");
      localFunction = true;
    }
    update(id, cp);
  }
  delete e1;
  return true;
}

/// Descend ::urbiserver->objaliastab looking for \a cp.
const char*
resolve_aliases(const char* cp)
{
  for (HMaliastab::iterator i = ::urbiserver->objaliastab.find(cp);
       i != ::urbiserver->objaliastab.end();
       i = ::urbiserver->objaliastab.find(cp))
    cp = i->second->str();
  return cp;
}


//! UVariableName name extraction, witch caching
/*! This method builds the name of the variable (or function) and stores it in fullname_.
 If the building blocks are static, non variable parameters (like static
 indexes in an array or constant string in a $(...)), cached is set to
 true to avoid recalculus on next call.
 */
UString*
UVariableName::buildFullname (UCommand* command,
			      UConnection* connection,
			      bool withalias)
{
  if (cached)
    return fullname_;

  if (str && !build_from_str (command, connection))
      return 0;

  if (*device == "local")
    *device = connection->connectionTag->str();

  cached = true;

  // index on object name
  if (!update_array_mangling (command, connection, device, index_obj))
    return 0;
  // index on attribute
  if (!update_array_mangling (command, connection, id, index))
    return 0;

  // Local function call
  if ((localFunction || selfFunction)
      && firsttime)
  {
    firsttime = false;
    if (!connection->stack.empty())
    {
      if (UCallid *funid = connection->stack.front())
      {
	if (selfFunction)
	  *device = funid->self();

	if (localFunction)
	{
	  if (local_scope)
	    *device = funid->str();
	  else
	  {
	    // does the symbol exist as a symbol local to the function call?
	    bool function_symbol = false;
	    std::string tmpstr(funid->str());
	    tmpstr = tmpstr + "." + id->str();
	    const char* cp = tmpstr.c_str();
	    if (libport::mhas(::urbiserver->variabletab, cp)
		|| kernel::eventSymbolDefined (cp))
	      function_symbol = true;

	    // does the symbol exist as an object symbol (direct on inherited)?
	    bool class_symbol = false;

	    HMobjtab::iterator objit =::urbiserver->objtab.find(funid->self());
	    if (objit != ::urbiserver->objtab.end())
	    {
	      bool ambiguous = true;
	      class_symbol =
		objit->second->searchVariable(id->str(), ambiguous)
		|| objit->second->searchFunction(id->str(), ambiguous)
		|| objit->second->searchEvent(id->str(), ambiguous);
	      class_symbol = class_symbol && !ambiguous;
	    }

	    // name resolution
	    if (class_symbol)
	    {
	      if (!function_symbol)
		*device = funid->self();
	      else
		*device = funid->str();
	    }
	    else
	    {
	      std::string tmploc(connection->connectionTag->str());
	      tmploc = tmploc + "." + id->str();
	      const char* cp = tmploc.c_str();

	      // does the symbol exist as a symbol local to the connection?
	      bool local_symbol =
		libport::mhas(::urbiserver->variabletab, cp)
		|| libport::mhas(::urbiserver->functiontab, cp)
		|| kernel::eventSymbolDefined (cp);

	      if (local_symbol && !function_symbol)
		*device = connection->connectionTag->str();
	      else
		*device = funid->str();
	    }
	  }
	}
      }
    }
    else
    {
      send_error(connection, command, "invalid prefix resolution");
      delete fullname_;
      fullname_ = 0;
      return 0;
    }
  }

  // Create the concatened variable name
  std::string name = std::string(device->str()) + "." + id->str();
  ECHO(name);

  // Alias updating
  if (withalias)
  {
    HMaliastab::iterator hmi;
    if (nostruct)
    {
      // Comes from a simple IDENTIFIER.
      const char* cp = resolve_aliases(suffix(name.c_str()));
      hmi = ::urbiserver->aliastab.find(suffix(cp));
    }
    else
      hmi = ::urbiserver->aliastab.find(name.c_str());

    HMaliastab::iterator past_hmi = hmi;
    while (hmi != ::urbiserver->aliastab.end())
    {
      past_hmi = hmi;
      hmi = ::urbiserver->aliastab.find(hmi->second->str());
    }

    if (past_hmi != ::urbiserver->aliastab.end())
    {
      name = past_hmi->second->str();
      nostruct = false;
      delete device;
      device = 0;
      delete method;
      method = 0; // forces recalc of device.method
      variable = 0;
    }
  }
  else if (nostruct)
  {
    if (name.find('.') != std::string::npos)
      return set_fullname (suffix(name.c_str()));
  }

  if (name.find('.') != std::string::npos)
  {
    const char* cp = resolve_aliases(prefix(name.c_str()).c_str());
    name = std::string(cp) + "." + suffix(name.c_str());
  }

  return set_fullname (name.c_str());
}

//! UVariableName name update for functions scope hack
void
UVariableName::nameUpdate(const char* _device, const char* _id)
{
  update(device, _device);
  update(id, _id);
}

//! UNamedParameters hard copy function
UVariableName*
UVariableName::copy() const
{
  UVariableName *ret;

  if (str)
    ret = new UVariableName(str->copy(), rooted);
  else if (index_obj)
    ret = new UVariableName (ucopy (device),
			     ucopy (index_obj),
			     ucopy (id),
			     ucopy (index));
  else
    ret = new UVariableName(ucopy (device),
			    ucopy (id),
			    rooted,
			    ucopy (index));

  ret->isstatic     = isstatic;
  ret->isnormalized = isnormalized;
  ret->deriv        = deriv;
  ret->varerror     = varerror;
  ret->fromGroup    = fromGroup;
  ret->nostruct     = nostruct;
  ret->local_scope  = local_scope;
  ret->doublecolon  = doublecolon;

  return ret;
}

//! Print the variable
/*! This function is for debugging purpose only.
 It is not safe, efficient or crash proof. A better version will come later.
 */
void
UVariableName::print() const
{
  ::urbiserver->debug("(VAR root=%d ", rooted);
  if (device)
    ::urbiserver->debug("device='%s' ", device->str());
  if (id)
    ::urbiserver->debug("id='%s' ", id->str());
  ::urbiserver->debug(") ");
}
