/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/format.hh>
#include "callbacks-caller.hh"


jmethodID	CallbacksCaller::double_valueof_id = 0;
jclass  	CallbacksCaller::double_cls = 0;
jmethodID	CallbacksCaller::float_valueof_id = 0;
jclass  	CallbacksCaller::float_cls = 0;
jmethodID	CallbacksCaller::long_valueof_id = 0;
jclass  	CallbacksCaller::long_cls = 0;
jmethodID	CallbacksCaller::short_valueof_id = 0;
jclass  	CallbacksCaller::short_cls = 0;
jmethodID	CallbacksCaller::character_valueof_id = 0;
jclass  	CallbacksCaller::character_cls = 0;
jmethodID	CallbacksCaller::byte_valueof_id = 0;
jclass  	CallbacksCaller::byte_cls = 0;
jmethodID	CallbacksCaller::boolean_valueof_id = 0;
jclass  	CallbacksCaller::boolean_cls = 0;
jmethodID	CallbacksCaller::integer_valueof_id = 0;
jclass  	CallbacksCaller::integer_cls = 0;
jmethodID	CallbacksCaller::class_getname_id = 0;
jclass  	CallbacksCaller::class_cls = 0;
jmethodID	CallbacksCaller::string_ctor_id = 0;
jclass  	CallbacksCaller::string_cls = 0;
jmethodID	CallbacksCaller::ulist_ctor_id = 0;
jclass  	CallbacksCaller::ulist_cls = 0;
jmethodID	CallbacksCaller::uimage_ctor_id = 0;
jclass  	CallbacksCaller::uimage_cls = 0;
jmethodID	CallbacksCaller::usound_ctor_id = 0;
jclass  	CallbacksCaller::usound_cls = 0;
jmethodID	CallbacksCaller::ubinary_ctor_id = 0;
jclass  	CallbacksCaller::ubinary_cls = 0;
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
CallbacksCaller::callNotifyChangeInt_0 ()
{
  if (!init_env ())
    return 0;
  int ret = env_->CallIntMethod(obj, mid);
  testForException();
  return ret;
}

int
CallbacksCaller::callNotifyChangeInt_1 (urbi::UVar& v)
{
  if (!init_env ())
    return 0;
  jvalue obj1;
  if (arg_types[0] == "class urbi.generated.UVar")
    obj1.l = getObjectFromUVar (v);
  else
    obj1 = getObjectFrom (arg_types[0], v.val());
  jvalue arg[] = { obj1 };
  int ret = env_->CallIntMethodA(obj, mid, arg);
  testForException();
  return ret;
}

void
CallbacksCaller::callNotifyChangeVoid_0 ()
{
  if (!init_env ())
    return;
  env_->CallVoidMethod(obj, mid);
  testForException();
}

