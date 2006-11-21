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

#include <cstdio>
#include <cmath>

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

#include "uvariablename.hh"
#include "ucommand.hh"
#include "uconnection.hh"
#include "userver.hh"
#include "ucallid.hh"
#include "urbi/uobject.hh"


MEMORY_MANAGER_INIT(UVariableName);

namespace
{
  // FIXME: Should take a const arg, but does not work currently.
  template <typename T>
    inline
    T*
    ucopy (T* t)
    {
      return t ? t->copy () : 0;
    }
}



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
    variable  (0),
    function  (0),
    fromGroup (false),
    firsttime (true),
    nostruct  (false),
    id_type   (UDEF_VAR),
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
    fullname_ (0),
    str       (0),
    isstatic  (false),
    isnormalized (false),
    deriv     (UNODERIV),
    varerror  (false),
    cached    (false),
    fromGroup (false),
    variable  (0),
    function  (0),
    firsttime (true),
    nostruct  (false),
    id_type   (UDEF_VAR),
    local_scope  (false),
    localFunction (device && device->equal("__Funct__")),
    selfFunction (device && device->equal("self")),
    doublecolon (false)
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
    fullname_ (0),
    str       (str),
    isstatic  (false),
    isnormalized (false),
    deriv     (UNODERIV),
    varerror  (false),
    cached    (false),
    localFunction   (false),
    selfFunction    (false),
    variable  (0),
    function  (0),
    firsttime (true),
    nostruct  (false),
    id_type   (UDEF_VAR),
    local_scope  (false),
  doublecolon (false)
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
UVariableName::getVariable(UCommand *command, UConnection *connection)
{
  UVariable *tmpvar;

  if (variable)
    if (variable->toDelete)
      return 0;
    else
      return variable;

  if (!fullname_ || !cached)
    buildFullname(command, connection);

  if (!fullname_)
    return 0;

  if (nostruct &&
      (::urbiserver->objtab.find(getMethod()->str())
       != ::urbiserver->objtab.end()))
  {
    if ((hmi2 = ::urbiserver->variabletab.find(getMethod()->str())) !=
	::urbiserver->variabletab.end())
      tmpvar = hmi2->second;
    else
      tmpvar = 0;
  }
  else
  {
    if ((hmi2 = ::urbiserver->variabletab.find(fullname_->str())) !=
	::urbiserver->variabletab.end())
      tmpvar = hmi2->second;
    else
      tmpvar = 0;
  }


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
  UFunction *tmpfun;

  if (function) return function;

  if (!fullname_ || !cached)
    buildFullname(command, connection);

  if (!fullname_) return 0;

  if ((hmf = ::urbiserver->functiontab.find(fullname_->str())) !=
      ::urbiserver->functiontab.end())
    tmpfun = hmf->second;
  else
    tmpfun = 0;

  if (cached) function = tmpfun;

  return tmpfun;
}

//! UVariableName test to know if there is a function with that name
  bool
UVariableName::isFunction(UCommand *command, UConnection *connection)
{
  UFunction* tmpfun = getFunction(command, connection);
  if (tmpfun) return true;
  if (!fullname_) return false;
  if (urbi::functionmap.find(fullname_->str()) !=
      urbi::functionmap.end()) return true;

  if (::urbiserver->functionbindertab.find(fullname_->str()) !=
      ::urbiserver->functionbindertab.end()) return true;

  return false;
}


//! UVariableName access to method (with cache)
  UString*
UVariableName::getMethod()
{
  if (method) return method;

  if (!fullname_) return 0;
  const char *pointPos = strstr(fullname_->str(), ".");

  if (pointPos == 0)
    method = new UString("");
  else
    method = new UString(pointPos + 1);
  return method;
}

//! UVariableName access to device (with cache)
  UString*
UVariableName::getDevice()
{
  if (device) return device;

  if (!fullname_) return 0;
  char *pointPos = const_cast<char*>(strstr(fullname_->str(), "."));
  if (pointPos == 0) return fullname_;
  pointPos[0] = 0;

  device = new UString(fullname_->str());
  pointPos[0] = '.';
  return device;
}


//! UVariableName name extraction, witch caching
/*! This method builds the name of the variable (or function) and stores it in fullname_.
  If the building blocks are static, non variable parameters (like static
  indexes in an array or constant string in a $(...)), cached is set to
  true to avoid recalculus on next call.
  */
  UString*
