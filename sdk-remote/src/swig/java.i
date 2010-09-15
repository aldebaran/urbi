// -*- C++ -*-

%module(directors="1") urbi
%{

///
/// liburbi includes:
///
#include <sstream>
#include <urbi/ubinary.hh>
#include <urbi/uimage.hh>
#include <urbi/usound.hh>
#include <urbi/umessage.hh>
#include <urbi/uvalue.hh>
#include <urbi/ucontext.hh>
#include <urbi/ucontext-impl.hh>
#include <urbi/uabstractclient.hh>
#include <urbi/uclient.hh>
#include <urbi/usyncclient.hh>
#include <urbi/uconversion.hh>
#include <urbi/urbi-root.hh>
#include <urbi/umain.hh>

using urbi::BinaryData;
using urbi::UImage;
using urbi::USound;
using urbi::UList;
using urbi::UVar;
using urbi::UMessage;
using urbi::UAutoValue;
using urbi::UEvent;
using urbi::UObjectMode;
using urbi::impl::UVarImpl;
using urbi::impl::UObjectImpl;
using urbi::impl::UGenericCallbackImpl;
using urbi::UTimerCallback;
using urbi::TimerHandle;
using urbi::UProp;
using urbi::baseURBIStarter;


namespace urbi {

  typedef signed char * bytetype;

  class UCallbackInterface
  {
  public:
    UCallbackInterface () {};
    virtual UCallbackAction onMessage (const UMessage &) = 0;
    virtual ~UCallbackInterface () {};
  };

  /// temporary buffer used to copy images
  static unsigned char *img_data = 0;
};

///
/// UObject Remote includes:
///
#include <urbi/uproperty.hh>
#include <urbi/uvar.hh>

#include <urbi/ustarter.hh>

namespace urbi
{

  class URBIStarterJAVA : public baseURBIStarter
  {
  public:
    URBIStarterJAVA (const std::string& name, bool local = false)
      : baseURBIStarter(name, local)
    {
      list().push_back(this);
    }

    virtual ~URBIStarterJAVA()
    {
      clean();
    }

    void clean()
    {
      /// Delete nothing since allocation is done in the Java side
      /// (it will be freed by Java GC).
      list().remove(this);
    }

    virtual UObject* instanciate(impl::UContextImpl* ctx,
				 const std::string& n)
    {
      assert(0);
      return 0;
    }

  };
};


%}

// Swig doesn't like these macro
#define URBI_SDK_API
#define __attribute__(foobar)
#define ATTRIBUTE_DEPRECATED


/**************************************************************************\
|                                                                          |
|                              Liburbi                                     |
|                                                                          |
\**************************************************************************/


%include "typemaps.i"

/// We want that swig maps C++ std::string to Java String
%include "std_string.i"

/// We want that swig generates Javastyle enums, so we include this file:
%include "enums.swg"
%javaconst(1);

// Rename all operators
%rename("set") operator=;
%rename("get") operator[];
%rename("getConst") operator[](int) const;

%rename("isEqual") operator ==;
%rename("getDouble") operator ufloat;
%rename("getString") operator std::string;
%rename("getInt") operator int;
%rename("getUnsignedInt") operator unsigned int;
%rename("getLong") operator long;
%rename("getUnsignedLong") operator unsigned long;
%rename("getBool") operator bool;
%rename("getUBinary") operator UBinary*;
%rename("getUBinary") operator const UBinary&;
%rename("getUList") operator UList;
%rename("getUImage") operator UImage;
%rename("getUSound") operator USound;
%rename("getUDictionary") operator UDictionary;
//%rename("getUValue") operator urbi::UValue;


// or ignore them
%ignore operator,;
%ignore operator<<;
%ignore operator>>;
%ignore operator();

%ignore wavheader;

/// Ignore the attribute message
/// Because swig generate a accessor named getMessage
/// which conflict with getMessage method.
/// We manually rewrite the accessor in %extend UBinary
%ignore urbi::UBinary::message;

/// Ignore attribute client (setter/getter conflict)
%ignore urbi::UMessage::client;

/// Ignore global variable defaultClient (setter/getter conflict)
%ignore urbi::defaultClient;

/// Tell swig that UClient is not abstract
%feature("notabstract") UClient;

namespace urbi
{
  //typedef unsigned int UCallbackID;
  class UAbstractClient;
  class UCallback;
  class UCustomCallback;
  class UValue;
  class UCallbackWrapper;
  class UCallbackInterface;
  class UClient;
};

%include "arrays_java.i"

/// Tell swig that ufloat is a double (I wonder if it's that much
/// a good idea...)
typedef double ufloat;


////////////////////////////
///                      ///
///        UImage        ///
///                      ///
////////////////////////////

namespace urbi
{

