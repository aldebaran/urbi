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

#define CLEAN_AND_THROW(msg)			\
  do						\
  {						\
    THROW_RUNTIME(env, msg);			\
    return false;				\
  } while (false)


  /// Get UValue swigCPtr attribute id
  if (!(uvalue_swigptr_id = env->GetFieldID(UValueConverter::cls, "swigCPtr", "J")))
    CLEAN_AND_THROW("Can't find UValue swigCPtr");

  /// Get UList swigCPtr attribute id
  if (!(ulist_swigptr_id = env->GetFieldID(UListConverter::cls, "swigCPtr", "J")))
    CLEAN_AND_THROW("Can't find UList swigCPtr");

  /// Get UImage swigCPtr attribute id
  if (!(uimage_swigptr_id = env->GetFieldID(UImageConverter::cls, "swigCPtr", "J")))
    CLEAN_AND_THROW("Can't find UImage swigCPtr");

  /// Get UBinary swigCPtr attribute id
  if (!(ubinary_swigptr_id = env->GetFieldID(UBinaryConverter::cls, "swigCPtr", "J")))
    CLEAN_AND_THROW("Can't find UBinary swigCPtr");

  /// Get Usound swigCPtr attribute id
  if (!(usound_swigptr_id = env->GetFieldID(USoundConverter::cls, "swigCPtr", "J")))
    CLEAN_AND_THROW("Can't find USound swigCPtr");

  /// Get Udictionary swigCPtr attribute id
  if (!(udictionary_swigptr_id = env->GetFieldID(UDictionaryConverter::cls, "swigCPtr", "J")))
    CLEAN_AND_THROW("Can't find UDictionary swigCPtr");

  /// Get UValue swigCPtr attribute id
  if (!(uvar_swigptr_id = env->GetFieldID(UVarConverter::cls, "swigCPtr", "J")))
    CLEAN_AND_THROW("Can't find UVar swigCPtr");

  /// Get the jclass for UObject
  if (!(uobject_cls = getGlobalRef(env, "urbi/UObjectCPP")))
    CLEAN_AND_THROW("Can't find UObject class");

  /// Get UObject swigCPtr attribute id
  if (!(uobject_swigptr_id = env->GetFieldID(uobject_cls, "swigCPtr", "J")))
    CLEAN_AND_THROW("Can't find UObject swigCPtr");

  /// Get the jclass for Class
  if (!(class_cls = getGlobalRef(env, "java/lang/Class")))
    CLEAN_AND_THROW("Can't find Class class");

  /// Get String (char) Constructor id
  if (!(class_getname_id = env->GetMethodID(class_cls, "getName", "()Ljava/lang/String;")))
    CLEAN_AND_THROW("Can't find Class getName function");

#undef CLEAN_AND_THROW

  jni_variables_cached_ = true;
  return true; /// all went OK
}

jclass
CallbacksCaller::getGlobalRef(JNIEnv* env, const char* classname)
{
  /// Get the jclass for UValue
  jclass tmp, res;
  if (!(tmp = env->FindClass(classname)))
  {
    THROW_RUNTIME(env, libport::format("Can't find class %s", classname));
    return false;
  }

  if (!(res = (jclass) env->NewGlobalRef(tmp)))
  {
    THROW_RUNTIME(env, libport::format("Can't create Global Ref for class %s", classname));
    return false;
  }

  env->DeleteLocalRef(tmp);
  return res;
}

urbi::UObject*
CallbacksCaller::getUObjectFromObject(jobject obj, JNIEnv* env)
{
  if (obj)
  {
    jlong ptr = env->GetLongField(obj, uobject_swigptr_id);
    if (ptr)  /// Java allocated memory, prefer allocate mine
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
    if (ptr)  /// Java allocated memory, prefer allocate mine
      return (urbi::UVar*) ptr;
    else
      return 0;
  }
  else
    return 0;
}

urbi::UValue
CallbacksCaller::getUValueFromObject(jobject obj)
{
  if (obj)
  {
    jlong ptr = env_->GetLongField(obj, uvalue_swigptr_id);
    if (ptr)  /// Java allocated memory, prefer allocate mine
      return *(urbi::UValue*) ptr;
    else
      return urbi::UValue();
  }
  else
    return urbi::UValue();
}

urbi::UDictionary
CallbacksCaller::getUDictionaryFromObject(jobject obj)
{
  if (obj)
  {
    jlong ptr = env_->GetLongField(obj, udictionary_swigptr_id);
    if (ptr)  /// Java allocated memory, prefer allocate mine
      return *(urbi::UDictionary*) ptr;
    else
      return urbi::UDictionary();
  }
  else
    return urbi::UDictionary();
}

urbi::UBinary
CallbacksCaller::getUBinaryFromObject(jobject obj)
{
  if (obj)
  {
    jlong ptr = env_->GetLongField(obj, ubinary_swigptr_id);
    if (ptr)  /// Java allocated memory, prefer allocate mine
      return *(urbi::UBinary*) ptr;
    else
      return urbi::UBinary();
  }
  else
    return urbi::UBinary();
}

urbi::UImage
CallbacksCaller::getUImageFromObject(jobject obj)
{
  if (obj)
  {
    jlong ptr = env_->GetLongField(obj, uimage_swigptr_id);
    if (ptr)  /// Java allocated memory, prefer allocate mine
      return *(urbi::UImage*) ptr;
    else
      return urbi::UImage();
  }
  else
    return urbi::UImage();
}

urbi::USound
CallbacksCaller::getUSoundFromObject(jobject obj)
{
  if (obj)
  {
    jlong ptr = env_->GetLongField(obj, usound_swigptr_id);
    if (ptr)  /// Java allocated memory, prefer allocate mine
      return *(urbi::USound*) ptr;
    else
      return urbi::USound();
  }
  else
    return urbi::USound();
}

urbi::UList
CallbacksCaller::getUListFromObject(jobject obj)
{
  if (obj)
  {
    jlong ptr = env_->GetLongField(obj, ulist_swigptr_id);
    if (ptr)  /// Java allocated memory, prefer allocate mine
      return *(urbi::UList*) ptr;
    else
      return urbi::UList();
  }
  else
    return urbi::UList();
}

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
    throw std::runtime_error(n + ": " + m);
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
