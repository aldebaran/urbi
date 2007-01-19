/*! \file uvar.cc
*******************************************************************************

File: uvar.cc\n
Implementation of the UVar class.

This file is part of
%URBI Kernel, version __kernelversion__\n
(c) Jean-Christophe Baillie, 2004-2006.

Permission to use, copy, modify, and redistribute this software for
non-commercial use is hereby granted.

This software is provided "as is" without warranty of any kind,
either expressed or implied, including but not limited to the
implied warranties of fitness for a particular purpose.

For more information, comments, bug reports: http://www.urbiforge.com

**************************************************************************** */

#include "urbi/uobject.hh"
#include "userver.hh"
#include "uvalue.hh"
#include "uvariable.hh"

namespace urbi
{

  class UVardata
  {
  public:
    UVardata (UVariable *v)
    {
      variable = v;
    }
    ~UVardata()  {};

    UVariable *variable;
  };


  //! UVar initialization.
  void
  UVar::__init()
  {
    owned = false;
    varmap[name].push_back(this);

    HMvariabletab::iterator it = ::urbiserver->variabletab.find(name.c_str());
    if (it == ::urbiserver->variabletab.end())
      // autoupdate unless otherwise specified
      vardata = new UVardata(new UVariable(name.c_str(), new ::UValue(),
					   false, false, true));
    else
    {
      vardata = new UVardata(it->second);
      //XXX why?? owned = !vardata->variable->autoUpdate;
    }
  }

  //! set own mode
  void
  UVar::setOwned()
  {
    owned = true;
    if (vardata)
      vardata->variable->autoUpdate = false;
  }

  //! UVar destructor.
  UVar::~UVar()
  {
    UVarTable::iterator varmapfind = varmap.find(name);

    if (varmapfind != varmap.end())
    {
      for (std::list<UVar*>::iterator it = varmapfind->second.begin();
	   it != varmapfind->second.end();)
	if ((*it) == this)
	  it=varmapfind->second.erase(it);
	else
	  ++it;

      if (varmapfind->second.empty())
	varmap.erase(varmapfind);
    }
    delete vardata;
  }


  //! UVar float assignment
  void
  UVar::operator = (ufloat n)
  {
    if (!invariant())
      return;

    // type mismatch is not integrated at this stage
    vardata->variable->value->dataType = ::DATA_NUM;

    if (owned)
    {
      vardata->variable->setSensorVal(n);
      vardata->variable->updated (true);
    }
    else
      vardata->variable->setFloat(n);
  }

  //! UVar string assignment
  void
  UVar::operator = (const std::string& s)
  {
    if (!invariant())
      return;

    if (vardata->variable->value->dataType == ::DATA_VOID)
      vardata->variable->value->str = new UString("");

    // type mismatch is not integrated at this stage
    ::UValue tmpv(s.c_str());
    vardata->variable->set(&tmpv);
  }

  //! UVar binary assignment
  void
  UVar::operator = (const UBinary &b)
  {
    if (!invariant())
      return;
    *vardata->variable->value=b;
    vardata->variable->updated();
  }

  //! UVar binary assignment
  void
  UVar::operator = (const UImage &b)
  {
    if (!invariant())
      return;
    *vardata->variable->value=b;
    vardata->variable->updated();
  }


  //! UVar binary assignment
  void
  UVar::operator = (const USound &b)
  {
    if (!invariant())
      return;
    *vardata->variable->value=b;
    vardata->variable->updated();
  }

  void
  UVar::operator = (const UList &l)
  {
    if (!invariant())
      return;
    *vardata->variable->value=l;
    vardata->variable->updated();
  }

  // UVar Casting
  UVar::operator int ()
  {
    //check of dataType is done inside in and out
    return owned ? (int) out() : (int) in();
  }

  UVar::operator ufloat ()
  {
    //check of dataType is done inside in and out
    return owned ? out() : in();
  }


  UVar::operator std::string ()
  {
    if (vardata && vardata->variable->value->dataType == ::DATA_STRING)
      return std::string(vardata->variable->value->str->str());
    else
      return std::string("");
  }

  UVar::operator UList()
  {
    return (UList)*vardata->variable->value;
  }

  UVar::operator UBinary()
  {
    if (vardata
	&& vardata->variable->value->dataType == ::DATA_BINARY)
      return (*vardata->variable->value).operator UBinary();
    else
      return UBinary();
  }

  UVar::operator UBinary*()
  {
    if (vardata
	&& vardata->variable->value->dataType == ::DATA_BINARY)
      return (UBinary*) vardata->variable->value;
    else
      return new UBinary();
  }

  UVar::operator UImage()
  {
    return (UImage)*vardata->variable->value;
  }

  UVar::operator USound()
  {
    return (USound)*vardata->variable->value;
  }


  //! UVar out value (read mode)
  ufloat&
  UVar::out()
  {
    static ufloat er=0;
    if (vardata && vardata->variable->value->dataType == ::DATA_NUM)
      return vardata->variable->target;
    else
      return er;
  }

  //! UVar in value (write mode)
  ufloat&
  UVar::in()
  {
    static ufloat er=0;
    if (vardata && vardata->variable->value->dataType == ::DATA_NUM)
      return vardata->variable->value->val;
    else
      return er;
  }


  void
  UVar::setProp(UProperty prop, const UValue &v)
  {
    if (!vardata)
      return;
    switch (prop)
    {
      case PROP_RANGEMIN:
	vardata->variable->rangemin = (double) v;
	break;
      case PROP_RANGEMAX:
	vardata->variable->rangemax = (double) v;
	break;
      case PROP_SPEEDMIN:
	vardata->variable->speedmin = (double) v;
	break;
      case PROP_SPEEDMAX:
	vardata->variable->speedmax = (double) v;
	break;
      case PROP_DELTA:
	vardata->variable->delta = (double) v;
	break;
      case PROP_BLEND:
	if (v.type == DATA_DOUBLE)
	  //numeric val
	  vardata->variable->blendType = (UBlendType) (int) (double) v;
	else if (v.type == DATA_STRING)
	  vardata->variable->blendType = ublendtype (std::string(v).c_str ());
    }
  }

  void
  UVar::setProp(UProperty prop, const char * v)
  {
    return setProp(prop, UValue(v));
  }

  void
  UVar::setProp(UProperty prop, double v)
  {
    return setProp(prop, UValue(v));
  }

  UValue
  UVar::getProp (UProperty prop)
  {
    if (!vardata)
      return UValue ();
    switch (prop)
    {
      case PROP_RANGEMIN:
	return UValue (vardata->variable->rangemin);
      case PROP_RANGEMAX:
	return UValue (vardata->variable->rangemax);
      case PROP_SPEEDMIN:
	return UValue (vardata->variable->speedmin);
      case PROP_SPEEDMAX:
	return UValue (vardata->variable->speedmax);
      case PROP_DELTA:
	return UValue (vardata->variable->delta);
      case PROP_BLEND:
	return UValue (vardata->variable->blendType);
    }
    return UValue ();
  }

  /*
   UBlendType
   UVar::blend()
   {
   if (vardata)
   return (UBlendType)vardata->variable->blendType;
   else
   {
   echo("Internal error on variable 'vardata', should not be zero\n");
   return UNORMAL;
   }
   }*/


  void
  UVar::requestValue()
  {
    //do nothing
  }


  //! UVar update
  void
  UVar::__update(UValue& v)
  {
    value = v;
  }

}
