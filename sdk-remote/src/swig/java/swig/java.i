/*
 * Copyright (C) 2010, 2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

%module(directors="1") urbi

 // Disable warnings we are not interested in.
 // 312. Unnamed nested class not currently supported (ignored).
 // 322. Multiple declarations.
 // 325. Nested class not currently supported (name ignored).
#pragma SWIG nowarn=312,322,325

 // Define __attribute__, ATTRIBUTE_* etc., before that SWIG tries to
 // read them.
%include "libport/compiler.hh"

%{

// Disable warnings about deprecated functions, as we generate
// bindings for them too.
#define ATTRIBUTE_DEPRECATED

///
/// liburbi includes:
///

#include <libport/cmath>
#include <sstream>
#include <urbi/ubinary.hh>
#include <urbi/uimage.hh>
#include <urbi/usound.hh>
#include <urbi/umessage.hh>
#include <urbi/uvalue.hh>
#include <urbi/uevent.hh>
#include <urbi/ucontext.hh>
#include <urbi/ucontext-impl.hh>
#include <urbi/uabstractclient.hh>
#include <urbi/uclient.hh>
#include <urbi/usyncclient.hh>
#include <urbi/uconversion.hh>
#include <urbi/urbi-root.hh>
#include <urbi/umain.hh>

using urbi::BinaryData;
using urbi::TimerHandle;
using urbi::UAutoValue;
using urbi::UDataType;
using urbi::UDictionary;
using urbi::UEvent;
using urbi::UImage;
using urbi::UList;
using urbi::UMessage;
using urbi::UObjectMode;
using urbi::UProp;
using urbi::USound;
using urbi::UTimerCallback;
using urbi::UVar;
using urbi::baseURBIStarter;
using urbi::impl::UGenericCallbackImpl;
using urbi::impl::UObjectImpl;
using urbi::impl::UVarImpl;


namespace urbi
{

  typedef signed char* bytetype;

  class UCallbackInterface
  {
  public:
    UCallbackInterface() {};
    virtual UCallbackAction onMessage(const UMessage &) = 0;
    virtual ~UCallbackInterface() {};
  };

  /// temporary buffer used to copy images
  static unsigned char* img_data = 0;
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
    URBIStarterJAVA(const std::string& name, bool local = false)
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

%include "generated-comments.i"

// SWIG doesn't like these macros.
// FIXME: Factor all the Libport.Compiler construct handling?
#define URBI_SDK_API
#define LIBPORT_API

%include "urbi/fwd.hh"


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
%rename("setValue") operator=;
%rename("get") operator[];
%rename("getConst") operator[](int) const;
%rename("getConst") operator[](size_t) const;

%rename("isEqual") operator ==;
%rename("doubleValue") operator ufloat;
%rename("stringValue") operator std::string;
%rename("intValue") operator int;
%rename("longValue") operator unsigned int;
%rename("intValue") operator long;
%rename("longValue") operator unsigned long;
%rename("booleanValue") operator bool;
%rename("ubinaryValue") operator UBinary*;
%rename("ubinaryValue") operator const UBinary&;
%rename("ulistValue") operator UList;
%rename("uimageValue") operator UImage;
%rename("usoundValue") operator USound;
%rename("udictionaryValue") operator UDictionary;
//%rename("getUValue") operator urbi::UValue;


// or ignore them
%ignore operator,;
%ignore operator<<;
%ignore operator>>;
%ignore operator();

%ignore wavheader;

/// Ignore global variable defaultClient (setter/getter conflict)
namespace urbi
{
  %ignore callback(UCallback);
  %ignore callback(UCustomCallback, void*);
  %ignore defaultClient;
  %ignore default_stream;
  %ignore kernelMajor;
  %ignore send(const char*);
  %ignore send(const void*, size_t);
  %ignore unarmorAndSend;
}

/// Tell swig that UClient is not abstract
%feature("notabstract") UClient;

%include "arrays_java.i"

/// Tell swig that ufloat is a double (I wonder if it's that much
/// a good idea...)
typedef double ufloat;

namespace libport
{
  /// Microseconds.
  typedef long long utime_t;
};

// Tell SWIG about size_t;
typedef unsigned int size_t;

// Java typemap
// change default SWIG mapping of unsigned char* return values
// to byte[]
//
// Assumes that there are the following function defined (T is the
// type of the class possessing the method we currently process):
// void setSize(T* b, size_t size)
// size_t getSize(T* b)
//
// inspired from
// https://valelab.ucsf.edu/svn/micromanager2/trunk/MMCoreJ_wrap/MMCoreJ.i (LGPL)

%typemap(jni) unsigned char*        "jbyteArray"
%typemap(jtype) unsigned char*      "byte[]"
%typemap(jstype) unsigned char*     "byte[]"
%typemap(out) unsigned char* %{
  $result = SWIG_JavaArrayOutSchar(jenv, (signed char*) $1, getSize((arg1)));
%}

// Map input argument: java byte[] -> C++ unsigned char *
%typemap(in) unsigned char* {
  if (!arg1) {
    SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "null array");
    return $null;
  }
  size_t size = JCALL1(GetArrayLength, jenv, $input);
  setSize((arg1), size);
  $1 = (unsigned char*) JCALL2(GetByteArrayElements, jenv, $input, 0);
}

%typemap(freearg) unsigned char* {
  // Allow the Java byte array to be garbage collected.
  // JNI_ABORT = Don't alter the original array.
  // JCALL3(ReleaseByteArrayElements, jenv, $input, (jbyte *) $1, JNI_ABORT);
  JCALL3(ReleaseByteArrayElements, jenv, $input, (jbyte *) $1, JNI_COMMIT);
}

// change Java wrapper output mapping for unsigned char*
%typemap(javaout) unsigned char* {
  return $jnicall;
}

%typemap(javain) unsigned char* "$javainput"


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
  /// But the array must be of 'char*' to be converted to byte[]
  /// So we cast 'unsigned char*' to 'signed char*' and we say that
  /// it is converted to 'signed char[]' so that then it get converted to
  /// byte[]
  /// NB: the typedef is done to convert char* to byte[] only this time,
  /// not all the time for all classes

  typedef signed char* bytetype;
  %apply signed char[] {bytetype};

};

%include "urbi/uimage.hh"

%{
  void setSize(urbi::UImageImpl* b, size_t size)
  {
    b->size = size;
  }
  size_t getSize(urbi::UImageImpl* b)
  {
    return b->size;
  }
%}

namespace urbi
{
  %extend UImageImpl {
    // Delete data if allocated _JAVA_SIDE_
    void deleteData()
    {
      delete[] self->data;
      self->data = 0;
    }
  }
};

////////////////////////////
///                      ///
///        USound        ///
///                      ///
////////////////////////////

namespace urbi
{
  %ignore USound::dump;

  %extend USoundImpl {
    // Place this definition of data before the usound.hh header
    // so that swig consider data as unsigned char and generate correct
    // setter to -> byte[]
    unsigned char* data;

    // Delete data if allocated _JAVA_SIDE_
    void deleteData()
    {
      delete[] self->data;
      self->data = 0;
    }
  }
};

%include "urbi/usound.hh"

%{
  void urbi_USoundImpl_data_set(urbi::USoundImpl* b, unsigned char* data)
  {
    b->data = (char*) data;
  }
  unsigned char* urbi_USoundImpl_data_get(urbi::USoundImpl* b)
  {
    return (unsigned char*) b->data;
  }
  void setSize(urbi::USoundImpl* b, size_t size)
  {
    b->size = size;
  }
  size_t getSize(urbi::USoundImpl* b)
  {
    return b->size;
  }
%}


////////////////////////////
///                      ///
///       UBinary        ///
///                      ///
////////////////////////////


namespace urbi
{
  %ignore BinaryData;

  // Ignore the attribute message because SWIG generates a accessor
  // named getMessage which conflicts with the getMessage method.  We
  // manually rewrite the accessor in %extend UBinary
  %ignore UBinary::message;
  %ignore UBinary::parse;
  %ignore UBinary::print;

  %extend UBinary
  {
    /// Accessor for the UImage
    UImage uimageValue()
    {
      return self->image;
    }

    /// Accessor for the USound
    USound usoundValue()
    {
      return self->sound;
    }

    size_t getSize()
    {
      return self->common.size;
    }

    std::string getExtraHeader()
    {
      return self->message;
    }

    void setExtraHeader(const std::string& msg)
    {
      self->message = msg;
    }

    // make swig generate getData and setData
    unsigned char* data;

    /// FIXME: we want to be able to retrieve the data in common in arrays
    /// of various type
  }
};

%include "urbi/ubinary.hh"

%{
  void urbi_UBinary_data_set(urbi::UBinary* b, unsigned char* data)
  {
    b->common.data = data;
  }

  unsigned char* urbi_UBinary_data_get(urbi::UBinary* b)
  {
    return (unsigned char*) b->common.data;
  }

  void setSize(urbi::UBinary* b, size_t size)
  {
    b->common.size = size;
  }

  size_t getSize(urbi::UBinary* b)
  {
    return b->common.size;
  }
%}


////////////////////////////
///                      ///
///        UList         ///
///                      ///
////////////////////////////

namespace urbi
{
  %ignore UList::print;
  %ignore UList::begin() const;
  %ignore UList::end() const;
  %ignore UList::operator[](size_t i) const;

  %define ULIST_PUSH_BACK(value_type)
    %extend UList {
      UList& push_back(value_type v)
      {
	return self->push_back(v);
      }
    }
  %enddef

  ULIST_PUSH_BACK(const UValue&)
  ULIST_PUSH_BACK(const UList&)
  ULIST_PUSH_BACK(const UBinary&)
  ULIST_PUSH_BACK(const USound&)
  ULIST_PUSH_BACK(const UImage&)
  ULIST_PUSH_BACK(const std::string&)

   // See http://www.swig.org/Doc2.0/SWIGDocumentation.html#SWIGPlus,
   // some overloads cannot be solved.
   //  ULIST_PUSH_BACK(int)
  ULIST_PUSH_BACK(long)
  ULIST_PUSH_BACK(long long)
  ULIST_PUSH_BACK(double)
  ULIST_PUSH_BACK(float)
  ULIST_PUSH_BACK(char)
  ULIST_PUSH_BACK(bool)

  %extend UList {

    std::string toString()
      {
	std::ostringstream os;
	os << *self;
	return os.str();
      }

  }
}

%include "urbi/ulist.hh"


  /// Thanks to
%include "std_vector.i"
  /// the C++ std::vector type is mapped to Vector type in Java.
  /// We declare the type 'UValueVector' corresponding to the C++ templated
  /// type. and Swig generate the class with some convenient functions to
  /// manipulate the Vector of UValue
namespace std
{
  %template(UValueVector) vector<urbi::UValue*>;
  %template(StringVector) vector<std::string>;
  %template(UNamedValueVector) std::vector<urbi::UNamedValue>;
};

%include "std_pair.i"

namespace std
{
  %template(IntPair) std::pair<int, int>;
}

/*------------------.
| Boost.SharedPtr.  |
`------------------*/

