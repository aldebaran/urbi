/*
 * Copyright (C) 2010, 2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */
#include <libport/debug.hh>
#include <jni.h>
#include "urbi_Log.h"

GD_INIT();

JNIEXPORT void JNICALL
Java_urbi_Log_info (JNIEnv *env, jobject,
                    jstring category,
                    jstring msg,
                    jstring functionname, jstring filename, jint linenumber)
{
  const char* msg_ = env->GetStringUTFChars(msg, 0);
  const char* functionname_ = env->GetStringUTFChars(functionname, 0);
  const char* filename_ = env->GetStringUTFChars(filename, 0);
  const char*category_ = env->GetStringUTFChars(category, 0);
  libport::debug::category_type _libport_gd_category =
    ::libport::debug::add_category(::libport::debug::category_type(category_));
  if (GD_DEBUGGER->enabled(::libport::Debug::levels::debug,
                           _libport_gd_category))
    GD_DEBUGGER->debug(msg_,
                       ::libport::Debug::types::info,
                       _libport_gd_category,
                       functionname_, filename_, (int) linenumber);
  env->ReleaseStringUTFChars(msg, msg_);
  env->ReleaseStringUTFChars(functionname, functionname_);
  env->ReleaseStringUTFChars(filename, filename_);
  env->ReleaseStringUTFChars(category, category_);
}

JNIEXPORT void JNICALL
Java_urbi_Log_error (JNIEnv *env, jobject,
                     jstring category,
                     jstring msg,
                     jstring functionname, jstring filename, jint linenumber)
{
  const char* msg_ = env->GetStringUTFChars(msg, 0);
  const char* functionname_ = env->GetStringUTFChars(functionname, 0);
  const char* filename_ = env->GetStringUTFChars(filename, 0);
  const char*category_ = env->GetStringUTFChars(category, 0);
  ::libport::debug::category_type _libport_gd_category =
      ::libport::debug::add_category(::libport::debug::category_type(category_));
  if (GD_DEBUGGER->enabled(::libport::Debug::levels::log,
                           _libport_gd_category))
    GD_DEBUGGER->debug(msg_,
                       ::libport::Debug::types::error,
                       _libport_gd_category,
                       functionname_, filename_, (int) linenumber);
  env->ReleaseStringUTFChars(msg, msg_);
  env->ReleaseStringUTFChars(functionname, functionname_);
  env->ReleaseStringUTFChars(filename, filename_);
  env->ReleaseStringUTFChars(category, category_);
}
