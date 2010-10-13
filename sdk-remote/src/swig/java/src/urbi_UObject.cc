/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// The code in this file is associated to the java class:
/// liburbi.main.UObjectJava

#include <iostream>
#include <libport/cstdlib>
#include <libport/cstdio>
#include <urbi/fwd.hh>
#include "callbacks-caller.hh"
#include "urbi_UObject.h"

#define CREATE_FUN_CALLBACK(uobj, obj, fun, func_name)			\
  ::urbi::createUCallback(*uobj, 0, "function", obj, fun, func_name)



#define createFunctionCallbackType(RetType)				\
static urbi::UGenericCallback*						\
createFunctionCallback##RetType (JNIEnv* env,				\
                                 urbi::UObject *uobj,			\
                                 CallbacksCaller* obj,			\
                                 std::string obj_name,			\
				 std::string funcName,			\
				 int argNb)				\
{									\
  switch (argNb)							\
  {									\
    case 0:								\
      return CREATE_FUN_CALLBACK(uobj, obj, (&CallbacksCaller::call##RetType##_0), funcName); \
    case 1:								\
      return CREATE_FUN_CALLBACK(uobj, obj, (&CallbacksCaller::call##RetType##_1), funcName); \
    case 2:								\
      return CREATE_FUN_CALLBACK(uobj, obj, (&CallbacksCaller::call##RetType##_2), funcName); \
    case 3:								\
      return CREATE_FUN_CALLBACK(uobj, obj, (&CallbacksCaller::call##RetType##_3), funcName); \
    case 4:								\
      return CREATE_FUN_CALLBACK(uobj, obj, (&CallbacksCaller::call##RetType##_4), funcName); \
    case 5:								\
      return CREATE_FUN_CALLBACK(uobj, obj, (&CallbacksCaller::call##RetType##_5), funcName); \
    case 6:								\
      return CREATE_FUN_CALLBACK(uobj, obj, (&CallbacksCaller::call##RetType##_6), funcName); \
    case 7:								\
      return CREATE_FUN_CALLBACK(uobj, obj, (&CallbacksCaller::call##RetType##_7), funcName); \
    case 8:								\
      return CREATE_FUN_CALLBACK(uobj, obj, (&CallbacksCaller::call##RetType##_8), funcName); \
    case 9:								\
      return CREATE_FUN_CALLBACK(uobj, obj, (&CallbacksCaller::call##RetType##_9), funcName); \
    case 10:								\
      return CREATE_FUN_CALLBACK(uobj, obj, (&CallbacksCaller::call##RetType##_10), funcName); \
    case 11:								\
      return CREATE_FUN_CALLBACK(uobj, obj, (&CallbacksCaller::call##RetType##_11), funcName); \
    case 12:								\
      return CREATE_FUN_CALLBACK(uobj, obj, (&CallbacksCaller::call##RetType##_12), funcName); \
    case 13:								\
      return CREATE_FUN_CALLBACK(uobj, obj, (&CallbacksCaller::call##RetType##_13), funcName); \
    case 14:								\
      return CREATE_FUN_CALLBACK(uobj, obj, (&CallbacksCaller::call##RetType##_14), funcName); \
    case 15:								\
      return CREATE_FUN_CALLBACK(uobj, obj, (&CallbacksCaller::call##RetType##_15), funcName); \
    case 16:								\
      return CREATE_FUN_CALLBACK(uobj, obj, (&CallbacksCaller::call##RetType##_16), funcName); \
    default:								\
      char tmp[256];							\
      std::string msg = "Can't define a callback on a function with ";	\
      sprintf(tmp,"%d", argNb);						\
      msg += tmp;							\
      msg += " arguments. Maximum is 16.";				\
      THROW_RUNTIME(env, msg.c_str ());					\
  }									\
  return 0;								\
}

createFunctionCallbackType(Void);
createFunctionCallbackType(Boolean);
createFunctionCallbackType(Byte);
createFunctionCallbackType(Char);
createFunctionCallbackType(Short);
createFunctionCallbackType(Int);
createFunctionCallbackType(Long);
createFunctionCallbackType(Float);
createFunctionCallbackType(Double);
createFunctionCallbackType(UValue);
createFunctionCallbackType(UList);
createFunctionCallbackType(UBinary);
createFunctionCallbackType(UImage);
createFunctionCallbackType(USound);
createFunctionCallbackType(UDictionary);
createFunctionCallbackType(String);


static urbi::UGenericCallback* createFunctionCallback (JNIEnv* env,
						       urbi::UObject* uobj,
						       CallbacksCaller* obj,
						       const std::string obj_name,
						       const std::string& funcName,
						       const std::string& retType,
						       int argNb)
{
  if ("void" == retType)
    return createFunctionCallbackVoid (env, uobj, obj, obj_name, funcName, argNb);
  else if ("boolean" == retType)
    return createFunctionCallbackBoolean (env, uobj, obj, obj_name, funcName, argNb);
  else if ("byte" == retType)
    return createFunctionCallbackByte (env, uobj, obj, obj_name, funcName, argNb);
  else if ("char" == retType)
    return createFunctionCallbackChar (env, uobj, obj, obj_name, funcName, argNb);
  else if ("short" == retType)
    return createFunctionCallbackShort (env, uobj, obj, obj_name, funcName, argNb);
  else if ("int" == retType)
    return createFunctionCallbackInt (env, uobj, obj, obj_name, funcName, argNb);
  else if ("long" == retType)
    return createFunctionCallbackLong (env, uobj, obj, obj_name, funcName, argNb);
  else if ("float" == retType)
    return createFunctionCallbackFloat (env, uobj, obj, obj_name, funcName, argNb);
  else if ("double" == retType)
    return createFunctionCallbackDouble (env, uobj, obj, obj_name, funcName, argNb);
  else if ("urbi.UValue" == retType)
    return createFunctionCallbackUValue (env, uobj, obj, obj_name, funcName, argNb);
  else if ("urbi.UList" == retType)
    return createFunctionCallbackUList (env, uobj, obj, obj_name, funcName, argNb);
  else if ("urbi.UBinary" == retType)
    return createFunctionCallbackUBinary (env, uobj, obj, obj_name, funcName, argNb);
  else if ("urbi.UImage" == retType)
    return createFunctionCallbackUImage (env, uobj, obj, obj_name, funcName, argNb);
  else if ("urbi.USound" == retType)
    return createFunctionCallbackUSound (env, uobj, obj, obj_name, funcName, argNb);
  else if ("urbi.UDictionary" == retType)
    return createFunctionCallbackUDictionary (env, uobj, obj, obj_name, funcName, argNb);
  else if ("java.lang.String" == retType)
    return createFunctionCallbackString (env, uobj, obj, obj_name, funcName, argNb);

  std::string msg = "Can't define a callback on a function with return type ";
  msg += retType;
  THROW_RUNTIME(env, msg.c_str ());
  return 0;
}


struct MethodIdAndUrbiName
{
  jmethodID	java_mid;
  std::string	urbi_method_name;
};

static MethodIdAndUrbiName getMethodIdAndUrbiName (JNIEnv *env,
						   jobject obj,
						   jstring obj_name,
						   jstring method_name,
						   jstring method_signature)
{
  MethodIdAndUrbiName res;
  res.java_mid = 0;

  jclass cls = env->GetObjectClass(obj);
  if (!cls)
  {
    THROW_RUNTIME(env, "Can't find java object class");
    return res;
  }

  const char* method_name_ = env->GetStringUTFChars(method_name, 0);
  const char* method_signature_ = env->GetStringUTFChars(method_signature, 0);

  res.java_mid = env->GetMethodID(cls, method_name_, method_signature_);
  if (!res.java_mid)
  {
    THROW_RUNTIME(env, "Can't find java callback method id");
    return res;
  }

  const char* obj_name_ = env->GetStringUTFChars(obj_name, 0);

  res.urbi_method_name = obj_name_;
  res.urbi_method_name += ".";
  res.urbi_method_name += method_name_;

  env->ReleaseStringUTFChars(method_name, method_name_);
  env->ReleaseStringUTFChars(method_signature, method_signature_);
  env->ReleaseStringUTFChars(obj_name, obj_name_);

  return res;
}


void registerNotify (JNIEnv *env,
		     jobject obj,
		     jlong var, // can contains NULL UVar (in case name is given)
		     jstring var_name,
		     jboolean is_owned,
		     jstring obj_name,
		     jstring method_name,
		     jstring method_signature,
		     jstring return_type,
		     jint arg_nb,
		     const char *notify_type,
		     jobjectArray types)
{
  /// First, assure that the JNI variables used by the caller are correctly
  /// set. If the are not and we can't set them, return.
  if (!CallbacksCaller::areJNIVariablesCached ()
      && !CallbacksCaller::cacheJNIVariables (env))
      return;

  MethodIdAndUrbiName miaun =
    getMethodIdAndUrbiName (env, obj, obj_name, method_name, method_signature);
  if (!miaun.java_mid)
    return;

  CallbacksCaller *f = new CallbacksCaller ();
  f->setObject (env->NewGlobalRef(obj));
  f->setMethodID (miaun.java_mid);

  const char* uvar_name = env->GetStringUTFChars(var_name, 0);
  const char* obj_name_ = env->GetStringUTFChars(obj_name, 0);

  if (arg_nb > 0) {

    jstring jstr = (jstring) env->GetObjectArrayElement(types, 0);
    const char* type_ = env->GetStringUTFChars(jstr, 0);
    Converter* c = Converter::instance(type_, true);
    f->arg_convert.push_back(c);
    env->ReleaseStringUTFChars(jstr, type_);
  }
  urbi::UObject* uob = CallbacksCaller::getUObjectFromObject(obj, env);
  if (!uob)
  {
    delete f;
    env->DeleteGlobalRef (obj);
    env->ReleaseStringUTFChars(obj_name, obj_name_);
    env->ReleaseStringUTFChars(var_name, uvar_name);
    return;
  }

  const char* return_type_ = env->GetStringUTFChars(return_type, 0);
  const std::string ret_type = return_type_;
  switch ((int) arg_nb)
  {
    case 0:
      if (ret_type == "int")
	::urbi::createUCallback(*uob, (urbi::UVar*) var, notify_type, f,
				(&CallbacksCaller::callNotifyChangeInt_0),
				uvar_name);
      else if (ret_type == "void")
	::urbi::createUCallback(*uob, (urbi::UVar*) var, notify_type, f,
				(&CallbacksCaller::callNotifyChangeVoid_0),
				uvar_name);
      else
	throw std::runtime_error(libport::format("%s is not supported as a return type for notify change", return_type_));
      break;
    case 1:
      if (ret_type == "int")
	::urbi::createUCallback(*uob, new urbi::UVar(uvar_name), notify_type, f,
				(&CallbacksCaller::callNotifyChangeInt_1),
				uvar_name);
      else if (ret_type == "void")
	::urbi::createUCallback(*uob, new urbi::UVar(uvar_name), notify_type, f,
				(&CallbacksCaller::callNotifyChangeVoid_1),
				uvar_name);
      else
	throw std::runtime_error(libport::format("%s is not supported as a return type for notify change", return_type_));
      break;
  }
  env->ReleaseStringUTFChars(return_type, return_type_);
  env->ReleaseStringUTFChars(obj_name, obj_name_);
  env->ReleaseStringUTFChars(var_name, uvar_name);
}


JNIEXPORT void JNICALL
Java_urbi_UObject_registerNotifyChange(JNIEnv *env,
					   jobject obj,
					   jlong var,
					   jstring var_name,
					   jboolean is_owned,
					   jstring obj_name,
					   jstring method_name,
					   jstring method_signature,
				       jstring return_type,
					   jint arg_nb,
				       jobjectArray types)
{
  registerNotify (env, obj, var, var_name, is_owned, obj_name,
		  method_name, method_signature, return_type, arg_nb, "var", types);
}

JNIEXPORT void JNICALL
Java_urbi_UObject_registerNotifyOnRequest(JNIEnv *env,
					      jobject obj,
					      jlong var,
					      jstring var_name,
					      jboolean is_owned,
					      jstring obj_name,
					      jstring method_name,
					      jstring method_signature,
				       jstring return_type,
					      jint arg_nb,
					  jobjectArray types)
{
  registerNotify (env, obj, var, var_name, is_owned, obj_name,
		  method_name, method_signature, return_type, arg_nb, "var_onrequest", types);
}



JNIEXPORT void JNICALL
Java_urbi_UObject_registerFunction(JNIEnv *env,
				       jobject,
				       jobject obj,
				       jstring obj_name,
				       jstring method_name,
				       jstring method_signature,
				       jstring return_type,
				   jint arg_nb,
				   jobjectArray types)
{
  /// First, assure that the JNI variables used by the caller are correctly
  /// set. If the are not and we can't set them, return.
  if (!CallbacksCaller::areJNIVariablesCached ()
      && !CallbacksCaller::cacheJNIVariables (env))
      return;

  MethodIdAndUrbiName miaun =
    getMethodIdAndUrbiName (env, obj, obj_name, method_name, method_signature);
  if (!miaun.java_mid)
    return;

  CallbacksCaller *f = new CallbacksCaller ();
  f->setObject (env->NewGlobalRef(obj));
  f->setMethodID (miaun.java_mid);

  const char* return_type_ = env->GetStringUTFChars(return_type, 0);
  const char* obj_name_ = env->GetStringUTFChars(obj_name, 0);

  for (int i = 0; i < arg_nb; ++i)
  {
    jstring jstr = (jstring) env->GetObjectArrayElement(types, i);
    const char* type_ = env->GetStringUTFChars(jstr, 0);
    Converter* c = Converter::instance(type_);
    f->arg_convert.push_back(c);
    env->ReleaseStringUTFChars(jstr, type_);
  }
  urbi::UObject* uob = CallbacksCaller::getUObjectFromObject(obj, env);
  if (!uob)
  {
    delete f;
    env->DeleteGlobalRef (obj);
  }
  else
    if (!createFunctionCallback (env, uob, f, obj_name_,
				 miaun.urbi_method_name, return_type_, arg_nb))
    {
      delete f;
      env->DeleteGlobalRef (obj);
    }

  env->ReleaseStringUTFChars(obj_name, obj_name_);
  env->ReleaseStringUTFChars(return_type, return_type_);
}


JNIEXPORT jstring JNICALL
Java_urbi_UObject_registerTimerFunction(JNIEnv *env,
					    jobject,
					    jobject obj,
					    jstring obj_name,
					    jdouble period,
					    jstring method_name,
					    jstring method_signature,
					    jstring return_type,
					    jint arg_nb)
{
  /// First, assure that the JNI variables used by the caller are correctly
  /// set. If the are not and we can't set them, return.
  if (!CallbacksCaller::areJNIVariablesCached ()
      && !CallbacksCaller::cacheJNIVariables (env))
    return StringConverter::staticConvert(env, "");

  MethodIdAndUrbiName miaun =
    getMethodIdAndUrbiName (env, obj, obj_name, method_name, method_signature);
  if (!miaun.java_mid)
    return StringConverter::staticConvert(env, "");

  CallbacksCaller *f = new CallbacksCaller ();
  f->setObject (env->NewGlobalRef(obj));
  f->setMethodID (miaun.java_mid);

  const char* return_type_ = env->GetStringUTFChars(return_type, 0);
  const char* obj_name_ = env->GetStringUTFChars(obj_name, 0);

  std::string res = "";
  switch ((int) arg_nb)
  {
    case 0:
      urbi::TimerHandle th =
	(new urbi::UTimerCallbackobj<CallbacksCaller> (obj_name_,
						       (double) period,
						       f,
						       boost::bind(&CallbacksCaller::callNotifyChangeInt_0, f),
						       urbi::getCurrentContext()))->handle_get();
      res = *th;
      break;
  }
  env->ReleaseStringUTFChars(obj_name, obj_name_);
  env->ReleaseStringUTFChars(return_type, return_type_);
  return StringConverter::staticConvert(env, res);
}
