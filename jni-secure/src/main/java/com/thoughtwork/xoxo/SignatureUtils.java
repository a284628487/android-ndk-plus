package com.thoughtwork.xoxo;

import android.app.Application;

public class SignatureUtils {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public static native String stringFromJNI();

    public static native String getSecureKey();

    public static native String getSignatureByType(String type);

    public static native Application getApplicationFromNative();
}