%include "boost_shared_ptr.i"

namespace boost
{
  %template(FinallySharedPtr) boost::shared_ptr<libport::Finally>;
  %template(TimerHandle) boost::shared_ptr<std::string>;

  %extend shared_ptr<std::string>
  {
    static boost::shared_ptr<std::string> create(const std::string& s)
    {
      return boost::shared_ptr<std::string>(new std::string(s));
    }

    std::string get()
    {
      return **self;
    }
  }
};




////////////////////////////
///                      ///
///     UDictionary      ///
///                      ///
////////////////////////////

%include "udictionary-javacode.i"
%include "boost_unordered_map.i"

namespace boost
{
  %template(UDictionary) unordered_map<std::string, urbi::UValue>;
  %template(UDictionaryCPPIterator) iterator_wrapper<std::string, urbi::UValue>;

  %define UDIRECTORY_PUT(value_type)

  %extend unordered_map<std::string, urbi::UValue>
  {
    value_type put(const std::string& key, value_type v)
    {
      return (*self)[key] = v;
    }
  }
  %enddef

  UDIRECTORY_PUT(const urbi::UList&)
  UDIRECTORY_PUT(const urbi::UBinary&)
  UDIRECTORY_PUT(const urbi::USound&)
  UDIRECTORY_PUT(const urbi::UImage&)
  UDIRECTORY_PUT(const std::string&)
  UDIRECTORY_PUT(int)
  UDIRECTORY_PUT(long long)
  UDIRECTORY_PUT(double)
  UDIRECTORY_PUT(bool)

