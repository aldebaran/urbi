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
  CALL_METHODS (Void, Void, void, , ,);
  CALL_METHODS (Boolean, Boolean, jboolean, 0, return,);
  CALL_METHODS (Byte, Byte, jbyte, 0, return,);
  CALL_METHODS (Char, Char, jchar, 0, return,);
  CALL_METHODS (Short, Short, jshort, 0, return,);
  CALL_METHODS (Int, Int, jint, 0, return,);
  CALL_METHODS (Long, Long, long, 0, return,);
  CALL_METHODS (Float, Float, jfloat, 0, return,);
  CALL_METHODS (Double, Double, jdouble, 0, return,);


  /// UNotifyChange callbacks
  int			callNotifyChange_0 ();
  int			callNotifyChange_1 (urbi::UVar& v);

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
  jobject		getObjectFromUVar (urbi::UVar& v);

public:

  urbi::UValue	getUValueFromObject (jobject obj);
  urbi::UVar*	getUVarFromObject (jobject obj);
  static urbi::UObject*	getUObjectFromObject (jobject obj, JNIEnv* env);

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
  SWIG_JavaThrowException(env, SWIG_JavaRuntimeException, msg)

#endif
