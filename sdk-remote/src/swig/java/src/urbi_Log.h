/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <jni.h>

#ifndef _Included_urbi_Log
#define _Included_urbi_Log
#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL Java_urbi_Log_info
(JNIEnv *, jobject, jstring, jstring, jstring, jstring, jint);

JNIEXPORT void JNICALL Java_urbi_Log_error
(JNIEnv *, jobject, jstring, jstring, jstring, jstring, jint);

#ifdef __cplusplus
}
#endif
#endif