  %extend unordered_map<std::string, urbi::UValue>
  {
    float put(const std::string& key, float v)
    {
      return (ufloat) ((*self)[key] = v);
    }

    char put(const std::string& key, char v)
    {
      return (int) ((*self)[key] = v);
    }
  }
}


////////////////////////////
///                      ///
///       UValue         ///
///                      ///
////////////////////////////

namespace urbi
{

  %ignore UValue::UValue(const void*);
  %ignore UValue::UValue(const void*, bool);
  %ignore UValue::UValue(long, bool);
  %ignore UValue::UValue(unsigned int, bool);
  %ignore UValue::UValue(unsigned long, bool);
  %ignore UValue::UValue(long long, bool);
  // Use the UValue::set functions instead.
  %ignore UValue::operator=;
  %ignore UValue::parse;
  %ignore UValue::print;
  %ignore UValue::copy;

  %rename("setValue") UValue::set;

  %extend UValue
  {
    double doubleValue()
    {
      return self->val;
    }

    void setValue(std::string s)
    {
      self->stringValue = new std::string(s);
    }

    void setValue(double d)
    {
      self->val = d;
    }

    void setValue(UBinary& b)
    {
      self->binary = new urbi::UBinary(b);
    }

    void setValue(UList& l)
    {
      self->list = new urbi::UList(l);
    }

    std::string toString()
    {
      std::ostringstream os;
      self->print(os);
      return os.str();
    }
  }

};

