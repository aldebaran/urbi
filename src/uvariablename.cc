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

#include "urbi/uobject.hh"

#include "kernel/userver.hh"
#include "kernel/uconnection.hh"
#include "kernel/uvalue.hh"
#include "kernel/uvariable.hh"
#include "ucallid.hh"
#include "ucommand.hh"
#include "ucopy.hh"
#include "ueventhandler.hh"
#include "unamedparameters.hh"
#include "uobj.hh"
#include "uvariablename.hh"


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
  return update (fullname_, s);
#if 0
  if (device && id
      && *res != device->str().str() + "." + id->str())
    std::cerr << "Warning, \"" << *res << "\" != \""
	      << *device << "\".\"" << *id << "\"" << std::endl;
#endif
}


//! UVariableName access to variable (with cache)
/*! If variable is not null, it means that the variable name is
 constant and that the access to the variable hash table has been done
 already. This access is then cached to limitate the number of calls to
 the hash table.
 */
UVariable*
UVariableName::getVariable (UCommand* command, UConnection* connection,
			    bool force)
{
  if (variable && !force)
    return variable->toDelete ? 0 : variable;

  if (!fullname_ || !cached)
    buildFullname(command, connection);

  if (!fullname_)
    return 0;

  UVariable *res;
  if (nostruct &&
      libport::mhas(::urbiserver->objtab, getMethod()->c_str()))
    res = libport::find0(::urbiserver->variabletab, getMethod()->c_str());
  else
    res = libport::find0(::urbiserver->variabletab, fullname_->c_str());
  if (cached)
    variable = res;

  return res;
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

  UFunction* f = libport::find0(::urbiserver->functiontab, fullname_->c_str());

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
  return (libport::mhas (*urbi::functionmap, fullname_->c_str())
	  || libport::mhas (::urbiserver->functionbindertab, fullname_->c_str()));
}


//! UVariableName access to method (with cache)
UString*
UVariableName::getMethod()
{
  if (method)
    return method;

  if (!fullname_)
    return 0;

  return method = new UString(suffix(*fullname_));
}

//! UVariableName access to device (with cache)
UString*
UVariableName::getDevice()
{
  if (device)
    return device;

  if (!fullname_)
    return 0;
  if (strchr(fullname_->c_str(), '.'))
    return device = new UString(prefix(*fullname_));
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
    o << s->c_str();

    for (UNamedParameters* p = ps; p; p = p->next)
    {
      UValue* e1 = p->expression->eval(cmd, cn);
      if (e1 == 0)
      {
	send_error(cn, cmd, "array index evaluation failed");
	delete fullname_;
	fullname_ = 0;
	return false;
      }
      if (e1->dataType == DATA_NUM)
	o << "__" << (int)e1->val;
      else if (e1->dataType == DATA_STRING)
	o << "__" << e1->str->c_str();
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
  const std::string& n = e1->str->str();
  if (n.find('.') != std::string::npos)
  {
    update(device, prefix(n).c_str());
    update(id, suffix(n));
  }
  else
  {
    nostruct = true;
    if (connection->stack.empty())
      update (device, connection->connectionTag->c_str());
    else
    {
      update(device, "__Funct__");
      localFunction = true;
    }
    update(id, n);
  }
  delete e1;
  return true;
}

namespace
{
  /// Descend \a tab looking for \a cp.
  std::string
  resolve_aliases(const HMaliastab& tab, const std::string& s)
  {
    const std::string* res = &s;
    for (HMaliastab::const_iterator i = tab.find(res->c_str()); i != tab.end();
	 i = tab.find(res->c_str()))
      res = &i->second->str();
    return *res;
  }

  /// Descend ::urbiserver->objaliastab looking for \a cp.
  std::string
  resolve_aliases(const std::string& s)
  {
    return resolve_aliases (::urbiserver->objaliastab, s);
  }

}

//! UVariableName name extraction, with caching
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
    *device = connection->connectionTag->c_str();

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
	    std::string n = funid->str().str() + "." + id->str();
	    const char* cp = n.c_str();
	    if (libport::mhas(::urbiserver->variabletab, cp)
		|| kernel::eventSymbolDefined (cp))
	      function_symbol = true;

	    // does the symbol exist as an object symbol (direct on inherited)?
	    bool class_symbol = false;

	    if (const UObj* u = libport::find0(::urbiserver->objtab,
					       funid->self().c_str()))
	    {
	      bool ambiguous = true;
	      class_symbol =
		u->searchVariable(id->c_str(), ambiguous)
		|| u->searchFunction(id->c_str(), ambiguous)
		|| u->searchEvent(id->c_str(), ambiguous);
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
	      std::string tmploc(connection->connectionTag->c_str());
	      tmploc = tmploc + "." + id->str();
	      const char* cp = tmploc.c_str();

	      // does the symbol exist as a symbol local to the connection?
	      bool local_symbol =
		libport::mhas(::urbiserver->variabletab, cp)
		|| libport::mhas(::urbiserver->functiontab, cp)
		|| kernel::eventSymbolDefined (cp);

	      if (local_symbol && !function_symbol)
		*device = connection->connectionTag->c_str();
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
  std::string name = device->str() + "." + id->str();
  ECHO(name);

  // Alias updating
  if (withalias)
  {
    std::string n = name;
    if (nostruct)
      // Comes from a simple IDENTIFIER.
      n = suffix(resolve_aliases(suffix(n)));

    std::string resolved = resolve_aliases (::urbiserver->aliastab, n);
    if (resolved != n)
    {
      send_error(connection, command,
		 ("'" + n + "' is an alias for '" + resolved + "'. "
		   "Using aliases is deprecated.").c_str());
      name = resolved;
      if (name.find('.') == std::string::npos)
	name = device->str () + "." + suffix(name);
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
      return set_fullname(suffix(name).c_str());
  }

  if (name.find('.') != std::string::npos)
    name = resolve_aliases(prefix(name)) + "." + suffix(name);

  return set_fullname (name.c_str());
}

//! UVariableName name update for functions scope hack
void
UVariableName::nameUpdate(const char* _device, const char* _id)
{
  update(device, _device);
  update(id, _id);
}

//! UVariableName name update for functions scope hack
void
UVariableName::nameUpdate(const std::string& _device, const std::string& _id)
{
  nameUpdate(_device.c_str(), _id.c_str());
}

//! UVariableName name update for functions scope hack
void
UVariableName::nameUpdate(const UString& _device, const UString& _id)
{
  nameUpdate(_device.str(), _id.str());
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
    ::urbiserver->debug("device='%s' ", device->c_str());
  if (id)
    ::urbiserver->debug("id='%s' ", id->c_str());
  ::urbiserver->debug(") ");
}
