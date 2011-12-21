/*
 * Copyright (C) 2010, 2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef CALLBACKS_CALLER_HH_
# define CALLBACKS_CALLER_HH_

# include <boost/function.hpp>
# include <libport/bind.hh>
# include <jni.h>
# include "converter.hh"
# include "call-macros.hh"
# include <urbi/uvar.hh>
# include <urbi/ucallbacks.hh>
# include <urbi/utimer-callback.hh>

typedef boost::function<jvalue(JNIEnv*, urbi::UValue)> ConversionFunc;

/// This class allow to register Java callbacks in C++.
///
/// This class define C++ callbacks, that can be registered in C++
/// in the class UGenericCallbacks (see liburbi C++ sources).
/// This class also store a java method id and an object. When the
/// callbacks in this class are triggered, they trigger in turn the
/// associated callbacks in Java.
class CallbacksCaller
{
public:
  CallbacksCaller();

  void setObject(jobject o);
  void setMethodID(jmethodID id);

public:
  /// Function callbacks
  /// below are macros. The the file call-macros.hh.
  /// NB: long long is not handled in Urbi. jlong is 'long long'.
  /// We cast it to 'long'.
  CALL_METHODS(UValue, Object, urbi::UValue, urbi::UValue(), jobject res = , jobject ret_no_pop = res; const urbi::UValue& r = getUValueFromObject(res);, return r);
  CALL_METHODS(UBinary, Object, urbi::UBinary, urbi::UBinary(), jobject res = , jobject ret_no_pop = res; const urbi::UBinary& r = getUBinaryFromObject(res);, return r);
  CALL_METHODS(UList, Object, urbi::UList, urbi::UList(), jobject res = , jobject ret_no_pop = res; const urbi::UList& r = getUListFromObject(res);, return r);
  CALL_METHODS(USound, Object, urbi::USound, urbi::USound(), jobject res = , jobject ret_no_pop = res; const urbi::USound& r = getUSoundFromObject(res);, return r);
  CALL_METHODS(UImage, Object, urbi::UImage, urbi::UImage(), jobject res = , jobject ret_no_pop = res; const urbi::UImage& r = getUImageFromObject(res);, return r);
  // FIXME: making r a const-ref should be enough, but it seems to
  // trigger a bug between Apple G++ 4.2 and Boost 1.48.
  // (https://svn.boost.org/trac/boost/ticket/6311).
  CALL_METHODS(UDictionary, Object, urbi::UDictionary, urbi::UDictionary(), jobject res = , jobject ret_no_pop = res; urbi::UDictionary r = getUDictionaryFromObject(res); , return r);
  CALL_METHODS(String, Object, std::string, std::string(), jstring res =(jstring), jobject ret_no_pop = res; const std::string& r = getStringFromJString(res);, return r);
  CALL_METHODS(Void, Void, void, , , jobject ret_no_pop = NULL;,);
  CALL_METHODS(Boolean, Boolean, jboolean, 0, jboolean res = , jobject ret_no_pop = NULL;, return res;);
  CALL_METHODS(Byte, Byte, jbyte, 0, jbyte res = , jobject ret_no_pop = NULL;, return res;);
  CALL_METHODS(Char, Char, jchar, 0, jchar res = , jobject ret_no_pop = NULL;, return res;);
  CALL_METHODS(Short, Short, jshort, 0, jshort res = , jobject ret_no_pop = NULL;, return res;);
  CALL_METHODS(Int, Int, jint, 0, jint res = , jobject ret_no_pop = NULL;, return res;);
  CALL_METHODS(Long, Long, jlong, 0, jlong res = , jobject ret_no_pop = NULL;, return res;);
  CALL_METHODS(Float, Float, jfloat, 0, jfloat res = , jobject ret_no_pop = NULL;, return res;);
  CALL_METHODS(Double, Double, jdouble, 0, jdouble res = , jobject ret_no_pop = NULL;, return res;);


  /// UNotifyChange callbacks
  int callNotifyChangeInt_0();
  int callNotifyChangeInt_1(urbi::UVar& v);
  void callNotifyChangeVoid_0();
  void callNotifyChangeVoid_1(urbi::UVar& v);


public:
  static bool areJNIVariablesCached();

  /// Initialize all the Jni variable we cache to speed up the execution
  /// of the callbacks.
  static bool cacheJNIVariables(JNIEnv* env);

private:

  /// Initialise the Java environment variable. Indeed, the JNIEnv pointer
  /// cannot be shared between threads, and the callbacks are triggered by
  /// a thread different from the one where the callbacks were registered.
  /// So the first time one of the callbacks is called, we search for the
  /// created javaVM, and we retrieve the jnienv pointer.
  bool init_env();

  /// Return a global reference to the class with name 'classname'
  static jclass getGlobalRef(JNIEnv* env, const char* classname);

  void testForException();

public:

  /// Private conversion functions. These function are used to convert from
  /// Java object(jobject) to C++ real object type.
  urbi::UValue getUValueFromObject(jobject obj);
  urbi::UBinary getUBinaryFromObject(jobject obj);
  urbi::UImage getUImageFromObject(jobject obj);
  urbi::USound getUSoundFromObject(jobject obj);
  urbi::UDictionary getUDictionaryFromObject(jobject obj);
  urbi::UList getUListFromObject(jobject obj);
  urbi::UVar* getUVarFromObject(jobject obj);
  std::string getStringFromJString(jstring obj);
  static urbi::UObject* getUObjectFromObject(jobject obj, JNIEnv* env);

public:

  std::vector<Converter*> arg_convert;

private:

  /// The java object containing the Java callback method
  jobject obj;

  /// The id of the java callback method. This method is then called in the
  /// callbacks contained in this class.
  jmethodID mid;

private:

  JavaVM* jvm;

  /// Java interface pointer. Initialised the first time one of the calllbacks
  /// is triggered.
  JNIEnv* env_;

  /// All theses variables below are used in the conversion functions to
  /// convert from jobject to UVar or UValue or UObject.
  /// We cache these variable because it is costly to retrieve them.
  static jfieldID ulist_swigptr_id;
  static jfieldID ubinary_swigptr_id;
  static jfieldID uimage_swigptr_id;
  static jfieldID usound_swigptr_id;
  static jfieldID udictionary_swigptr_id;
  static jfieldID uvar_swigptr_id;
  static jfieldID uvalue_swigptr_id;
  static jclass uobject_cls;
  static jfieldID uobject_swigptr_id;

  static jclass class_cls;
  static jmethodID class_getname_id;

  /// This variable equals true when the cached variable have been
  /// initialised correctly.
  static bool jni_variables_cached_;
};




/// ------------------------------------ ///
///                                      ///
/// Support for throwing Java exceptions ///
///                                      ///
/// ------------------------------------ ///

enum SWIG_JavaExceptionCodes
{
  SWIG_JavaOutOfMemoryError = 1,
  SWIG_JavaIOException,
  SWIG_JavaRuntimeException,
  SWIG_JavaIndexOutOfBoundsException,
  SWIG_JavaArithmeticException,
  SWIG_JavaIllegalArgumentException,
  SWIG_JavaNullPointerException,
  SWIG_JavaDirectorPureVirtual,
  SWIG_JavaUnknownError
};

typedef struct
{
  SWIG_JavaExceptionCodes code;
  const char *java_exception;
} SWIG_JavaExceptions_t;

void
SWIG_JavaThrowException(JNIEnv *jenv,
                        SWIG_JavaExceptionCodes code,
                        const char *msg);

#endif