%include "urbi/uvalue.hh";




////////////////////////////
///                      ///
///      UMessage        ///
///                      ///
////////////////////////////

namespace urbi
{

  %ignore UMessage::UMessage(UAbstractClient& client, int timestamp,
                             const char* tag, const char* message,
                             const binaries_type& bins = binaries_type());

  // Ignore attribute client (setter/getter conflict)
  %ignore UMessage::client;
  %ignore UMessage::print;

  %extend UMessage
  {
    UAbstractClient& getClient()
      {
	return self->client;
      }
  }
};

%include "urbi/umessage.hh"



////////////////////////////
///                      ///
///   UAbstractClient    ///
///                      ///
////////////////////////////

namespace urbi
{
  %ignore UAbstractClient::putFile(const void*, size_t, const char*);
  %ignore UAbstractClient::send(std::istream&);
  %ignore UAbstractClient::send(const char*, ...);
  %ignore UAbstractClient::sendBin(const void*, size_t);
  %ignore UAbstractClient::sendBin(const void*, size_t, const char*, ...);
  %ignore UAbstractClient::sendBinary;
  %ignore UAbstractClient::sendCommand(UCallback, const char*, ...);
  %ignore UAbstractClient::sendCommand(UCustomCallback, void*, const char*, ...);
  %ignore UAbstractClient::setCallback(UCallback, const char*);
  %ignore UAbstractClient::setCallback(UCustomCallback, void*, const char*);
  %ignore UAbstractClient::stream_get;
  %ignore UAbstractClient::vpack;

