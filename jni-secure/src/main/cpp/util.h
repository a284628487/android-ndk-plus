//
// Created by ccfyyn on 19/7/14.
//

#ifndef XOXO_UTIL_H
#define XOXO_UTIL_H
#include <jni.h>

char *jbyteArrayToChars(JNIEnv *env, jbyteArray byteArray);

jobject getApplicationContext(JNIEnv *env);

#endif //XOXO_UTIL_H
