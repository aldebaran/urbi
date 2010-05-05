/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include "callbacks-caller.hh"


jmethodID	CallbacksCaller::uvalue_ctor_id = 0;
jfieldID	CallbacksCaller::uvalue_swigptr_id = 0;
jclass		CallbacksCaller::uvalue_cls = 0;
jfieldID	CallbacksCaller::uvar_swigptr_id = 0;
jmethodID	CallbacksCaller::uvar_ctor_id = 0;
jclass		CallbacksCaller::uvar_cls = 0;
jfieldID	CallbacksCaller::uobject_swigptr_id = 0;
jclass		CallbacksCaller::uobject_cls = 0;
bool		CallbacksCaller::jni_variables_cached_ = false;


CallbacksCaller::CallbacksCaller ()
  : mid (0),
    obj (0),
    jvm (0),
    env_ (0)
{}

int
CallbacksCaller::callNotifyChange_0 ()
{
  if (!init_env ())
    return 0;
  return env_->CallIntMethod(obj, mid);
}

int
CallbacksCaller::callNotifyChange_1 (urbi::UVar& v)
{
  if (!init_env ())
    return 0;
  jobject obj1 = getObjectFromUVar (v);
  return env_->CallIntMethod(obj, mid, obj1);
}

bool
CallbacksCaller::init_env ()
{
  if (!env_ || !jvm)
  {
    jsize size;
    JNI_GetCreatedJavaVMs(&jvm, 1, &size);
    if (!size)
    {
      std::cerr << "Error retrivieving created JavaVMs." << std::endl;
      return false;
    }
  }
  jvm->AttachCurrentThread((void**) &env_, 0);

  if (!env_)
  {
    std::cerr << "Error retrivieving JNIEnv pointer." << std::endl;
    return false;
  }
  return true;
}


void
CallbacksCaller::setObject (jobject o)
{
  obj = o;
}


void
CallbacksCaller::setMethodID (jmethodID id)
{
  mid = id;
}

bool
CallbacksCaller::areJNIVariablesCached ()
{
  return jni_variables_cached_;
}


bool
CallbacksCaller::cacheJNIVariables (JNIEnv* env)
{
  /// Get the jclass for UValue
  if (!(uvalue_cls = getGlobalRef (env, "liburbi/main/UValue")))
  {
    TROW_RUNTIME(env, "Can't find UValue class");
    return false;
  }

  /// Get UValue (int, bool) Constructor id
  if (!(uvalue_ctor_id = env->GetMethodID(uvalue_cls, "<init>", "(JZ)V")))
  {
    env->DeleteGlobalRef (uvalue_cls);
    TROW_RUNTIME(env, "Can't find UValue constructor");
    return false;
  }

  /// Get UValue swigCPtr attribute id
  if (!(uvalue_swigptr_id = env->GetFieldID(uvalue_cls, "swigCPtr", "J")))
  {
    env->DeleteGlobalRef (uvalue_cls);
    TROW_RUNTIME(env, "Can't find UValue swigCPtr");
    return false;
  }

  /// Get the jclass for UVar
  if (!(uvar_cls = getGlobalRef (env, "liburbi/main/UVar")))
  {
    env->DeleteGlobalRef (uvalue_cls);
    TROW_RUNTIME(env, "Can't find UVar class");
    return false;
  }

  if (!(uvar_ctor_id = env->GetMethodID(uvar_cls, "<init>", "(JZ)V")))
  {
    env->DeleteGlobalRef (uvalue_cls);
    env->DeleteGlobalRef (uvar_cls);
    TROW_RUNTIME(env, "Can't find UVar constructor");
    return false;
  }

  /// Get UValue swigCPtr attribute id
  if (!(uvar_swigptr_id = env->GetFieldID(uvar_cls, "swigCPtr", "J")))
  {
    env->DeleteGlobalRef (uvalue_cls);
    env->DeleteGlobalRef (uvar_cls);
    TROW_RUNTIME(env, "Can't find UVar swigCPtr");
    return false;
  }

  /// Get the jclass for UObject
  if (!(uobject_cls = getGlobalRef(env, "liburbi/main/UObject")))
  {
    env->DeleteGlobalRef (uvalue_cls);
    env->DeleteGlobalRef (uvar_cls);
    TROW_RUNTIME(env, "Can't find UObject class");
    return false;
  }

  /// Get UObject swigCPtr attribute id
  if (!(uobject_swigptr_id = env->GetFieldID(uobject_cls, "swigCPtr", "J")))
  {
    env->DeleteGlobalRef (uvalue_cls);
    env->DeleteGlobalRef (uvar_cls);
    env->DeleteGlobalRef (uobject_cls);
    TROW_RUNTIME(env, "Can't find UObject swigCPtr");
    return false;
  }

  jni_variables_cached_ = true;
  return true; /// all went OK
}