  %ignore UCallbackWrapperF;
  %ignore UCallbackWrapperCF;

  %extend UAbstractClient
  {

    UCallbackID setCallback(UCallbackInterface& ref, const char* tag)
    {
      return self->setCallback(ref, &urbi::UCallbackInterface::onMessage, tag);
    }

    void sendBin(bytetype bin, int len)
    {
      self->sendBin(bin, len);
    }

    void sendBin(bytetype bin, int len, char* header)
    {
      self->sendBin(bin, len, header);
    }

  }

};


%include "urbi/uabstractclient.hh"


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
    UCallbackInterface();
    virtual UCallbackAction onMessage(const UMessage &msg) = 0;
    virtual ~UCallbackInterface();
  };
};



////////////////////////////
///                      ///
///       UClient        ///
///                      ///
////////////////////////////

// FIXME: handle options
namespace urbi
{
  %ignore UClient::UClient(const std::string&, unsigned, size_t, const UClient::options&);
  %ignore UClient::send(std::istream&);
  %ignore UClient::pong;
}

/// Generate code for UClient:
%include "urbi/uclient.hh"


////////////////////////////
///                      ///
///     USyncClient      ///
///                      ///
////////////////////////////

namespace urbi
{
  %ignore USyncClient::USyncClient(const std::string&, unsigned, size_t, const USyncClient::options&);
  %ignore USyncClient::getOptions;
  %ignore USyncClient::listen;
  %ignore USyncClient::setDefaultOptions;
  %ignore USyncClient::syncGet(const char*, ...);
  %ignore USyncClient::syncGetDevice(const char*, const char*, double &);
  %ignore USyncClient::syncGetDevice(const char*, const char*, double &, libport::utime_t);
  %ignore USyncClient::syncGetDevice(const char*, double &);
  %ignore USyncClient::syncGetDevice(const char*, double &, libport::utime_t);
  %ignore USyncClient::syncGetImage(const char*, void*, size_t&, int, int, size_t&, size_t&);
  %ignore USyncClient::syncGetImage(const char*, void*, size_t&, int, int, size_t&, size_t&, libport::utime_t);
  %ignore USyncClient::syncGetNormalizedDevice(const char*, double &);
  %ignore USyncClient::syncGetNormalizedDevice(const char*, double &, libport::utime_t);
  %ignore USyncClient::syncGetResult(const char*, double &);
  %ignore USyncClient::syncGetResult(const char*, double &, libport::utime_t);
  %ignore USyncClient::syncSend(const void*, size_t);

  // FIXME: handle options (and setDefaultOptions and getOptions)
  // FIXME: handle listen ?
  %extend USyncClient
  {

  }
}

/// Generate code for UClient:
%include "urbi/usyncclient.hh"


 /*----------------------.
 | urbi/uconversion.hh.  |
 `----------------------*/

namespace urbi
{
  %ignore convertJPEGtoRGB;
  %ignore convertJPEGtoYCrCb;
  %ignore convertRGBtoGrey8_601;
  %ignore convertRGBtoJPEG;
  %ignore convertRGBtoYCbCr;
  %ignore convertYCbCrtoRGB;
  %ignore convertYCrCbtoJPEG;
  %ignore convertYCrCbtoYCbCr;
};


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
%apply char**STRING_ARRAY {const char** argv};
%apply char**STRING_ARRAY {char* argv[]};
%apply char**STRING_ARRAY {char** argv};


%include "urbi/uproperty.hh"


 /*------------------------.
 | urbi/ucontext-impl.hh.  |
 `------------------------*/