  /// We want that swig generate a byte[] array for the image data.
  /// This is done easyly thanks to:
  /// %include "arrays_java.i"
  /// But the array must be of 'char *' to be converted to byte[]
  /// So we cast 'unsigned char *' to 'signed char *' and we say that
  /// it is converted to 'signed char[]' so that then it get converted to
  /// byte[]
  /// NB: the typedef is done to convert char* to byte[] only this time,
  /// not all the time for all classes

  typedef signed char * bytetype;
  %apply signed char[] {bytetype};

};

%include "urbi/uimage.hh"

namespace urbi
{
  %extend UImage
  {
    /// Get the data as a byte[] array
    bytetype getDataAsByte ()
      {
	return (signed char*) self->data;
      }

    /// FIXME: we might want the data as some other types array
  }
}


////////////////////////////
///                      ///
///        USound        ///
///                      ///
////////////////////////////

%ignore urbi::USound::size;
%ignore urbi::USound::channels;
%ignore urbi::USound::rate;
%ignore urbi::USound::sampleSize;

%include "urbi/usound.hh"

namespace urbi
{
  %extend USound
  {
    /// Get the data as a byte[] array
    bytetype getDataAsByte ()
      {
	return (signed char*) self->data;
      }

    int getChannels()
    {
      return self->channels;
    }

    void setChannels(int channels)
    {
      self->channels = channels;
    }

    int getSize()
    {
      return self->size;
    }

    void setSize(int size)
    {
      self->size = size;
    }

    int getRate()
    {
      return self->rate;
    }

    void setRate(int rate)
    {
      self->rate = rate;
    }

    int getSampleSize()
    {
      return self->sampleSize;
    }

    void setSampleSize(int sampleSize)
    {
      self->sampleSize = sampleSize;
    }
    /// FIXME: we might want the data as some other types array
  }

};


////////////////////////////
///                      ///
///       UBinary        ///
///                      ///
////////////////////////////

%include "urbi/ubinary.hh"

namespace urbi
{
  %extend UBinary
  {
    /// Accessor for the UImage
    UImage getUImage ()
      {
	return self->image;
      }

    /// Accessor for the USound
    USound getUSound ()
      {
	return self->sound;
      }

    long getSize ()
      {
	return self->common.size;
      }

    std::string getExtraHeader ()
      {
	return self->message;
      }

    void setExtraHeader (std::string msg)
      {
	self->message = msg;
      }

    /// FIXME: we want to be able to retrieve the data in common in arrays
    /// of various type
  }

};



////////////////////////////
///                      ///
///        UList         ///
///                      ///
////////////////////////////

  /// Thanks to
%include "std_vector.i"
  /// the C++ std::vector type is mapped to Vector type in Java.
  /// We declare the type 'UValueVector' corresponding to the C++ templated
  /// type. and Swig generate the class with some convenient functions to
  /// manipulate the Vector of UValue
namespace std {
  %template(UValueVector) vector<urbi::UValue*>;
};

%ignore urbi::UValue::UValue(const void*);
%ignore urbi::UValue::UValue(long, bool);
%ignore urbi::UValue::UValue(unsigned int, bool);
%ignore urbi::UValue::UValue(unsigned long, bool);
%ignore urbi::UValue::UValue(long long, bool);
%ignore urbi::UValue::operator=(const void*);
%ignore urbi::UValue::parse;
%ignore urbi::UValue::print;
%ignore urbi::UValue::copy;
%include "urbi/uvalue.hh"


////////////////////////////
///                      ///
///       UValue         ///
///                      ///
////////////////////////////


namespace urbi
{

  %extend UValue {

    ///bool isStringNull () { return self->stringValue == 0; }

    /// FIXME !!! This can SEGFAULT !!
    ///std::string		getString () { return *self->stringValue; }

    double		getDouble () { return self->val; }

    ///UBinary*		getUBinary () { return self->binary; }
    ///UList*		getUList () { return self->list; }

    void		setString (std::string s)
      {
	self->stringValue = new std::string (s);
      }

    void		setDouble (double d) { self->val = d; }

    void		setUBinary (UBinary& b)
      {
	self->binary = new urbi::UBinary (b);
      }

    void		setUList (UList& l)
      {
	self->list = new urbi::UList (l);
      }

    std::string		toString ()
      {
	std::ostringstream os;
	self->print (os);
	return os.str ();
      }
  }

};


%include "urbi/uabstractclient.hh"


////////////////////////////
///                      ///
///      UMessage        ///
///                      ///
////////////////////////////

%include "urbi/umessage.hh"

namespace urbi
{
  %extend UMessage
  {
    UAbstractClient& getClient ()
      {
	return self->client;
      }
  }
};


////////////////////////////
///                      ///
///  UCallbackInterface  ///
///                      ///
////////////////////////////


