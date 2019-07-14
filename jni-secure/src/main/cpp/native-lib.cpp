#include <jni.h>
#include <string>
#include "util.h"

const char *SECRET_KEY_MSR = "com.thoughtworks.com.msr.appkey";

char *getSHA1BySignature(JNIEnv *env, jobject signature, jstring type) {
    jclass signatureCls = env->GetObjectClass(signature);
    jmethodID toByteArrayMID = env->GetMethodID(signatureCls, "toByteArray", "()[B");
    jbyteArray bytes = (jbyteArray) (env->CallObjectMethod(signature, toByteArrayMID));

    jclass mdCls = env->FindClass("java/security/MessageDigest");
    jmethodID mdInstanceMID = env->GetStaticMethodID(mdCls, "getInstance",
                                                     "(Ljava/lang/String;)Ljava/security/MessageDigest;");
    jobject digest = env->CallStaticObjectMethod(mdCls, mdInstanceMID, type);

    jmethodID digestMID = env->GetMethodID(mdCls, "digest", "([B)[B");
    jbyteArray digestBytes = (jbyteArray) (env->CallObjectMethod(digest, digestMID, bytes));

    char *pStr = jbyteArrayToChars(env, digestBytes);
    jsize strLen = env->GetArrayLength(digestBytes);

    jclass integerCls = env->FindClass("java/lang/Integer");
    jmethodID toHexStrMID = env->GetStaticMethodID(integerCls, "toHexString",
                                                   "(I)Ljava/lang/String;");
    jclass stringCls = env->FindClass("java/lang/String");
    jmethodID subStringMID = env->GetMethodID(stringCls, "substring", "(II)Ljava/lang/String;");
    jmethodID concatMID = env->GetMethodID(stringCls, "concat",
                                           "(Ljava/lang/String;)Ljava/lang/String;");

    jstring jstringResult = env->NewStringUTF("");

    for (int i = 0; i < strLen; ++i) {
        int j = (pStr[i] & 0xFF) | 0x100;
        jstring hexStr = (jstring) env->CallStaticObjectMethod(integerCls, toHexStrMID, j);
        jstring hexSubStr = (jstring) env->CallObjectMethod(hexStr, subStringMID, 1, 3);
        jstringResult = (jstring) env->CallObjectMethod(jstringResult, concatMID, hexSubStr);
    }

    return (char *) env->GetStringUTFChars(jstringResult, JNI_FALSE);
}

jobject getSignature(JNIEnv *env) {
    const char *applicationName = "com/thoughtwork/xoxo/app/PrivacyApplication";
    const char *instanceFieldSig = "Lcom/thoughtwork/xoxo/app/PrivacyApplication;";

    jclass applicationCls = env->FindClass(applicationName);
    jfieldID instanceFieldId = env->GetStaticFieldID(applicationCls, "instance", instanceFieldSig);
    jobject applicationObj = env->GetStaticObjectField(applicationCls, instanceFieldId);

    jmethodID getPMgrMID = env->GetMethodID(env->FindClass("android/app/Application"),
                                            "getPackageManager",
                                            "()Landroid/content/pm/PackageManager;");
    jobject pmgrObj = env->CallObjectMethod(applicationObj, getPMgrMID);

    jmethodID pkgInfoMID = env->GetMethodID(env->GetObjectClass(pmgrObj), "getPackageInfo",
                                            "(Ljava/lang/String;I)Landroid/content/pm/PackageInfo;");
    jstring arg1 = env->NewStringUTF("com.thoughtwork.xoxo");
    jobject pkgInfo = env->CallObjectMethod(pmgrObj, pkgInfoMID, arg1, 64);

    jfieldID signaturesFID = env->GetFieldID(env->GetObjectClass(pkgInfo), "signatures",
                                             "[Landroid/content/pm/Signature;");
    jobjectArray signatures = (jobjectArray) env->GetObjectField(pkgInfo, signaturesFID);
    jobject signature = env->GetObjectArrayElement(signatures, 0);

    return signature;
}

extern "C" JNIEXPORT jstring

JNICALL
Java_com_thoughtwork_xoxo_SignatureUtils_stringFromJNI(JNIEnv *env, jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_thoughtwork_xoxo_SignatureUtils_getSecureKey(JNIEnv *env, jobject /* this */) {
    const char *applicationName = "com/thoughtwork/xoxo/app/PrivacyApplication";
    const char *instanceFieldSig = "Lcom/thoughtwork/xoxo/app/PrivacyApplication;";
    const char *validProcess = "com.thoughtwork.xoxo";

    jclass applicationCls = env->FindClass(applicationName);
    jfieldID instanceFieldId = env->GetStaticFieldID(applicationCls, "instance", instanceFieldSig);
    jobject applicationObj = env->GetStaticObjectField(applicationCls, instanceFieldId);

    const char *getAppInfoSig = "()Landroid/content/pm/ApplicationInfo;";
    jmethodID getAppInfoMethodId = env->GetMethodID(env->FindClass("android/app/Application"),
                                                    "getApplicationInfo",
                                                    getAppInfoSig);
    jobject appInfoObj = env->CallObjectMethod(applicationObj, getAppInfoMethodId);

    jclass appInfoCls = env->FindClass("android/content/pm/ApplicationInfo");
    jfieldID appInfoProcessNameFieldId = env->GetFieldID(appInfoCls, "processName",
                                                         "Ljava/lang/String;");
    jobject processName = env->GetObjectField(appInfoObj, appInfoProcessNameFieldId);

    const char *processChars = env->GetStringUTFChars((jstring) processName, JNI_FALSE);

    if (strcasecmp(processChars, validProcess) == 0) {
        return env->NewStringUTF(SECRET_KEY_MSR);
    } else {
        return env->NewStringUTF("0");
    }
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_thoughtwork_xoxo_SignatureUtils_getSignatureByType(JNIEnv *env, jclass /**/,
                                                            jstring type_) {
    // const char *type = env->GetStringUTFChars(type_, 0);
    // env->ReleaseStringUTFChars(type_, type);
    jobject signature = getSignature(env);
    return env->NewStringUTF(getSHA1BySignature(env, signature, type_));
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_thoughtwork_xoxo_SignatureUtils_getApplicationFromNative(JNIEnv *env, jclass type) {
    return getApplicationContext(env);
}