void
CallbacksCaller::callNotifyChangeVoid_1 (urbi::UVar& v)
{
  if (!init_env ())
    return;
  jvalue obj1;
  if (arg_types[0] == "class urbi.generated.UVar")
    obj1.l = getObjectFromUVar (v);
  else
    obj1 = getObjectFrom (arg_types[0], v.val());
  jvalue arg[] = { obj1 };
  env_->CallVoidMethodA(obj, mid, arg);
  testForException();
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

void
CallbacksCaller::deleteClassRefs(JNIEnv* env)
{
  if (uvalue_cls)
    env->DeleteGlobalRef (uvalue_cls);
  if (uvar_cls)
    env->DeleteGlobalRef (uvar_cls);
  if (uobject_cls)
    env->DeleteGlobalRef (uobject_cls);
  if (string_cls)
    env->DeleteGlobalRef (string_cls);
  if (class_cls)
    env->DeleteGlobalRef (class_cls);
  if (integer_cls)
    env->DeleteGlobalRef (integer_cls);
  if (boolean_cls)
    env->DeleteGlobalRef (boolean_cls);
  if (byte_cls)
    env->DeleteGlobalRef (byte_cls);
  if (character_cls)
    env->DeleteGlobalRef (character_cls);
  if (short_cls)
    env->DeleteGlobalRef (short_cls);
  if (long_cls)
    env->DeleteGlobalRef (long_cls);
  if (float_cls)
    env->DeleteGlobalRef (float_cls);
  if (double_cls)
    env->DeleteGlobalRef (double_cls);
  if (ulist_cls)
    env->DeleteGlobalRef (ulist_cls);
  if (ubinary_cls)
    env->DeleteGlobalRef (ubinary_cls);
  if (usound_cls)
    env->DeleteGlobalRef (usound_cls);
  if (uimage_cls)
    env->DeleteGlobalRef (uimage_cls);
}

bool
CallbacksCaller::cacheJNIVariables (JNIEnv* env)
{

#define CLEAN_AND_THROW(msg)			\
  do						\
  {						\
    deleteClassRefs(env);			\
    TROW_RUNTIME(env, msg);			\
    return false;				\
  } while (0)


  /// Get the jclass for UValue
  if (!(uvalue_cls = getGlobalRef (env, "urbi/generated/UValue")))
    CLEAN_AND_THROW("Can't find UValue class");

  /// Get UValue (int, bool) Constructor id
  if (!(uvalue_ctor_id = env->GetMethodID(uvalue_cls, "<init>", "(JZ)V")))
    CLEAN_AND_THROW("Can't find UValue constructor");

  /// Get UValue swigCPtr attribute id
  if (!(uvalue_swigptr_id = env->GetFieldID(uvalue_cls, "swigCPtr", "J")))
    CLEAN_AND_THROW("Can't find UValue swigCPtr");

  /// Get the jclass for Ulist
  if (!(ulist_cls = getGlobalRef (env, "urbi/generated/UList")))
    CLEAN_AND_THROW("Can't find UList class");

  /// Get Ulist (int, bool) Constructor id
  if (!(ulist_ctor_id = env->GetMethodID(ulist_cls, "<init>", "(JZ)V")))
    CLEAN_AND_THROW("Can't find UList constructor");

  /// Get the jclass for Uimage
  if (!(uimage_cls = getGlobalRef (env, "urbi/generated/UImage")))
    CLEAN_AND_THROW("Can't find UImage class");

  /// Get Uimage (int, bool) Constructor id
  if (!(uimage_ctor_id = env->GetMethodID(uimage_cls, "<init>", "(JZ)V")))
    CLEAN_AND_THROW("Can't find UImage constructor");

  /// Get the jclass for Ubinary
  if (!(ubinary_cls = getGlobalRef (env, "urbi/generated/UBinary")))
    CLEAN_AND_THROW("Can't find UBinary class");

  /// Get Ubinary (int, bool) Constructor id
  if (!(ubinary_ctor_id = env->GetMethodID(ubinary_cls, "<init>", "(JZ)V")))
    CLEAN_AND_THROW("Can't find UBinary constructor");

  /// Get the jclass for Usound
  if (!(usound_cls = getGlobalRef (env, "urbi/generated/USound")))
    CLEAN_AND_THROW("Can't find USound class");

  /// Get Usound (int, bool) Constructor id
  if (!(usound_ctor_id = env->GetMethodID(usound_cls, "<init>", "(JZ)V")))
    CLEAN_AND_THROW("Can't find USound constructor");

  /// Get the jclass for UVar
  if (!(uvar_cls = getGlobalRef (env, "urbi/generated/UVar")))
    CLEAN_AND_THROW("Can't find UVar class");

  if (!(uvar_ctor_id = env->GetMethodID(uvar_cls, "<init>", "(JZ)V")))
    CLEAN_AND_THROW("Can't find UVar constructor");

  /// Get UValue swigCPtr attribute id
  if (!(uvar_swigptr_id = env->GetFieldID(uvar_cls, "swigCPtr", "J")))
    CLEAN_AND_THROW("Can't find UVar swigCPtr");

  /// Get the jclass for UObject
  if (!(uobject_cls = getGlobalRef(env, "urbi/generated/UObjectCPP")))
    CLEAN_AND_THROW("Can't find UObject class");

  /// Get UObject swigCPtr attribute id
  if (!(uobject_swigptr_id = env->GetFieldID(uobject_cls, "swigCPtr", "J")))
    CLEAN_AND_THROW("Can't find UObject swigCPtr");

  /// Get the jclass for String
  if (!(string_cls = getGlobalRef (env, "java/lang/String")))
    CLEAN_AND_THROW("Can't find String class");

  /// Get String (char) Constructor id
  if (!(string_ctor_id = env->GetMethodID(string_cls, "<init>", "([C)V")))
    CLEAN_AND_THROW("Can't find String constructor");

  /// Get the jclass for Class
  if (!(class_cls = getGlobalRef (env, "java/lang/Class")))
    CLEAN_AND_THROW("Can't find Class class");

  /// Get String (char) Constructor id
  if (!(class_getname_id = env->GetMethodID(class_cls, "getName", "()Ljava/lang/String;")))
    CLEAN_AND_THROW("Can't find Class getName function");

  /// Get the jclass for Integer
  if (!(integer_cls = getGlobalRef (env, "java/lang/Integer")))
    CLEAN_AND_THROW("Can't find Integer class");

  /// Get Integer::valueOf (int) method id
  if (!(integer_valueof_id = env->GetStaticMethodID(integer_cls, "valueOf", "(I)Ljava/lang/Integer;")))
    CLEAN_AND_THROW("Can't find Integer valueOf function");

  /// Get the jclass for Boolean
  if (!(boolean_cls = getGlobalRef (env, "java/lang/Boolean")))
    CLEAN_AND_THROW("Can't find Boolean class");

  /// Get Boolean::valueOf (boolean) method id
  if (!(boolean_valueof_id = env->GetStaticMethodID(boolean_cls, "valueOf", "(Z)Ljava/lang/Boolean;")))
    CLEAN_AND_THROW("Can't find Boolean valueOf function");

  /// Get the jclass for Byte
  if (!(byte_cls = getGlobalRef (env, "java/lang/Byte")))
    CLEAN_AND_THROW("Can't find Byte class");

  /// Get Byte::valueOf (byte) method id
  if (!(byte_valueof_id = env->GetStaticMethodID(byte_cls, "valueOf", "(B)Ljava/lang/Byte;")))
    CLEAN_AND_THROW("Can't find Byte valueOf function");

  /// Get the jclass for Character
  if (!(character_cls = getGlobalRef (env, "java/lang/Character")))
    CLEAN_AND_THROW("Can't find Character class");

  /// Get Character::valueOf (character) method id
  if (!(character_valueof_id = env->GetStaticMethodID(character_cls, "valueOf", "(C)Ljava/lang/Character;")))
    CLEAN_AND_THROW("Can't find Character valueOf function");

  /// Get the jclass for Short
  if (!(short_cls = getGlobalRef (env, "java/lang/Short")))
    CLEAN_AND_THROW("Can't find Short class");

  /// Get Short::valueOf (short) method id
  if (!(short_valueof_id = env->GetStaticMethodID(short_cls, "valueOf", "(S)Ljava/lang/Short;")))
    CLEAN_AND_THROW("Can't find Short valueOf function");

  /// Get the jclass for Long
  if (!(long_cls = getGlobalRef (env, "java/lang/Long")))
    CLEAN_AND_THROW("Can't find Long class");

  /// Get Long::valueOf (long) method id
  if (!(long_valueof_id = env->GetStaticMethodID(long_cls, "valueOf", "(J)Ljava/lang/Long;")))
    CLEAN_AND_THROW("Can't find Long valueOf function");

  /// Get the jclass for Float
  if (!(float_cls = getGlobalRef (env, "java/lang/Float")))
    CLEAN_AND_THROW("Can't find Float class");

  /// Get Float::valueOf (float) method id
  if (!(float_valueof_id = env->GetStaticMethodID(float_cls, "valueOf", "(F)Ljava/lang/Float;")))
    CLEAN_AND_THROW("Can't find Float valueOf function");

  /// Get the jclass for Double
  if (!(double_cls = getGlobalRef (env, "java/lang/Double")))
    CLEAN_AND_THROW("Can't find Double class");

  /// Get Double::valueOf (double) method id
  if (!(double_valueof_id = env->GetStaticMethodID(double_cls, "valueOf", "(D)Ljava/lang/Double;")))
    CLEAN_AND_THROW("Can't find Double valueOf function");

#undef CLEAN_AND_THROW

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

jvalue
CallbacksCaller::getObjectFrom (const std::string& type_name, urbi::UValue v)
{
  jvalue res;
  if (type_name.length() > 10)
  {
    if (type_name == "class urbi.generated.UValue")
      res.l = getObjectFromUValue (v);
    else if (type_name == "class java.lang.String")
      res.l = getObjectFromString (v);
    else if (type_name == "class urbi.generated.UVar")
      res.l = getObjectFromUVar (urbi::uvar_uvalue_cast<urbi::UVar>(v));
    else if (type_name == "class urbi.generated.UList")
      res.l = getObjectFromUList (v);
    else if (type_name == "class urbi.generated.UBinary")
      res.l = getObjectFromUBinary (v);
    else if (type_name == "class urbi.generated.UImage")
      res.l = getObjectFromUImage (v);
    else if (type_name == "class urbi.generated.USound")
      res.l = getObjectFromUSound (v);
    else if (type_name == "class java.lang.Integer")
      res.l = getObjectFromInteger(v);
    else if (type_name == "class java.lang.Boolean")
      res.l = getObjectFromBoolean(v);
    else if (type_name == "class java.lang.Double")
      res.l = getObjectFromDouble(v);
    else if (type_name == "class java.lang.Float")
      res.l = getObjectFromFloat(v);
    else if (type_name == "class java.lang.Long")
      res.l = getObjectFromLong(v);
    else if (type_name == "class java.lang.Short")
      res.l = getObjectFromShort(v);
    else if (type_name == "class java.lang.Character")
      res.l = getObjectFromCharacter(v);
    else if (type_name == "class java.lang.Byte")
      res.l = getObjectFromByte(v);
    else
      throw std::runtime_error(libport::format("type %s not supported", type_name));
  }
  else
  {
    if (type_name == "int")
      res.i = (jint) v;
    else if (type_name == "boolean")
      res.z = (jboolean) (bool) v;
    else if (type_name == "byte")
      res.b = (jbyte) (int) v;
    else if (type_name == "char")
      res.c = (jchar) (int) v;
    else if (type_name == "short")
      res.s = (jshort) (int) v;
    else if (type_name == "long")
      res.j = (jlong) (long) v;
    else if (type_name == "float")
      res.f = (jfloat) (ufloat) v;
    else if (type_name == "double")
      res.d = (jdouble) v;
    else
      throw std::runtime_error(libport::format("type %s not supported", type_name));
  }
  return res;
}

jobject
CallbacksCaller::getObjectFromDouble (double val)
{
  jobject res = env_->CallStaticObjectMethod(double_cls, double_valueof_id, (jdouble) val);
  if (!res)
    std::cerr << "Cannot allocate a new object of type java.lang.Double"
	      << std::endl;
  return res;
}

jobject
CallbacksCaller::getObjectFromFloat (ufloat val)
{
  jobject res = env_->CallStaticObjectMethod(float_cls, float_valueof_id, (jfloat) val);
  if (!res)
    std::cerr << "Cannot allocate a new object of type java.lang.Float"
	      << std::endl;
  return res;
}

jobject
CallbacksCaller::getObjectFromLong (long val)
{
  jobject res = env_->CallStaticObjectMethod(long_cls, long_valueof_id, (jlong) val);
  if (!res)
    std::cerr << "Cannot allocate a new object of type java.lang.Long"
	      << std::endl;
  return res;
}

jobject
CallbacksCaller::getObjectFromShort (int val)
{
  jobject res = env_->CallStaticObjectMethod(short_cls, short_valueof_id, (jshort) val);
  if (!res)
    std::cerr << "Cannot allocate a new object of type java.lang.Short"
	      << std::endl;
  return res;
}

jobject
CallbacksCaller::getObjectFromCharacter (int val)
{
  jobject res = env_->CallStaticObjectMethod(character_cls, character_valueof_id, (jchar) val);
  if (!res)
    std::cerr << "Cannot allocate a new object of type java.lang.Character"
	      << std::endl;
  return res;
}

jobject
CallbacksCaller::getObjectFromByte (int val)
{
  jobject res = env_->CallStaticObjectMethod(byte_cls, byte_valueof_id, (jbyte) val);
  if (!res)
    std::cerr << "Cannot allocate a new object of type java.lang.Byte"
	      << std::endl;
  return res;
}

jobject
CallbacksCaller::getObjectFromBoolean (bool val)
{
  jobject res = env_->CallStaticObjectMethod(boolean_cls, boolean_valueof_id, (jboolean) val);
  if (!res)
    std::cerr << "Cannot allocate a new object of type java.lang.Boolean"
	      << std::endl;
  return res;
}

jobject
CallbacksCaller::getObjectFromInteger (int val)
{
  jobject res = env_->CallStaticObjectMethod(integer_cls, integer_valueof_id, val);
  if (!res)
    std::cerr << "Cannot allocate a new object of type java.lang.Integer"
	      << std::endl;
  return res;
}

jobject
CallbacksCaller::getObjectFromString (const std::string& val)
{
  jobject res = env_->NewStringUTF(val.c_str());
  if (!res)
    std::cerr << "Cannot allocate a new object of type java.lang.String"
	      << std::endl;
  return res;
}

jobject
CallbacksCaller::getObjectFromUList (const urbi::UList& v)
{
  jobject res = env_->NewObject(ulist_cls, ulist_ctor_id, (jlong) new urbi::UList(v), true);
  if (!res)
    std::cerr << "Cannot allocate a new object of type urbi.generated.UList"
	      << std::endl;
  return res;
}

jobject
CallbacksCaller::getObjectFromUBinary (const urbi::UBinary& v)
{
  jobject res = env_->NewObject(ubinary_cls, ubinary_ctor_id, (jlong) new urbi::UBinary(v), true);
  if (!res)
    std::cerr << "Cannot allocate a new object of type urbi.generated.UBinary"
	      << std::endl;
  return res;
}

jobject
CallbacksCaller::getObjectFromUImage (const urbi::UImage& v)
{
  jobject res = env_->NewObject(uimage_cls, uimage_ctor_id, (jlong) new urbi::UImage(v), true);
  if (!res)
    std::cerr << "Cannot allocate a new object of type urbi.generated.UImage"
	      << std::endl;
  return res;
}

jobject
CallbacksCaller::getObjectFromUSound (const urbi::USound& v)
{
  jobject res = env_->NewObject(usound_cls, usound_ctor_id, (jlong) new urbi::USound(v), true);
  if (!res)
    std::cerr << "Cannot allocate a new object of type urbi.generated.USound"
	      << std::endl;
  return res;
}

jobject
CallbacksCaller::getObjectFromUValue (const urbi::UValue& v)
{
  jobject res = env_->NewObject(uvalue_cls, uvalue_ctor_id, (jlong) &v, false);
  if (!res)
    std::cerr << "Cannot allocate a new object of type urbi.generated.UValue"
	      << std::endl;
  return res;
}

jobject
CallbacksCaller::getObjectFromUVar (urbi::UVar& v)
{
  jobject res = env_->NewObject(uvar_cls, uvar_ctor_id, (jlong) &v, false);
  if  (!res)
    std::cerr << "Cannot allocate a new object of type urbi.generated.UVar"
	      << std::endl;
  return res;
}

void
CallbacksCaller::testForException()
{
  jthrowable exc = env_->ExceptionOccurred();
  if (exc) {
    env_->ExceptionDescribe();
    env_->ExceptionClear();
    jclass java_class = env_->GetObjectClass (exc);
    assert(java_class);
    jmethodID getMessage = env_->GetMethodID (java_class,
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