///
/// Allow C++ virtual methods of UCallbackInterface to
/// be redefined in the Java side, and generate code so that
/// Java redefined method be called from the C++ side.
/// This is an advance and _experimental_ swig feature.
///
%feature("director") urbi::UCallbackInterface;

namespace urbi
{
  class UCallbackInterface
  {
  public:
    UCallbackInterface ();
    virtual UCallbackAction onMessage (const UMessage &msg) = 0;
    virtual ~UCallbackInterface ();
  };
};



////////////////////////////
///                      ///
///   UAbstractClient    ///
///                      ///
////////////////////////////

namespace urbi
{
  %extend UAbstractClient
  {

    UCallbackID setCallback (UCallbackInterface& ref, const char * tag)
    {
      return self->setCallback (ref, &urbi::UCallbackInterface::onMessage, tag);
    }

    void sendBin (bytetype bin, int len) { self->sendBin (bin, len); }
    void sendBin (bytetype bin, int len, char *header) { self->sendBin (bin, len, header); }

  }

};

////////////////////////////
///                      ///
///       UClient        ///
///                      ///
////////////////////////////

/// Generate code for UClient:
%include "urbi/uclient.hh"


////////////////////////////
///                      ///
///     USyncClient      ///
///                      ///
////////////////////////////

/// Generate code for UClient:
%include "urbi/usyncclient.hh"


namespace urbi
{
  %extend USyncClient
  {

  }
};

%ignore convertYCrCbtoYCbCr;
%ignore convertRGBtoYCrCb;
%ignore convertYCrCbtoRGB;
%ignore convertJPEGtoYCrCb;
%ignore convertJPEGtoRGB;
%ignore convertRGBtoJPEG;
%ignore convertYCrCbtoJPEG;

%include "urbi/uconversion.hh"



/**************************************************************************\
|                                                                          |
|                         UObjects Remote                                  |
|                                                                          |
\**************************************************************************/


/// transform Java String[] to C++ char**
%include "various.i"

/// and apply the transformation to const char* argv[]
%apply char**STRING_ARRAY {const char* argv[]};

namespace urbi
{
  class UVar;
  class UObject;
  class UObjectHub;
  class UStartlistobjectlist;
};

%include "urbi/uproperty.hh"

%include "urbi/ucontext-impl.hh"
%include "urbi/ucontext.hh"

 //%ignore urbi::UVar::value;
 //%ignore urbi::UVar::set_value;
 //%ignore urbi::UVar::get_value;
%ignore urbi::UVar::name;
%ignore urbi::UVar::set_name;
%ignore urbi::UVar::get_name;
//%ignore urbi::UVar::variable;
//%ignore urbi::UVar::__update;
//%ignore urbi::UVar::setZombie;
//%ignore urbi::UVar::in;
//%ignore urbi::UVar::out;
%ignore urbi::UVar::rangemin;
%ignore urbi::UVar::rangemax;
%ignore urbi::UVar::speedmin;
%ignore urbi::UVar::speedmax;
%ignore urbi::UVar::delta;
%ignore urbi::UVar::blend;
%include "urbi/uvar.hh"

namespace urbi
{
  %extend UVar
  {
    const UValue& getUValue ()
      {
    	return self->val ();
      }

    // void setUValue (UValue& v)
    //   {
    // 	self->set_value (v);
    //   }

    std::string getName ()
      {
	return self->get_name ();
      }

    void setName (std::string name)
      {
	self->set_name (name);
      }
  }
};

%feature("director") urbi::UObject;

/// ignore 'load' attribute (swig generate the setter and try to do
/// load = some_UVar which is not possible since you can't assign
/// an UVar to an UVar)
%ignore urbi::UObject::load;
%ignore urbi::UObject::gc;
%ignore urbi::UObject::members;
%ignore urbi::UObject::cloner;
%include "urbi/uobject.hh"

namespace urbi
{
  %extend UObject
  {
    /// 'load' attibutte setter
    void setLoad (UValue val)
      {
	self->load = val;
      }

    /// 'load' attibutter getter
    const UVar& getLoad ()
      {
	return self->load;
      }

    void setCloner (urbi::baseURBIStarter *cloner)
    {
      self->cloner = cloner;
    }
  }
};

//%feature("director") urbi::baseURBIStarter;
%feature("director") urbi::URBIStarterJAVA;

/// Tell swig that urbi::URBIStarterJAVA is not abstract
%feature("notabstract") urbi::URBIStarterJAVA;

%include "urbi/ustarter.hh"



namespace urbi
{
  class URBIStarterJAVA : public baseURBIStarter
  {
  public:
    URBIStarterJAVA (const std::string& name, bool local = false);
    virtual ~URBIStarterJAVA();

  public:

    /// To reimplement in Java subclasses:
    virtual UObject* instanciate(impl::UContextImpl* ctx,
				 const std::string& n);
  };


};

%include "urbi/urbi-root.hh"
%include "urbi/umain.hh"
