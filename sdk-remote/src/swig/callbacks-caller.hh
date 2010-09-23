/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef CALLBACKS_CALLER_HH_
# define CALLBACKS_CALLER_HH_

#include <jni.h>
#include "call-macros.hh"
#include "urbi/uvar.hh"
#include "urbi/ucallbacks.hh"
#include "urbi/utimer-callback.hh"


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

  CallbacksCaller ();

  void			setObject (jobject o);
  void			setMethodID (jmethodID id);

public:

  /// Function callbacks
  /// below are macros. The the file call-macros.hh.
  /// NB: long long is not handled in Urbi. jlong is 'long long'.
  /// We cast it to 'long'.
  CALL_METHODS (UValue, Object, urbi::UValue, urbi::UValue (), jobject res = , return getUValueFromObject(res););
  CALL_METHODS (UBinary, Object, urbi::UBinary, urbi::UBinary (), jobject res = , return getUBinaryFromObject(res););
  CALL_METHODS (UList, Object, urbi::UList, urbi::UList (), jobject res = , return getUListFromObject(res););
  CALL_METHODS (USound, Object, urbi::USound, urbi::USound (), jobject res = , return getUSoundFromObject(res););
  CALL_METHODS (UImage, Object, urbi::UImage, urbi::UImage (), jobject res = , return getUImageFromObject(res););
  CALL_METHODS (UDictionary, Object, urbi::UDictionary, urbi::UDictionary (), jobject res = , return getUDictionaryFromObject(res););
  CALL_METHODS (String, Object, std::string, std::string (), jstring res = (jstring), return getStringFromJString(res););
  CALL_METHODS (Void, Void, void, , ,);
  CALL_METHODS (Boolean, Boolean, jboolean, 0, jboolean res = , return res;);
  CALL_METHODS (Byte, Byte, jbyte, 0, jbyte res = , return res;);
  CALL_METHODS (Char, Char, jchar, 0, jchar res = , return res;);
  CALL_METHODS (Short, Short, jshort, 0, jshort res = , return res;);
  CALL_METHODS (Int, Int, jint, 0, jint res = , return res;);
  CALL_METHODS (Long, Long, long, 0, long res = , return res;);
  CALL_METHODS (Float, Float, jfloat, 0, jfloat res = , return res;);
  CALL_METHODS (Double, Double, jdouble, 0, jdouble res = , return res;);


  /// UNotifyChange callbacks
  int			callNotifyChangeInt_0 ();
  int			callNotifyChangeInt_1 (urbi::UVar& v);
  void			callNotifyChangeVoid_0 ();
  void			callNotifyChangeVoid_1 (urbi::UVar& v);


public:

  static bool		areJNIVariablesCached ();

  /// Initialize all the Jni variable we cache to speed up the execution
  /// of the callbacks.
  static bool		cacheJNIVariables (JNIEnv* env);

private:

  /// Initialise the Java environment variable. Indeed, the JNIEnv pointer
  /// cannot be shared between threads, and the callbacks are triggered by
  /// a thread different from the one where the callbacks were registered.
  /// So the first time one of the callbacks is called, we search for the
  /// created javaVM, and we retrieve the jnienv pointer.
  bool			init_env ();

  /// Return a global reference to the class with name 'classname'
  static jclass		getGlobalRef (JNIEnv* env, const char* classname);


  /// Private conversion functions. These function are used to convert from
  /// Java object (jobject) to C++ real object type. They are specific for
  /// UValue and UVar.
  jobject		getObjectFromUValue (const urbi::UValue& v);
  jobject		getObjectFromUBinary (const urbi::UBinary& v);
  jobject		getObjectFromUList (const urbi::UList& v);
  jobject		getObjectFromUSound (const urbi::USound& v);
  jobject		getObjectFromUImage (const urbi::UImage& v);
  jobject		getObjectFromUDictionary (const urbi::UDictionary& v);
  jobject		getObjectFromString (const std::string& v);
  jobject		getObjectFromInteger (int v);
  jobject               getObjectFromDouble (double val);
  jobject               getObjectFromFloat (ufloat val);
  jobject               getObjectFromLong (long val);
  jobject               getObjectFromShort (int val);
  jobject               getObjectFromCharacter (int val);
  jobject               getObjectFromByte (int val);
  jobject               getObjectFromBoolean (bool val);
  jobject		getObjectFromUVar (urbi::UVar& v);
  jvalue		getObjectFrom (const std::string& type_name, urbi::UValue v);

  void			testForException();

  static void	       	deleteClassRefs(JNIEnv* env);