namespace urbi
{
  namespace impl
  {

    %ignore UVarImpl::timestamp;

    %ignore UContextImpl::getGenericCallbackImpl;
    %ignore UContextImpl::getIoService;
    %ignore UContextImpl::hubs;
    %ignore UContextImpl::initialized;
    %ignore UContextImpl::objects;
    %ignore UContextImpl::send(const char*);
    %ignore UContextImpl::send(const void*, size_t);
    %ignore UContextImpl::setTimer;
  }
}

%include "urbi/ucontext-impl.hh"


////////////////////////////
///                      ///
///      UContext        ///
///                      ///
////////////////////////////

namespace urbi
{
  %ignore UContext::send(const char *);
  %ignore UContext::send(const void*, size_t);
  %ignore UContext::yield_for(libport::utime_t) const;
  %ignore UContext::yield_until(libport::utime_t) const;

  %extend UContext
  {
    void send(bytetype buf, size_t size)
    {
      self->send(buf, size);
    }

    void yield_for(long long delay)
    {
      self->yield_for(delay);
    }

    void yield_until(long long delay)
    {
      self->yield_until(delay);
    }
  }
}

%include "urbi/ucontext.hh"


////////////////////////////
///                      ///
///       UEvent         ///
///                      ///
////////////////////////////

namespace urbi
{
  %ignore UEvent::get_name() const;

  %typemap(javacode) UEvent
  %{
    public void emit(UValue v1, UValue v2, UValue v3, UValue v4, UValue v5, UValue v6, UValue v7) {
      emit(new UAutoValue(v1), new UAutoValue(v2), new UAutoValue(v3),
           new UAutoValue(v4), new UAutoValue(v5), new UAutoValue(v6),
           new UAutoValue(v7));
    }
    public void emit(UValue v1, UValue v2, UValue v3, UValue v4, UValue v5, UValue v6) {
      emit(new UAutoValue(v1), new UAutoValue(v2), new UAutoValue(v3),
           new UAutoValue(v4), new UAutoValue(v5), new UAutoValue(v6));
    }
    public void emit(UValue v1, UValue v2, UValue v3, UValue v4, UValue v5) {
      emit(new UAutoValue(v1), new UAutoValue(v2), new UAutoValue(v3),
           new UAutoValue(v4), new UAutoValue(v5));
    }
    public void emit(UValue v1, UValue v2, UValue v3, UValue v4) {
      emit(new UAutoValue(v1), new UAutoValue(v2), new UAutoValue(v3),
           new UAutoValue(v4));
    }
    public void emit(UValue v1, UValue v2, UValue v3) {
      emit(new UAutoValue(v1), new UAutoValue(v2), new UAutoValue(v3));
    }
    public void emit(UValue v1, UValue v2) {
      emit(new UAutoValue(v1), new UAutoValue(v2));
    }
    public void emit(UValue v1) {
      emit(new UAutoValue(v1));
    }
  %}
}

%include "urbi/uevent.hh"



////////////////////////////
///                      ///
///        UVar          ///
///                      ///
////////////////////////////

namespace urbi
{
  %ignore UVar::blend;
  %ignore UVar::constant;
  %ignore UVar::delta;
  %ignore UVar::get_local() const;
  %ignore UVar::get_name;
  %ignore UVar::get_rtp() const;
  %ignore UVar::get_temp() const;
  %ignore UVar::in;
  %ignore UVar::name;
  %ignore UVar::out;
  %ignore UVar::rangemax;
  %ignore UVar::rangemin;
  %ignore UVar::setProp(UProperty, const char *);
  %ignore UVar::set_name;
  %ignore UVar::speedmax;
  %ignore UVar::speedmin;

  %define DEFINE(Function)
    %exception UVar::Function {
    try
    {
      $action
    }
    catch (const std::runtime_error &e)
    {
      jclass clazz = jenv->FindClass("java/lang/RuntimeException");
      jenv->ThrowNew(clazz, e.what());
      return $null;
    }
  }
  %enddef