jclass
CallbacksCaller::getGlobalRef (JNIEnv* env, const char* classname)
{
  /// Get the jclass for UValue
  jclass tmp, res;
  if (!(tmp = env->FindClass(classname)))
  {
    std::string msg = "Can't find class ";
    msg += classname;
    TROW_RUNTIME (env, msg.c_str ());
    return false;
  }

  if (!(res = (jclass) env->NewGlobalRef(tmp)))
  {
    std::string msg = "Can't create Global Ref for class  ";
    msg += classname;
    TROW_RUNTIME (env, msg.c_str ());
    return false;
  }

  env->DeleteLocalRef(tmp);
  return res;
}

urbi::UObject*
CallbacksCaller::getUObjectFromObject (jobject obj, JNIEnv* env)
{
  if (obj)
  {
    jlong ptr = env->GetLongField(obj, uobject_swigptr_id);
    if (ptr)  /// Java alocated memory, prefer allocate mine
      return (urbi::UObject*) ptr;
    else
      return 0;
  }
  else
    return 0;
}

urbi::UVar*
CallbacksCaller::getUVarFromObject(jobject obj)
{
  if (obj)
  {
    jlong ptr = env_->GetLongField(obj, uobject_swigptr_id);
    if (ptr)  /// Java alocated memory, prefer allocate mine
      return (urbi::UVar*) ptr;
    else
      return 0;
  }
  else
    return 0;
}

urbi::UValue
CallbacksCaller::getUValueFromObject (jobject obj)
{
  if (obj)
  {
    jlong ptr = env_->GetLongField(obj, uvalue_swigptr_id);
    if (ptr)  /// Java alocated memory, prefer allocate mine
      return *(urbi::UValue*) ptr;
    else
      return urbi::UValue ();
  }
  else
    return urbi::UValue ();
}

jobject
CallbacksCaller::getObjectFromUValue (const urbi::UValue& v)
{
  jobject res = env_->NewObject(uvalue_cls, uvalue_ctor_id, (jlong) &v, false);
  if (!res)
    std::cerr << "Cannot allocate a new object of type liburbi.main.UValue"
	      << std::endl;
  return res;
}

jobject
CallbacksCaller::getObjectFromUVar (urbi::UVar& v)
{
  jobject res = env_->NewObject(uvar_cls, uvar_ctor_id, (jlong) &v, false);
  if (!res)
    std::cerr << "Cannot allocate a new object of type liburbi.main.UVar"
	      << std::endl;
  return res;
}


void
SWIG_JavaThrowException(JNIEnv *jenv,
			SWIG_JavaExceptionCodes code,
			const char *msg)
{
  jclass excep;
  static const SWIG_JavaExceptions_t java_exceptions[] = {
    { SWIG_JavaOutOfMemoryError, "java/lang/OutOfMemoryError" },
    { SWIG_JavaIOException, "java/io/IOException" },
    { SWIG_JavaRuntimeException, "java/lang/RuntimeException" },
    { SWIG_JavaIndexOutOfBoundsException, "java/lang/IndexOutOfBoundsException" },
    { SWIG_JavaArithmeticException, "java/lang/ArithmeticException" },
    { SWIG_JavaIllegalArgumentException, "java/lang/IllegalArgumentException" },
    { SWIG_JavaNullPointerException, "java/lang/NullPointerException" },
    { SWIG_JavaDirectorPureVirtual, "java/lang/RuntimeException" },
    { SWIG_JavaUnknownError,  "java/lang/UnknownError" },
    { (SWIG_JavaExceptionCodes)0,  "java/lang/UnknownError" } };
  const SWIG_JavaExceptions_t *except_ptr = java_exceptions;

  while (except_ptr->code != code && except_ptr->code)
    except_ptr++;

  jenv->ExceptionClear();
  excep = jenv->FindClass(except_ptr->java_exception);

  if (excep)
    jenv->ThrowNew(excep, msg);
}