public:

  urbi::UValue	getUValueFromObject (jobject obj);
  urbi::UBinary	getUBinaryFromObject (jobject obj);
  urbi::UImage	getUImageFromObject (jobject obj);
  urbi::USound	getUSoundFromObject (jobject obj);
  urbi::UDictionary	getUDictionaryFromObject (jobject obj);
  urbi::UList	getUListFromObject (jobject obj);
  urbi::UVar*	getUVarFromObject (jobject obj);
  std::string	getStringFromJString (jstring obj);
  static urbi::UObject*	getUObjectFromObject (jobject obj, JNIEnv* env);

public:
  std::vector<std::string> arg_types;

private:

  /// The java object containing the Java callback method
  jobject		obj;

  /// The id of the java callback method. This method is then called in the
  /// callbacks contained in this class.
  jmethodID		mid;

private:

  JavaVM* jvm;

  /// Java interface pointer. Initialised the first time one of the calllbacks
  /// is triggered.
  JNIEnv*	env_;

  /// All theses variables below are used in the conversion functions to
  /// convert from jobject to UVar or UValue or UObject.
  /// We cache these variable because it is costly to retrieve them.
  static jclass		double_cls;
  static jmethodID	double_valueof_id;
  static jclass		float_cls;
  static jmethodID	float_valueof_id;
  static jclass		long_cls;
  static jmethodID	long_valueof_id;
  static jclass		short_cls;
  static jmethodID	short_valueof_id;
  static jclass		character_cls;
  static jmethodID	character_valueof_id;
  static jclass		byte_cls;
  static jmethodID	byte_valueof_id;
  static jclass		boolean_cls;
  static jmethodID	boolean_valueof_id;
  static jclass		integer_cls;
  static jmethodID	integer_valueof_id;
  static jclass		class_cls;
  static jmethodID	class_getname_id;
  static jclass		string_cls;
  static jmethodID	string_ctor_id;
  static jclass		ulist_cls;
  static jmethodID	ulist_ctor_id;
  static jfieldID	ulist_swigptr_id;
  static jclass		ubinary_cls;
  static jmethodID	ubinary_ctor_id;
  static jfieldID	ubinary_swigptr_id;
  static jclass		uimage_cls;
  static jmethodID	uimage_ctor_id;
  static jfieldID	uimage_swigptr_id;
  static jclass		usound_cls;
  static jmethodID	usound_ctor_id;
  static jfieldID	usound_swigptr_id;
  static jclass		udictionary_cls;
  static jmethodID	udictionary_ctor_id;
  static jfieldID	udictionary_swigptr_id;
  static jclass		uvar_cls;
  static jmethodID	uvar_ctor_id;
  static jfieldID	uvar_swigptr_id;
  static jclass		uvalue_cls;
  static jmethodID	uvalue_ctor_id;
  static jfieldID	uvalue_swigptr_id;
  static jclass		uobject_cls;
  static jfieldID	uobject_swigptr_id;

  /// This variable equals true when the cached variable have been
  /// initialised correctly.
  static bool		jni_variables_cached_;
};




/// ------------------------------------ ///
///                                      ///
/// Support for throwing Java exceptions ///
///                                      ///
/// ------------------------------------ ///

typedef enum {
  SWIG_JavaOutOfMemoryError = 1,
  SWIG_JavaIOException,
  SWIG_JavaRuntimeException,
  SWIG_JavaIndexOutOfBoundsException,
  SWIG_JavaArithmeticException,
  SWIG_JavaIllegalArgumentException,
  SWIG_JavaNullPointerException,
  SWIG_JavaDirectorPureVirtual,
  SWIG_JavaUnknownError
} SWIG_JavaExceptionCodes;

typedef struct {
  SWIG_JavaExceptionCodes code;
  const char *java_exception;
} SWIG_JavaExceptions_t;

void SWIG_JavaThrowException(JNIEnv *jenv,
			     SWIG_JavaExceptionCodes code,
			     const char *msg);

# define TROW_RUNTIME(env, msg)					\
  throw std::runtime_error(msg);
//  SWIG_JavaThrowException(env, SWIG_JavaRuntimeException, msg)

#endif
