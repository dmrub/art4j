/*
 * This file is part of art4j. It is subject to the license terms in
 * the LICENSE file found in the top-level directory of this distribution.
 * You may not use this file except in compliance with the License.
 */

#ifndef _HANDLE_H_INCLUDED_
#define _HANDLE_H_INCLUDED_
#include <jni.h>

inline jfieldID getHandleField(JNIEnv *env, jobject obj)
{
    jclass c = env->GetObjectClass(obj);
    // J is the type signature for long:
    return env->GetFieldID(c, "nativeHandle", "J");
}

template <typename T>
T *getHandle(JNIEnv *env, jobject obj)
{
    jlong handle = env->GetLongField(obj, getHandleField(env, obj));
    return reinterpret_cast<T *>(handle);
}

template <typename T>
T *handle_cast(jlong handle)
{
    return reinterpret_cast<T *>(handle);
}

template <typename T>
jlong handle_to(T *t)
{
    return reinterpret_cast<jlong>(t);
}

template <typename T>
void setHandle(JNIEnv *env, jobject obj, T *t)
{
    jlong handle = reinterpret_cast<jlong>(t);
    env->SetLongField(obj, getHandleField(env, obj), handle);
}

#endif
