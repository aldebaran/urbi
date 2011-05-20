/*
 * Copyright (C) 2010, 2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/cmath>
#include <libport/format.hh>
#include "callbacks-caller.hh"

jfieldID CallbacksCaller::ulist_swigptr_id = 0;
jfieldID CallbacksCaller::uimage_swigptr_id = 0;
jfieldID CallbacksCaller::usound_swigptr_id = 0;
jfieldID CallbacksCaller::udictionary_swigptr_id = 0;
jfieldID CallbacksCaller::ubinary_swigptr_id = 0;
jfieldID CallbacksCaller::uvalue_swigptr_id = 0;
jfieldID CallbacksCaller::uvar_swigptr_id = 0;
jclass CallbacksCaller::uobject_cls = 0;
jfieldID CallbacksCaller::uobject_swigptr_id = 0;
jmethodID CallbacksCaller::class_getname_id = 0;
jclass CallbacksCaller::class_cls = 0;
bool CallbacksCaller::jni_variables_cached_ = false;

CallbacksCaller::CallbacksCaller()
  : mid(0)
  , obj(0)
  , jvm(0)
  , env_(0)
{}

int
CallbacksCaller::callNotifyChangeInt_0()
{
  if (!init_env())
    return 0;
  int res = env_->CallIntMethod(obj, mid);
  testForException();
  return res;
}

int
CallbacksCaller::callNotifyChangeInt_1(urbi::UVar& v)
{
  if (!init_env())
    return 0;
  jvalue obj1 = arg_convert[0]->convert(env_, v);
  jvalue arg[] = { obj1 };
  int res = env_->CallIntMethodA(obj, mid, arg);
  testForException();
  return res;
}

void
CallbacksCaller::callNotifyChangeVoid_0()
{
  if (!init_env())
    return;
  env_->CallVoidMethod(obj, mid);
  testForException();
}

void
CallbacksCaller::callNotifyChangeVoid_1(urbi::UVar& v)
{
  if (!init_env())
    return;
  jvalue obj1 = arg_convert[0]->convert(env_, v);
  jvalue arg[] = { obj1 };
  env_->CallVoidMethodA(obj, mid, arg);
  testForException();

}


bool
CallbacksCaller::init_env()
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
CallbacksCaller::setObject(jobject o)
{
  obj = o;
}


void
CallbacksCaller::setMethodID(jmethodID id)
{
  mid = id;
}

bool
CallbacksCaller::areJNIVariablesCached()
{
  return jni_variables_cached_;
}

bool
CallbacksCaller::cacheJNIVariables(JNIEnv* env)
{

  INIT_CONVERTERS_STATIC_ATTRS(env);

#define CASE(Type, Var)                                                 \
  if (!(Var ## _swigptr_id =                                            \
        env->GetFieldID(Type ## Converter::cls, "swigCPtr", "J")))      \
    FRAISE("Can't find %s swigCPtr", #Type)

  CASE(UBinary, ubinary);
  CASE(UDictionary, udictionary);
  CASE(UImage, uimage);
  CASE(UList, ulist);
  CASE(USound, usound);
  CASE(UValue, uvalue);
  CASE(UVar, uvar);
#undef CASE

  // Get the jclass for UObject
  if (!(uobject_cls = getGlobalRef(env, "urbi/UObjectCPP")))
    FRAISE("Can't find UObject class");

  // Get UObject swigCPtr attribute id
  if (!(uobject_swigptr_id = env->GetFieldID(uobject_cls, "swigCPtr", "J")))
    FRAISE("Can't find UObject swigCPtr");

  // Get the jclass for Class
  if (!(class_cls = getGlobalRef(env, "java/lang/Class")))
    FRAISE("Can't find Class class");

  // Get String (char) Constructor id
  if (!(class_getname_id =
        env->GetMethodID(class_cls, "getName", "()Ljava/lang/String;")))
    FRAISE("Can't find Class getName function");

  jni_variables_cached_ = true;
  return true; /// all went OK
}

jclass
CallbacksCaller::getGlobalRef(JNIEnv* env, const char* classname)
{
  /// Get the jclass for UValue
  jclass tmp, res;
  if (!(tmp = env->FindClass(classname)))
    FRAISE("Can't find class %s", classname);

  if (!(res = (jclass) env->NewGlobalRef(tmp)))
    FRAISE("Can't create Global Ref for class %s", classname);

  env->DeleteLocalRef(tmp);
  return res;
}

urbi::UObject*
CallbacksCaller::getUObjectFromObject(jobject obj, JNIEnv* env)
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

#define DEFINE(Type, Where)                                             \
  urbi::Type                                                            \
  CallbacksCaller::get ## Type ##FromObject(jobject obj)                \
  {                                                                     \
    if (obj)                                                            \
      /* Java alocated memory, prefer allocate mine. */                 \
      if (jlong ptr = env_->GetLongField(obj, Where ## _swigptr_id))    \
        return *(urbi::Type*) ptr;                                      \
    return urbi::Type();                                                \
  }

DEFINE(UBinary, ubinary);
DEFINE(UDictionary, udictionary);
DEFINE(UImage, uimage);
DEFINE(UList, ulist);
DEFINE(USound, usound);
DEFINE(UValue, uvalue);
#undef DEFINE

std::string
CallbacksCaller::getStringFromJString(jstring obj)
{
  char const* str = env_->GetStringUTFChars(obj, 0);
  std::string res = str;
  env_->ReleaseStringUTFChars(obj, str);
  return res;
}


void
CallbacksCaller::testForException()
{
  jthrowable exc = env_->ExceptionOccurred();
  if (exc)
  {
    env_->ExceptionDescribe();
    env_->ExceptionClear();
    jclass java_class = env_->GetObjectClass(exc);
    assert(java_class);
    jmethodID getMessage = env_->GetMethodID(java_class,
					     "getMessage",
					     "()Ljava/lang/String;");
    assert(getMessage);
    jstring message =
      static_cast<jstring>(env_->CallObjectMethod(exc, getMessage));
    assert(message);
    char const* utfMessage = env_->GetStringUTFChars(message, 0);
    jstring name =
      static_cast<jstring>(env_->CallObjectMethod(java_class, class_getname_id));
    assert(name);
    char const* utfName = env_->GetStringUTFChars(name, 0);
    std::string m = utfMessage;
    std::string n = utfName;
    env_->ReleaseStringUTFChars(message, utfMessage);
    env_->ReleaseStringUTFChars(name, utfName);
    FRAISE("%s: %s", n, m);
  }
};



void
SWIG_JavaThrowException(JNIEnv *jenv,
			SWIG_JavaExceptionCodes code,
			const char *msg)
{
  jclass excep;
  static const SWIG_JavaExceptions_t java_exceptions[] =
  {
    { SWIG_JavaOutOfMemoryError, "java/lang/OutOfMemoryError" },
    { SWIG_JavaIOException, "java/io/IOException" },
    { SWIG_JavaRuntimeException, "java/lang/RuntimeException" },
    { SWIG_JavaIndexOutOfBoundsException, "java/lang/IndexOutOfBoundsException" },
    { SWIG_JavaArithmeticException, "java/lang/ArithmeticException" },
    { SWIG_JavaIllegalArgumentException, "java/lang/IllegalArgumentException" },
    { SWIG_JavaNullPointerException, "java/lang/NullPointerException" },
    { SWIG_JavaDirectorPureVirtual, "java/lang/RuntimeException" },
    { SWIG_JavaUnknownError,  "java/lang/UnknownError" },
    { (SWIG_JavaExceptionCodes)0,  "java/lang/UnknownError" }
  };

  const SWIG_JavaExceptions_t *except_ptr = java_exceptions;
  while (except_ptr->code != code && except_ptr->code)
    except_ptr++;

  jenv->ExceptionClear();
  excep = jenv->FindClass(except_ptr->java_exception);

  if (excep)
    jenv->ThrowNew(excep, msg);
}
