package com.ccflying;

public class JNIBitmap {

	public static native void YUVtoRBGA(byte[] yuv, int width, int height,
			int[] out);

	public static native void Gray(int[] src, int width, int height, int[] out);

	public static native void BlackWhite(int[] src, int width, int height,
			int[] out);

	static {
		System.loadLibrary("gpuimage");
	}
}