UVariableName::buildFullname(UCommand *command,
			     UConnection *connection,
			     bool withalias)
{
  const int    fullnameMaxSize = 1024;
  char   name[fullnameMaxSize];
  char   indexstr[fullnameMaxSize];
  UValue *e1;
  UNamedParameters* itindex;

  if (cached)
    return fullname_;

  if (str)
  {
    e1 = str->eval(command, connection);
    cached = str->isconst;

    if ((e1==0) || (e1->str==0) || (e1->dataType != DATA_STRING))
    {
      snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
	  "!!! dynamic variable evaluation failed\n");
      connection->send(tmpbuffer, command->getTag().c_str());
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
	snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
		 "!!! array index evaluation failed\n");
	connection->send(tmpbuffer, command->getTag().c_str());
	delete fullname_;
	fullname_ = 0;
	return 0;
      }
      if (e1->dataType == DATA_NUM)
	snprintf(indexstr, fullnameMaxSize, "__%d", (int)e1->val);
      else
	if (e1->dataType == DATA_STRING)
	  snprintf(indexstr, fullnameMaxSize, "__%s",
		   e1->str->str());
	else
	{
	  delete e1;
	  snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
		   "!!! invalid array index type\n");
	  connection->send(tmpbuffer, command->getTag().c_str());
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
	snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
		 "!!! array index evaluation failed\n");
	connection->send(tmpbuffer, command->getTag().c_str());
	delete fullname_;
	fullname_ = 0;
	return 0;
      }
      if (e1->dataType == DATA_NUM)
	snprintf(indexstr, fullnameMaxSize, "__%d", (int)e1->val);
      else
	if (e1->dataType == DATA_STRING)
	  snprintf(indexstr, fullnameMaxSize, "__%s",
		   e1->str->str());
	else
	{
	  delete e1;
	  snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
		   "!!! invalid array index type\n");
	  connection->send(tmpbuffer, command->getTag().c_str());
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
		   (objit->second->searchVariable(id->str(), ambiguous) != 0)
		|| (objit->second->searchFunction(id->str(), ambiguous) != 0)
		|| (objit->second->searchEvent(id->str(), ambiguous) != 0);
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
	      // does the symbol exist as a symbol local to the connection?
	      bool local_symbol = false;
	      std::string tmploc(connection->connectionTag->str());
	      tmploc = tmploc + "." + id->str();

	      if (::urbiserver->variabletab.find(tmploc.c_str()) !=
		  ::urbiserver->variabletab.end()
		  || ::urbiserver->functiontab.find(tmploc.c_str()) !=
		  ::urbiserver->functiontab.end()
		  || kernel::eventSymbolDefined (tmploc.c_str()))
		local_symbol = true;

	      if ((local_symbol) && (!function_symbol))
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
      snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
	       "!!! invalid prefix resolution\n");
      connection->send(tmpbuffer, command->getTag().c_str());
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
      char* p =  strchr(name, '.');
      HMaliastab::iterator getobj;
      if (p)
	getobj = ::urbiserver->objaliastab.find(p+1);
      else
	getobj = ::urbiserver->objaliastab.find(name);
      if (getobj != ::urbiserver->objaliastab.end())
      {
	UString* newobj = getobj->second;
	getobj = ::urbiserver->objaliastab.find(newobj->str());
	while (getobj !=  ::urbiserver->objaliastab.end())
	{
	  newobj = getobj->second;
	  getobj = ::urbiserver->objaliastab.find(newobj->str());
	}
	snprintf(name, fullnameMaxSize, "%s", newobj->str());
      }

      p =  strchr(name, '.');
      if (p)
	hmi = ::urbiserver->aliastab.find(p+1);
      else
	hmi = ::urbiserver->aliastab.find(name);
    }
    else
      hmi = ::urbiserver->aliastab.find(name);
    past_hmi = hmi;

    while  (hmi != ::urbiserver->aliastab.end())
    {
      past_hmi = hmi;
      hmi = ::urbiserver->aliastab.find(hmi->second->str());
    };

    if (past_hmi != ::urbiserver->aliastab.end())
    {
      strncpy(name, past_hmi->second->str(), fullnameMaxSize);
      nostruct = false;
      delete device; device = 0;
      delete method; method = 0; // forces recalc of device.method
      if (variable)
	variable = 0;
    }
  }
  else if (nostruct)
  {
    char* p =  strchr(name, '.');
    if (p)
    {
      if (fullname_)
	fullname_->update(p+1);
      else
	fullname_ = new UString(p+1);

      return fullname_;
    }
  }

  char* p =  strchr(name, '.');
  if (p)
  {
    p[0]=0;
    HMaliastab::iterator getobj = ::urbiserver->objaliastab.find(name);
    p[0]='.';
    if (getobj != ::urbiserver->objaliastab.end())
    {
      UString* newobj = getobj->second;
      getobj = ::urbiserver->objaliastab.find(newobj->str());
      while (getobj !=  ::urbiserver->objaliastab.end())
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
UVariableName::copy()
{
  UVariableName *ret;

  if (str)
    ret = new UVariableName(str->copy(), rooted);
  else
    if (index_obj)
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
  ret->id_type      = id_type;
  ret->local_scope  = local_scope;
  ret->doublecolon  = doublecolon;

  return ret;
}

//! Print the variable
/*! This function is for debugging purpose only.
  It is not safe, efficient or crash proof. A better version will come later.
  */
  void
UVariableName::print()
{
  ::urbiserver->debug("(VAR root=%d ", rooted);
  if (device)
    ::urbiserver->debug("device='%s' ", device->str());
  if (id)
    ::urbiserver->debug("id='%s' ", id->str());
  ::urbiserver->debug(") ");
}
