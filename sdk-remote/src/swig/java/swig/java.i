/*
 * Copyright (C) 2010-2011, Gostai S.A.S.
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
%ignore urbi::defaultClient;
%ignore urbi::default_stream;
%ignore urbi::kernelMajor;
%ignore urbi::unarmorAndSend;
%ignore urbi_main_args;
%ignore urbi::main(const libport::cli_args_type&, UrbiRoot&, bool, bool);
%ignore urbi::main(const libport::cli_args_type&, UrbiRoot&, bool);
%ignore urbi::main(const libport::cli_args_type&, UrbiRoot&);
%ignore urbi::callback(UCustomCallback, void*);
%ignore urbi::callback(UCallback);
%ignore urbi::send(const void*, size_t);

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

namespace libport
{
  /// Microseconds.
  typedef long long utime_t;
};

// Tell SWIG about size_t;
typedef unsigned int size_t;

// Java typemap
// change deafult SWIG mapping of unsigned char* return values
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
  $1 = (unsigned char *) JCALL2(GetByteArrayElements, jenv, $input, 0);
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

%{
  void setSize(urbi::UImage* b, size_t size)
  {
    b->size = size;
  }
  size_t getSize(urbi::UImage* b)
  {
    return b->size;
  }
%}

namespace urbi
{
  %extend UImage {
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

%ignore urbi::USound::dump;

namespace urbi
{
  %extend USound {
    // Place this definition of data before the usound.hh header
    // so that swig consider data as unsigned char and generate correct
    // setter to -> byte[]
    unsigned char *data;

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
  void urbi_USound_data_set(urbi::USound* b, unsigned char *data)
  {
    b->data = (char*) data;
  }
  unsigned char * urbi_USound_data_get(urbi::USound* b)
  {
    return (unsigned char *) b->data;
  }
  void setSize(urbi::USound* b, size_t size)
  {
    b->size = size;
  }
  size_t getSize(urbi::USound* b)
  {
    return b->size;
  }
%}


////////////////////////////
///                      ///
///       UBinary        ///
///                      ///
////////////////////////////

%ignore urbi::BinaryData;

/// Ignore the attribute message
/// Because swig generate a accessor named getMessage
/// which conflict with getMessage method.
/// We manually rewrite the accessor in %extend UBinary
%ignore urbi::UBinary::message;
%ignore urbi::UBinary::print;
%ignore urbi::UBinary::parse;


%include "urbi/ubinary.hh"

%{
  void urbi_UBinary_data_set(urbi::UBinary* b, unsigned char *data)
  {
    b->common.data = data;
  }

  unsigned char * urbi_UBinary_data_get(urbi::UBinary* b)
  {
    return (unsigned char *) b->common.data;
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

namespace urbi
{
  %extend UBinary
  {
    /// Accessor for the UImage
    UImage uimageValue ()
    {
      return self->image;
    }

    /// Accessor for the USound
    USound usoundValue ()
    {
      return self->sound;
    }

    size_t getSize ()
    {
      return self->common.size;
    }

    std::string getExtraHeader ()
    {
      return self->message;
    }

    void setExtraHeader (const std::string& msg)
    {
      self->message = msg;
    }

    // make swig generate getData and setData
    unsigned char* data;

    /// FIXME: we want to be able to retrieve the data in common in arrays
    /// of various type
  }
};


////////////////////////////
///                      ///
///        UList         ///
///                      ///
////////////////////////////

%ignore urbi::UList::print;

namespace urbi
{

  %define ULIST_PUSH_BACK(value_type)
    %extend UList {
      urbi::UList& push_back(value_type v)
      {
	return self->push_back(v);
      }
    }
  %enddef

  ULIST_PUSH_BACK(const urbi::UValue&)
  ULIST_PUSH_BACK(const urbi::UList&)
  ULIST_PUSH_BACK(const urbi::UBinary&)
  ULIST_PUSH_BACK(const urbi::USound&)
  ULIST_PUSH_BACK(const urbi::UImage&)
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

    std::string toString ()
    {
      return string_cast(*self);
    }

  }
}

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

%include "boost_shared_ptr.i"

namespace boost
{
  %template(FinallySharedPtr) boost::shared_ptr<libport::Finally>;
  %template(TimerHandle) boost::shared_ptr<std::string>;

  %extend shared_ptr<std::string>
  {
    static boost::shared_ptr<std::string> create(const std::string& s)
    {
      return boost::shared_ptr<std::string>(new std::string (s));
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

    std::string toString ()
    {
      return string_cast(*self);
    }
  }
}


////////////////////////////
///                      ///
///       UValue         ///
///                      ///
////////////////////////////

%ignore urbi::UValue::UValue(const void*);
%ignore urbi::UValue::UValue(const void*, bool);
%ignore urbi::UValue::UValue(long, bool);
%ignore urbi::UValue::UValue(unsigned int, bool);
%ignore urbi::UValue::UValue(unsigned long, bool);
%ignore urbi::UValue::UValue(long long, bool);
%ignore urbi::UValue::operator=(const void*);
%ignore urbi::UValue::parse;
%ignore urbi::UValue::print;
%ignore urbi::UValue::copy;

%rename("setValue") urbi::UValue::set;

%include "urbi/uvalue.hh"

namespace urbi
{

  %extend UValue
  {
    double doubleValue ()
    {
      return self->val;
    }

    void setValue (std::string s)
    {
      self->stringValue = new std::string (s);
    }

    void setValue (double d)
    {
      self->val = d;
    }

    void setValue (UBinary& b)
    {
      self->binary = new urbi::UBinary (b);
    }

    void setValue (UList& l)
    {
      self->list = new urbi::UList (l);
    }

    std::string toString ()
    {
      return string_cast(*self);
    }
  }

};


////////////////////////////
///                      ///
///       UEvent         ///
///                      ///
////////////////////////////

%typemap(javacode) urbi::UEvent %{
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

%include "urbi/uevent.hh"


////////////////////////////
///                      ///
///      UMessage        ///
///                      ///
////////////////////////////

%ignore urbi::UMessage::UMessage(UAbstractClient&, int, const char*, const char*, const binaries_type&);

/// Ignore attribute client (setter/getter conflict)
%ignore urbi::UMessage::client;
%ignore urbi::UMessage::print;

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
///   UAbstractClient    ///
///                      ///
////////////////////////////

%ignore urbi::UAbstractClient::vpack;
%ignore urbi::UAbstractClient::sendBinary;
%ignore urbi::UAbstractClient::sendBin(const void*, size_t);
%ignore urbi::UAbstractClient::sendBin(const void*, size_t, const char*, ...);
%ignore urbi::UAbstractClient::stream_get;
%ignore urbi::UAbstractClient::send(std::istream&);
%ignore urbi::UAbstractClient::sendCommand(UCallback, const char*, ...);
%ignore urbi::UAbstractClient::sendCommand(UCustomCallback, void *, const char*, ...);
%ignore urbi::UAbstractClient::putFile(const void*, size_t, const char*);
%ignore urbi::UAbstractClient::setCallback(UCallback, const char*);
%ignore urbi::UAbstractClient::setCallback(UCustomCallback, void*, const char*);

%ignore urbi::UCallbackWrapperF;
%ignore urbi::UCallbackWrapperCF;

%include "urbi/uabstractclient.hh"

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
///       UClient        ///
///                      ///
////////////////////////////

// FIXME: handle options

%ignore urbi::UClient::UClient(const std::string&, unsigned, size_t, const UClient::options&);
%ignore urbi::UClient::send(std::istream&);
%ignore urbi::UClient::pong;

/// Generate code for UClient:
%include "urbi/uclient.hh"


////////////////////////////
///                      ///
///     USyncClient      ///
///                      ///
////////////////////////////

%ignore urbi::USyncClient::USyncClient(const std::string&, unsigned, size_t, const USyncClient::options&);
%ignore urbi::USyncClient::syncGetDevice(const char*, double &, libport::utime_t);
%ignore urbi::USyncClient::syncGetResult(const char*, double &, libport::utime_t);
%ignore urbi::USyncClient::syncGetNormalizedDevice(const char*, double &, libport::utime_t);
%ignore urbi::USyncClient::syncGetDevice(const char*, const char*, double &, libport::utime_t);
%ignore urbi::USyncClient::syncGetDevice(const char*, double &);
%ignore urbi::USyncClient::syncGetResult(const char*, double &);
%ignore urbi::USyncClient::syncGetNormalizedDevice(const char*, double &);
%ignore urbi::USyncClient::syncGetDevice(const char*, const char*, double &);
%ignore urbi::USyncClient::syncSend(const void*, size_t);
%ignore urbi::USyncClient::syncGetImage(const char*, void*, size_t&, int, int, size_t&, size_t&, libport::utime_t);
%ignore urbi::USyncClient::syncGetImage(const char*, void*, size_t&, int, int, size_t&, size_t&);
%ignore urbi::USyncClient::setDefaultOptions;
%ignore urbi::USyncClient::getOptions;
%ignore urbi::USyncClient::listen;

// FIXME: handle options (and setDefaultOptions and getOptions)
// FIXME: handle listen ?

/// Generate code for UClient:
%include "urbi/usyncclient.hh"


namespace urbi
{
  %extend USyncClient
  {

  }
};

%ignore urbi::convertJPEGtoRGB;
%ignore urbi::convertJPEGtoYCrCb;
%ignore urbi::convertRGBtoGrey8_601;
%ignore urbi::convertRGBtoJPEG;
%ignore urbi::convertRGBtoYCbCr;
%ignore urbi::convertYCbCrtoRGB;
%ignore urbi::convertYCrCbtoJPEG;
%ignore urbi::convertYCrCbtoYCbCr;

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

%ignore urbi::impl::UVarImpl::timestamp;

%ignore urbi::impl::UContextImpl::getIoService;
%ignore urbi::impl::UContextImpl::send(const void*, size_t);
%ignore urbi::impl::UContextImpl::setTimer;
%ignore urbi::impl::UContextImpl::getGenericCallbackImpl;
%ignore urbi::impl::UContextImpl::hubs;
%ignore urbi::impl::UContextImpl::objects;
%ignore urbi::impl::UContextImpl::initialized;

%include "urbi/ucontext-impl.hh"


////////////////////////////
///                      ///
///      UContext        ///
///                      ///
////////////////////////////


%ignore urbi::UContext::yield_for(libport::utime_t) const;
%ignore urbi::UContext::yield_until(libport::utime_t) const;
%ignore urbi::UContext::send(const void*, size_t);

%include "urbi/ucontext.hh"

namespace urbi
{
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

////////////////////////////
///                      ///
///        UVar          ///
///                      ///
////////////////////////////

%ignore urbi::UVar::name;
%ignore urbi::UVar::set_name;
%ignore urbi::UVar::get_name;
%ignore urbi::UVar::rangemin;
%ignore urbi::UVar::rangemax;
%ignore urbi::UVar::speedmin;
%ignore urbi::UVar::speedmax;
%ignore urbi::UVar::delta;
%ignore urbi::UVar::blend;
%ignore urbi::UVar::constant;
%ignore urbi::UVar::in;
%ignore urbi::UVar::out;

%define CPP_RUNTIME_ERROR_CATCH(function)
%exception function {
  try {
     $action
  }
  catch (std::runtime_error &e) {
    jclass clazz = jenv->FindClass("java/lang/RuntimeException");
    jenv->ThrowNew(clazz, e.what());
    return $null;
  }
}
%enddef

// Catch runtime errors thrown by UVar functions (for example when
// uvar is not binded) and rethrow as Java RuntimeExceptions
CPP_RUNTIME_ERROR_CATCH(urbi::UVar::operator=)
CPP_RUNTIME_ERROR_CATCH(urbi::UVar::setOwned)
CPP_RUNTIME_ERROR_CATCH(urbi::UVar::type)
CPP_RUNTIME_ERROR_CATCH(urbi::UVar::syncValue)
CPP_RUNTIME_ERROR_CATCH(urbi::UVar::val)
CPP_RUNTIME_ERROR_CATCH(urbi::UVar::keepSynchronized)
CPP_RUNTIME_ERROR_CATCH(urbi::UVar::setProp)
CPP_RUNTIME_ERROR_CATCH(urbi::UVar::setBypass)
CPP_RUNTIME_ERROR_CATCH(urbi::UVar::getProp)
CPP_RUNTIME_ERROR_CATCH(urbi::UVar::unnotify)
CPP_RUNTIME_ERROR_CATCH(urbi::UVar::operator UImage)
CPP_RUNTIME_ERROR_CATCH(urbi::UVar::operator UList)
CPP_RUNTIME_ERROR_CATCH(urbi::UVar::operator UDictionary)
CPP_RUNTIME_ERROR_CATCH(urbi::UVar::operator USound)
CPP_RUNTIME_ERROR_CATCH(urbi::UVar::operator UBinary)
CPP_RUNTIME_ERROR_CATCH(urbi::UVar::operator const UBinary&)
CPP_RUNTIME_ERROR_CATCH(urbi::UVar::operator int)
CPP_RUNTIME_ERROR_CATCH(urbi::UVar::operator std::string)
CPP_RUNTIME_ERROR_CATCH(urbi::UVar::operator ufloat)
CPP_RUNTIME_ERROR_CATCH(urbi::UVar::operator UBinary*)
CPP_RUNTIME_ERROR_CATCH(urbi::UVar::getUValue)

%include "urbi/uvar.hh"

namespace urbi
{
  %extend UVar
  {
    const UValue& getUValue ()
    {
      return self->val ();
    }

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


////////////////////////////
///                      ///
///      UObject         ///
///                      ///
////////////////////////////

%feature("director") urbi::UObject;

%rename("UObjectCPP") urbi::UObject;


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

////////////////////////////
///                      ///
///    UObjectHub        ///
///                      ///
////////////////////////////

// FIXME: handle UObjectList (so handle std_list)
%ignore urbi::UObjectHub::members;
%ignore urbi::UObjectHub::getSubClass;

%include "urbi/uobject-hub.hh"

////////////////////////////
///                      ///
///      UStarter        ///
///                      ///
////////////////////////////


%feature("director") urbi::URBIStarterJAVA;

/// Tell swig that urbi::URBIStarterJAVA is not abstract
%feature("notabstract") urbi::URBIStarterJAVA;

%ignore urbi::baseURBIStarter::list;
%ignore urbi::baseURBIStarter::getFullName;
%ignore urbi::baseURBIStarterHub::list;

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

// Local variables:
// mode: c++
// End:
