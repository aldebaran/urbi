/// \file libuobject/uvar.cc

#include <libport/escape.hh>

#include <urbi/uabstractclient.hh>
#include <urbi/ublend-type.hh>
#include <urbi/uexternal.hh>
#include <urbi/uobject.hh>
#include <urbi/usyncclient.hh>

namespace urbi
{
  class UVardata
  {
  public:
    UVardata()
    {
    }
    ~UVardata()
    {
    }
  };

  //! UVar initialization
  void
  UVar::__init()
  {
    varmap()[name].push_back(this);
    URBI_SEND_PIPED_COMMAND("if (!isdef(" << name << ")) var " << name);
    vardata = 0; // unused. For internal softdevices only
    this->owned = false;
    assert (dummyUObject);

    createUCallback(dummyUObject->__name,
		    "var",
		    dummyUObject, &UObject::voidfun, name, monitormap(), false);
  }

  //! UVar out value (read mode)
  ufloat&
  UVar::out()
  {
    return value.val;
  }

  //! UVar in value (write mode)
  ufloat&
  UVar::in()
  {
    return value.val;
  }


  void
  UVar::setProp(UProperty p, const UValue& v)
  {
    URBI_SEND_PIPED_COMMAND(name << "->" << urbi::name(p) << " = " << v);
  }

  void
  UVar::setProp(UProperty p, const char* v)
  {
    URBI_SEND_PIPED_COMMAND(name << "->" << urbi::name(p) << " = " << v);
  }

  void
  UVar::setProp(UProperty p, double v)
  {
    // FIXME: This is not the right way to do it.  Generalize
    // conversions between enums and strings.
    int i = static_cast<int>(v);
    if (p == PROP_BLEND && is_blendtype(i))
      URBI_SEND_PIPED_COMMAND(name << "->" << urbi::name(p) << " = "
                              << '"'
                              << urbi::name(static_cast<UBlendType>(i))
                              << '"');
    else
      URBI_SEND_PIPED_COMMAND(name << "->" << urbi::name(p) << " = " << v);
  }

  UValue
  UVar::getProp(UProperty p)
  {
    USyncClient& uc = dynamic_cast<USyncClient&>(get_default_client());
    UMessage* m = uc.syncGet("%s->%s", name.c_str(), urbi::name(p));
    UValue v = *m->value;
    delete m;
    return v;
  }

  /*
    UBlendType
    UVar::blend()
    {
    echo("Properties not implemented in remote mode yet.\n");
    return UNORMAL;
    }
  */

  //! UVar destructor.
  UVar::~UVar()
  {
    UVarTable::iterator varmapfind = varmap().find(name);

    if (varmapfind != varmap().end())
      {
	for (std::list<UVar*>::iterator it = varmapfind->second.begin();
	     it != varmapfind->second.end();)
	  if (*it == this)
	    it=varmapfind->second.erase(it);
	  else
	    ++it;

	if (varmapfind->second.empty())
	  varmap().erase(varmapfind);
      }
  }

  // This is a stub for something that might be existing in k1.  k2 no
  // longer uses it, but SDK-Remote tries to stay compatible with it.
  class UVariable
  {};

  //! Set the UVar in "zombie" mode  (the attached UVariable is dead)
  void
  UVar::setZombie ()
  {
    // no effect in remote mode.
  }

  //! Return the internal variable.
  UVariable*
  UVar::variable()
  {
    return 0;
  }

  //! UVar reset  (deep assignement)
  void
  UVar::reset (ufloat n)
  {
    *this = n;
  }

  //! UVar float assignment
  void
  UVar::operator = (ufloat n)
  {
    URBI_SEND_PIPED_COMMAND(name << "=" << n);
  }

  //! UVar string assignment
  void
  UVar::operator= (const std::string& s)
  {
    URBI_SEND_PIPED_COMMAND(name << "=\"" << libport::escape(s, '"') << '"');
  }

  //! UVar binary assignment
  void
  UVar::operator = (const UBinary& b)
  {
    getDefaultClient()->startPack();
    // K1 only supports a binary at top level within ';' and no other separator.
    if (getDefaultClient()->kernelMajor() < 2)
      URBI_SEND_COMMAND("");
    (*getDefaultClient()) << name << "=";
    getDefaultClient()->sendBinary(b.common.data, b.common.size,
                                   b.getMessage());
    (*getDefaultClient()) << ";";
    getDefaultClient()->endPack();
  }

  void
  UVar::operator= (const UImage& i)
  {
    //we don't use UBinary Image ctor because it copies data
    UBinary b;
    b.type = BINARY_IMAGE;
    b.image = i;
    (*this) = b;
    b.common.data = 0; //required, dtor frees data
  }

  void
  UVar::operator= (const USound& i)
  {
    //we don't use UBinary Image ctor because it copies data
    UBinary b;
    b.type = BINARY_SOUND;
    b.sound = i;
    (*this) = b;
    b.common.data = 0; //required, dtor frees data
  }

  void
  UVar::operator= (const UList& l)
  {
    UValue v;
    v.type = DATA_LIST;
    v.list = &const_cast<UList&>(l);
    URBI_SEND_PIPED_COMMAND(name << "=" << v);
    v.type = DATA_VOID;
    v.list = 0;
  }


  UVar::operator int ()
  {
    return (int) value;
  };

  UVar::operator ufloat ()
  {
    return (ufloat) value;
  };


  UVar::operator std::string ()
  {
    return (std::string) value;
  };


  UVar::operator UBinary()
  {
    return value;
  };

  UVar::operator UBinary*()
  {
    return new UBinary(value.operator UBinary());
  };

  UVar::operator UImage()
  {
    return (UImage) value;
  };

  UVar::operator USound()
  {
    return (USound) value;
  };

  UVar::operator UList()
  {
    return (UList) value;
  };

  //! UVar update
  void
  UVar::__update(UValue& v)
  {
# ifdef LIBURBIDEBUG
    std::cout << "  Variable " << name << " updated to : ";

    switch (v.type)
      {
      case DATA_DOUBLE:
	std::cout << (double)v << std::endl;
	break;
      case DATA_STRING:
	std::cout << (std::string)v << std::endl;
	break;
      case DATA_BINARY:
      case DATA_LIST:
      case DATA_OBJECT:
      case DATA_VOID:
	break;
      }
# endif
    value = v;
  }

  //! set own mode
  void
  UVar::setOwned()
  {
    owned = true;
  }

  //! Get Uvalue type
  UDataType
  UVar::type () const
  {
    return value.type;
  }

  void
  UVar::requestValue()
  {
    //build a getvalue message  that will be parsed and returned by the server
    URBI_SEND_PIPED_COMMAND(externalModuleTag << "<<"
		      <<'[' << UEM_ASSIGNVALUE << ","
		      << '"' << name << '"' << ',' << name << ']');
  }

  void
  UVar::syncValue ()
  {
    USyncClient&	client = dynamic_cast<USyncClient&> (URBI(()));
    UMessage*		m;
    char		tag[32];

    client.makeUniqueTag(tag);
    m = client.syncGetTag("{if (isdef (%s) && !isvoid (%s)) { %s<<%s } else { %s<<1/0 }};",
                     tag, 0, name.c_str (), name.c_str (), tag, name.c_str (), tag);
    if (m->type == MESSAGE_DATA)
      __update (*m->value);
  }

} //namespace urbi
