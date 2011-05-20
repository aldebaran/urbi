/*
 * Copyright (C) 2010, 2011, Gostai S.A.S.
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

#define CASE(Ret, N)                                                    \
  case N:                                                               \
  return createUCallback(*uobj, 0, "function", obj,                     \
                         &CallbacksCaller::call ## Ret ## _ ## N,       \
                         funcName)

#define DEFINE(RetType)                                                 \
  static urbi::UGenericCallback*                                        \
  createFunctionCallback##RetType (JNIEnv* env,				\
                                   urbi::UObject* uobj,			\
                                   CallbacksCaller* obj,                \
                                   std::string obj_name,                \
                                   std::string funcName,                \
                                   int argNb)				\
  {									\
    using ::urbi::createUCallback;                                      \
    switch (argNb)							\
    {									\
      CASE(RetType, 0);                                                 \
      CASE(RetType, 1);                                                 \
      CASE(RetType, 2);                                                 \
      CASE(RetType, 3);                                                 \
      CASE(RetType, 4);                                                 \
      CASE(RetType, 5);                                                 \
      CASE(RetType, 6);                                                 \
      CASE(RetType, 7);                                                 \
      CASE(RetType, 8);                                                 \
      CASE(RetType, 9);                                                 \
      CASE(RetType, 10);                                                \
      CASE(RetType, 11);                                                \
      CASE(RetType, 12);                                                \
      CASE(RetType, 13);                                                \
      CASE(RetType, 14);                                                \
      CASE(RetType, 15);                                                \
      CASE(RetType, 16);                                                \
    }                                                                   \
    FRAISE("Can't define a callback on a function with %d arguments."   \
           " Maximum is 16.", argNb);                                   \
  }

DEFINE(Boolean);
DEFINE(Byte);
DEFINE(Char);
DEFINE(Double);
DEFINE(Float);
DEFINE(Int);
DEFINE(Long);
DEFINE(Short);
DEFINE(String);
DEFINE(UBinary);
DEFINE(UDictionary);
DEFINE(UImage);
DEFINE(UList);
DEFINE(USound);
DEFINE(UValue);
DEFINE(Void);
#undef CASE
#undef DEFINE

static urbi::UGenericCallback*
createFunctionCallback(JNIEnv* env,
                       urbi::UObject* uobj,
                       CallbacksCaller* obj,
                       const std::string obj_name,
                       const std::string& funcName,
                       const std::string& retType,
                       int argNb)
{
#define CASE(Name, JavaName)                                            \
  if (retType == #Name)                                                 \
    return createFunctionCallback ## JavaName(env, uobj, obj,           \
                                              obj_name, funcName, argNb)
  CASE(boolean, Boolean);
  CASE(byte, Byte);
  CASE(char, Char);
  CASE(double, Double);
  CASE(float, Float);
  CASE(int, Int);
  CASE(java.lang.String, String);
  CASE(long, Long);
  CASE(short, Short);
  CASE(urbi.UBinary, UBinary);
  CASE(urbi.UDictionary, UDictionary);
  CASE(urbi.UImage, UImage);
  CASE(urbi.UList, UList);
  CASE(urbi.USound, USound);
  CASE(urbi.UValue, UValue);
  CASE(void, Void);
#undef CASE
  FRAISE("Can't define a callback on a function with return type %s", retType);
}


struct MethodIdAndUrbiName
{
  jmethodID	java_mid;
  std::string	urbi_method_name;
};

static MethodIdAndUrbiName
getMethodIdAndUrbiName(JNIEnv *env,
                       jobject obj,
                       jstring obj_name,
                       jstring method_name,
                       jstring method_signature)
{
  MethodIdAndUrbiName res;
  res.java_mid = 0;

  jclass cls = env->GetObjectClass(obj);
  if (!cls)
    FRAISE("Can't find java object class");

  const char* method_name_ = env->GetStringUTFChars(method_name, 0);
  const char* method_signature_ = env->GetStringUTFChars(method_signature, 0);

  res.java_mid = env->GetMethodID(cls, method_name_, method_signature_);
  if (!res.java_mid)
    FRAISE("Can't find java callback method id");

  const char* obj_name_ = env->GetStringUTFChars(obj_name, 0);

  res.urbi_method_name = obj_name_;
  res.urbi_method_name += ".";
  res.urbi_method_name += method_name_;

  env->ReleaseStringUTFChars(method_name, method_name_);
  env->ReleaseStringUTFChars(method_signature, method_signature_);
  env->ReleaseStringUTFChars(obj_name, obj_name_);

  return res;
}


void
registerNotify(JNIEnv *env,
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

  if (arg_nb > 0)
  {
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
      FRAISE("%s is not supported as a return type for notify change",
             return_type_);
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
      FRAISE("%s is not supported as a return type for notify change",
             return_type_);
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
  if (!uob
      || !createFunctionCallback (env, uob, f, obj_name_,
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
  if ((int) arg_nb == 0)
  {
    urbi::TimerHandle th =
      (new urbi::UTimerCallbackobj<CallbacksCaller>
       (obj_name_,
        (double) period,
        f,
        boost::bind(&CallbacksCaller::callNotifyChangeInt_0, f),
        urbi::getCurrentContext()))
      ->handle_get();
    res = *th;
  }
  env->ReleaseStringUTFChars(obj_name, obj_name_);
  env->ReleaseStringUTFChars(return_type, return_type_);
  return StringConverter::staticConvert(env, res);
}
