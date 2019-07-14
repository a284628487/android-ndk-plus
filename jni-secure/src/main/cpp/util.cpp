//
// Created by ccfyyn on 19/7/14.
//

#include "util.h"
#include <string>

char *jbyteArrayToChars(JNIEnv *env, jbyteArray byteArray) {
    jsize strLen = env->GetArrayLength(byteArray);
    jbyte *jBuf = env->GetByteArrayElements(byteArray, JNI_FALSE);
    char *pStr = NULL;
    if (jBuf > 0) {
        pStr = (char *) malloc(strLen + 1);
        if (!pStr) {
            return NULL;
        }

        memcpy(pStr, jBuf, strLen);
        pStr[strLen] = 0;
    }
    env->ReleaseByteArrayElements(byteArray, jBuf, 0);
    return pStr;
}

char *jstringToChars(JNIEnv *env, jstring string) {

    jclass jstrObj = env->FindClass("java/lang/String");
    jstring encode = env->NewStringUTF("utf-8");
    jmethodID methodId = env->GetMethodID(jstrObj, "getBytes", "(Ljava/lang/String;)[B");
    jbyteArray byteArray = (jbyteArray) env->CallObjectMethod(string, methodId, encode);

    return jbyteArrayToChars(env, byteArray);
}

jobject getApplicationContext(JNIEnv *env) {
    // 获取Activity Thread的实例对象
    jclass activityThread = env->FindClass("android/app/ActivityThread");
    jmethodID currentActivityThread = env->GetStaticMethodID(activityThread,
                                                             "currentActivityThread",
                                                             "()Landroid/app/ActivityThread;");
    jobject at = env->CallStaticObjectMethod(activityThread, currentActivityThread);
    // 获取Application，也就是全局的Context
    jmethodID getApplication = env->GetMethodID(activityThread, "getApplication",
                                                "()Landroid/app/Application;");
    jobject context = env->CallObjectMethod(at, getApplication);
    return context;
}
