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
//#define ENABLE_DEBUG_TRACES
#include "libport/compiler.hh"

#include "libport/cstdio"
#include <cmath>

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
    localFunction (device && device->equal("__Funct__")),
    selfFunction (device && device->equal("self")),
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
    localFunction (device && device->equal("__Funct__")),
    selfFunction (device && device->equal("self")),
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

  if (!fullname_ || !cached)
    buildFullname(command, connection);

  if (!fullname_)
    return 0;

  HMvariabletab::iterator i;
  if (nostruct &&
      (::urbiserver->objtab.find(getMethod()->str())
       != ::urbiserver->objtab.end()))
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
  return ((urbi::functionmap.find(fullname_->str())
	   != urbi::functionmap.end())
	  || (::urbiserver->functionbindertab.find(fullname_->str())
	      != ::urbiserver->functionbindertab.end()));
}


//! UVariableName access to method (with cache)
UString*
UVariableName::getMethod()
{
  if (method)
    return method;

  if (!fullname_)
    return 0;

  if (const char *pointPos = strstr(fullname_->str(), "."))
    method = new UString(pointPos + 1);
  else
    method = new UString("");

  return method;
}

//! UVariableName access to device (with cache)
UString*
UVariableName::getDevice()
{
  if (device)
    return device;

  if (!fullname_)
    return 0;
  if (char *pointPos = const_cast<char*>(strstr(fullname_->str(), ".")))
    {
      pointPos[0] = 0;

      device = new UString(fullname_->str());
      pointPos[0] = '.';
      return device;
    }
  else
    return fullname_;
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
  /* The behavior of snprintf is not portable when we hit the limit.
   For instance:

   #include <string>
   #include <cstdlib>
   #include <iostream>
   
   int main()
   {
     char buf[10];
     const char* lng = "012345678901234567890123456789";
     snprintf (buf, sizeof buf, "%s", lng);
     std::cout << buf << std::endl;
     return 0;
   }

   gives "0123456789$" on MingW, and "012345678" on OSX.

   So to make sure we do nothing silly (such as the $ on MingW),
   leave at least one 0 at the end.  Of course the right answer is
   to use stringstreams.  That's already done in the trunk.
   */
  const int		fullnameMaxSize = 1023;
  char			name[fullnameMaxSize + 1];
  char			indexstr[fullnameMaxSize + 1];
  name[fullnameMaxSize] = 0;
  indexstr[fullnameMaxSize] = 0;

  UValue*		e1;
  UNamedParameters*	itindex;
  HMaliastab::iterator	hmi;
  HMaliastab::iterator	past_hmi;

  if (cached)
    return fullname_;

  if (str)
  {
    e1 = str->eval(command, connection);
    cached = str->isconst;

    if (e1==0 || e1->str==0 || e1->dataType != DATA_STRING)
    {
      send_error (connection, command,
		  "dynamic variable evaluation failed");
      delete e1;
      if (fullname_)
      {
	delete fullname_;
	fullname_ = 0;
      }
      return 0;
    }

    if (strchr(e1->str->str(), '.') == 0)
    {
      nostruct = true;
      if (connection->stack.empty())
	snprintf(name, fullnameMaxSize,
		 "%s.%s", connection->connectionTag->str(),
		 e1->str->str());
      else
      {
	snprintf(name, fullnameMaxSize,
		 "%s.%s", "__Funct__",
		 e1->str->str());
	localFunction = true;
      }
    }
    else
      strncpy(name, e1->str->str(), fullnameMaxSize);

    delete e1;
    char* p = strchr (name, '.');
    ASSERT (p!=0)
    {
      p[0]=0;
      delete device;
      delete id;
      device = new UString (name);
      id = new UString (p+1);
      p[0]= '.';
    }
  };

  if (device->equal("local"))
    device->update(connection->connectionTag->str());

  cached = true;

  // index on object name
  if (index_obj != 0)
  {
    // rebuilding name based on index

    //snprintf(name, fullnameMaxSize, "%s.%s", device->str(), id->str());
    itindex = index_obj;
    std::string buildstr = device->str ();

    while (itindex)
    {
      e1 = itindex->expression->eval(command, connection);
      if (e1==0)
      {
	send_error(connection, command, "array index evaluation failed");
	delete fullname_;
	fullname_ = 0;
	return 0;
      }
      if (e1->dataType == DATA_NUM)
	snprintf(indexstr, fullnameMaxSize, "__%d", (int)e1->val);
      else if (e1->dataType == DATA_STRING)
	snprintf(indexstr, fullnameMaxSize, "__%s",
		 e1->str->str());
      else
      {
	delete e1;
	send_error(connection, command, "invalid array index type");
	delete fullname_;
	fullname_ = 0;
	return 0;
      }

      // Suppress this to make index non static by default
      // if (!itindex->expression->isconst) cached = false;

      buildstr = buildstr + indexstr;
      itindex = itindex->next;
      delete e1;
    }
    device->update (buildstr.c_str());
  }

  // index on attribute
  if (index != 0)
  {
    // rebuilding name based on index

    //snprintf(name, fullnameMaxSize, "%s.%s", device->str(), id->str());
    itindex = index;
    std::string buildstr = id->str ();

    while (itindex)
    {
      e1 = itindex->expression->eval(command, connection);
      if (e1==0)
      {
	send_error(connection, command, "array index evaluation failed");
	delete fullname_;
	fullname_ = 0;
	return 0;
      }

      if (e1->dataType == DATA_NUM)
	snprintf(indexstr, fullnameMaxSize, "__%d", (int)e1->val);
      else if (e1->dataType == DATA_STRING)
	snprintf(indexstr, fullnameMaxSize, "__%s",
		 e1->str->str());
      else
      {
	delete e1;
	send_error(connection, command, "invalid array index type");
	delete fullname_;
	fullname_ = 0;
	return 0;
      }

      // Suppress this to make index non static by default
      // if (!itindex->expression->isconst) cached = false;

      buildstr = buildstr + indexstr;
      itindex = itindex->next;
      delete e1;
    }
    id->update (buildstr.c_str());
  }

  // Local function call
  if ((localFunction || selfFunction)
      && firsttime)
  {
    firsttime = false;
    if (!connection->stack.empty())
    {
      UCallid *funid = connection->stack.front();
      if (funid)
      {
	if (selfFunction)
	  device->update(funid->self());

	if (localFunction)
	{
	  if (local_scope)
	    device->update(funid->str());
	  else
	  {
	    // does the symbol exist as a symbol local to the function call?
	    bool function_symbol = false;
	    std::string tmpstr(funid->str());
	    tmpstr = tmpstr + "." + id->str();
	    if (::urbiserver->variabletab.find(tmpstr.c_str()) !=
		::urbiserver->variabletab.end()
		|| kernel::eventSymbolDefined (tmpstr.c_str()))
	      function_symbol = true;

	    // does the symbol exist as an object symbol (direct on inherited)?
	    bool class_symbol = false;
	    bool ambiguous = true;

	    HMobjtab::iterator objit =::urbiserver->objtab.find(funid->self());
	    if (objit != ::urbiserver->objtab.end())
	    {
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
		device->update(funid->self());
	      else
		device->update(funid->str());
	    }
	    else
	    {
	      std::string tmploc(connection->connectionTag->str());
	      tmploc = tmploc + "." + id->str();
	      const char* cp = tmploc.c_str();

	      // does the symbol exist as a symbol local to the connection?
	      bool local_symbol =
		(::urbiserver->variabletab.find(cp)
		 != ::urbiserver->variabletab.end())
		|| (::urbiserver->functiontab.find(cp)
		    != ::urbiserver->functiontab.end())
		|| kernel::eventSymbolDefined (cp);

	      if (local_symbol && !function_symbol)
		device->update(connection->connectionTag->str());
	      else
		device->update(funid->str());
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
  snprintf(name, fullnameMaxSize, "%s.%s", device->str(), id->str());

  // Alias updating
  if (withalias)
  {
    if (nostruct)
    {
      // Comes from a simple IDENTIFIER
      HMaliastab::iterator getobj;
      if (char* p = strchr(name, '.'))
	getobj = ::urbiserver->objaliastab.find(p+1);
      else
	getobj = ::urbiserver->objaliastab.find(name);
      if (getobj != ::urbiserver->objaliastab.end())
      {
	UString* newobj = getobj->second;
	getobj = ::urbiserver->objaliastab.find(newobj->str());
	while (getobj != ::urbiserver->objaliastab.end())
	{
	  newobj = getobj->second;
	  getobj = ::urbiserver->objaliastab.find(newobj->str());
	}
	snprintf(name, fullnameMaxSize, "%s", newobj->str());
      }

      if (char* p = strchr(name, '.'))
	hmi = ::urbiserver->aliastab.find(p+1);
      else
	hmi = ::urbiserver->aliastab.find(name);
    }
    else
      hmi = ::urbiserver->aliastab.find(name);
    past_hmi = hmi;

    while (hmi != ::urbiserver->aliastab.end())
    {
      past_hmi = hmi;
      hmi = ::urbiserver->aliastab.find(hmi->second->str());
    }

    if (past_hmi != ::urbiserver->aliastab.end())
    {
      strncpy(name, past_hmi->second->str(), fullnameMaxSize);
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
    if (char* p = strchr(name, '.'))
    {
      if (fullname_)
	fullname_->update(p+1);
      else
	fullname_ = new UString(p+1);

      return fullname_;
    }
  }

  if (char* p = strchr(name, '.'))
  {
    p[0]=0;
    HMaliastab::iterator getobj = ::urbiserver->objaliastab.find(name);
    p[0]='.';
    if (getobj != ::urbiserver->objaliastab.end())
    {
      UString* newobj = getobj->second;
      getobj = ::urbiserver->objaliastab.find(newobj->str());
      while (getobj != ::urbiserver->objaliastab.end())
      {
	newobj = getobj->second;
	getobj = ::urbiserver->objaliastab.find(newobj->str());
      }
      UString* newmethod = new UString(p+1);
      snprintf(name, fullnameMaxSize, "%s.%s", newobj->str(),
	       newmethod->str());
      delete newmethod;
    }
  }

  if (fullname_)
    fullname_->update(name);
  else
    fullname_ = new UString(name);

  ECHO(*fullname_);
  return fullname_;
}

//! UVariableName name update for functions scope hack
void
UVariableName::nameUpdate(const char* _device, const char* _id)
{
  if (device)
    device->update(_device);
  else
    device = new UString(_device);

  if (id)
    id->update(_id);
  else
    id = new UString(_id);
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
