package com.ccflying.jnibitmap;

import android.graphics.Bitmap;

public class Bmp {

    static {
        System.loadLibrary("bmputil");
    }

    public static native void huidu(Bitmap bmp);
}