  // Catch runtime errors thrown by UVar functions (for example when
  // uvar is not binded) and rethrow as Java RuntimeExceptions
  DEFINE(operator=)
  DEFINE(setOwned)
  DEFINE(type)
  DEFINE(syncValue)
  DEFINE(val)
  DEFINE(keepSynchronized)
  DEFINE(setProp)
  DEFINE(setBypass)
  DEFINE(getProp)
  DEFINE(unnotify)
  DEFINE(operator UImage)
  DEFINE(operator UList)
  DEFINE(operator UDictionary)
  DEFINE(operator USound)
  DEFINE(operator UBinary)
  DEFINE(operator const UBinary&)
  DEFINE(operator int)
  DEFINE(operator std::string)
  DEFINE(operator ufloat)
  DEFINE(operator UBinary*)
  DEFINE(getUValue)

  %extend UVar
  {
    const UValue& getUValue()
    {
      return self->val();
    }

    std::string getName()
    {
      return self->get_name();
    }

    void setName(std::string name)
    {
      self->set_name(name);
    }

  }
}

%include "urbi/uvar.hh"



////////////////////////////
///                      ///
///      UObject         ///
///                      ///
////////////////////////////

%feature("director") urbi::UObject;

%rename("UObjectCPP") urbi::UObject;


namespace urbi
{
  // ignore 'load' attribute (SWIG generates the setter and tries to
  // do load = some_UVar which is not possible since you can't assign
  // an UVar to an UVar).
  %ignore UObject::cloner;
  %ignore UObject::gc;
  %ignore UObject::load;
  %ignore UObject::members;

  %extend UObject
  {
    /// 'load' attibutte setter
    void setLoad(UValue val)
    {
      self->load = val;
    }

    /// 'load' attibutter getter
    const UVar& getLoad()
    {
      return self->load;
    }

    void setCloner(baseURBIStarter* cloner)
    {
      self->cloner = cloner;
    }
  }
}
%include "urbi/uobject.hh"



////////////////////////////
///                      ///
///    UObjectHub        ///
///                      ///
////////////////////////////

namespace urbi
{
  // FIXME: handle UObjectList (so handle std_list)
  %ignore UObjectHub::members;
  %ignore UObjectHub::getSubClass;
}
%include "urbi/uobject-hub.hh"

////////////////////////////
///                      ///
///      UStarter        ///
///                      ///
////////////////////////////


namespace urbi
{
  %feature("director") URBIStarterJAVA;

  // Tell swig that URBIStarterJAVA is not abstract
  %feature("notabstract") URBIStarterJAVA;

  %ignore baseURBIStarter::list;
  %ignore baseURBIStarter::getFullName;
  %ignore baseURBIStarterHub::list;
}

%include "urbi/ustarter.hh"

namespace urbi
{
  class URBIStarterJAVA : public baseURBIStarter
  {
  public:
    URBIStarterJAVA(const std::string& name, bool local = false);
    virtual ~URBIStarterJAVA();

  public:

    /// To reimplement in Java subclasses:
    virtual UObject* instanciate(impl::UContextImpl* ctx,
				 const std::string& n);
  };


};

/*--------------------.
| urbi/urbi-root.hh.  |
`--------------------*/
%ignore UrbiRoot::urbi_launch(int,char **);
%include "urbi/urbi-root.hh"


/*----------------.
| urbi/umain.hh.  |
`----------------*/
%ignore urbi_main_args;
namespace urbi
{
  %ignore main(const libport::cli_args_type&, UrbiRoot&);
  %ignore main(const libport::cli_args_type&, UrbiRoot&, bool);
  %ignore main(const libport::cli_args_type&, UrbiRoot&, bool, bool);
}
%include "urbi/umain.hh"

// Local variables:
// mode: c++
// End:
