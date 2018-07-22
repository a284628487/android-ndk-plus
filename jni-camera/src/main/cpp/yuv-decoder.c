#include <jni.h>
#include <android/log.h>
#include <stdlib.h>

// 16进制FF为256，即为2进制的100000000，p & 相应的值再右移，即得到真实的 A/R/G/B 的值
#define RGBA_A(p) (((p) & 0xFF000000) >> 24)
#define RGBA_R(p) (((p) & 0x00FF0000) >> 16)
#define RGBA_G(p) (((p) & 0x0000FF00) >>  8)
#define RGBA_B(p)  ((p) & 0x000000FF)
#define MAKE_RGBA(r, g, b, a) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b))

JNIEXPORT void JNICALL
Java_com_ccflying_JNIBitmap_YUVtoRBGA(JNIEnv *env, jobject obj,
                                      jbyteArray yuv420sp, jint width, jint height,
                                      jintArray rgbOut) {
    // byte[] yuv, int width, int height, int[] out
    int sz;
    int i;
    int j;
    int Y;
    int U = 0;
    int V = 0;
    int pixPtr = 0;
    int jDiv2 = 0;
    int R = 0;
    int G = 0;
    int B = 0;
    int cOff;
    int pixel;
    //
    int w = width;
    int h = height;
    // 总像素点个数
    sz = w * h;
    // 输出结果: pixels
    jint *rgbData = (jint *) ((*env)->GetPrimitiveArrayCritical(env, rgbOut, 0));
    // Camera原始byte数据
    jbyte *yuv = (jbyte *) (*env)->GetPrimitiveArrayCritical(env, yuv420sp, 0);

    srand((int) time(NULL));
    int randNum;

    for (j = 0; j < h; j++) { // height: 从上到下
        pixPtr = j * w; // 第 j 行左侧位置索引
        jDiv2 = j >> 1; // jDiv2为j整除2
        for (i = 0; i < w; i++) { // width: 从左到右
            Y = yuv[pixPtr]; // Y 行第 i 个像素点
            if (Y < 0)
                Y += 255;
            // ? (i & 0x1) 表示 i 为单数(奇数)，1, 3, 5, 7, 9
            if ((i & 0x1) != 1) {
                cOff = sz + jDiv2 * w + (i >> 1) * 2;
                U = yuv[cOff]; // U
                V = yuv[cOff + 1]; // V
                if (U < 0)
                    U += 127;
                else
                    U -= 128;
                if (V < 0)
                    V += 127;
                else
                    V -= 128;
                // randNum = rand();
            }

            //ITU-R BT.601 conversion
            //
            //R = 1.164*(Y-16) + 2.018*(U-128);
            //G = 1.164*(Y-16) - 0.813*(V-128) - 0.391*(U-128);
            //B = 1.164*(Y-16) + 1.596*(V-128);
            //
            Y = Y + (Y >> 3) + (Y >> 5) + (Y >> 7);
            // R
            R = Y + (int) (1.4f * U);
            R = limitRGBValue(R);
            // G
            G = Y - (int) (0.344f * V + 0.714f * U);
            G = limitRGBValue(G);
            // B
            B = Y + (int) (1.77f * V);
            B = limitRGBValue(B);
            //
            pixel = 0xff000000 + (R << 16) + (G << 8) + B;
            pixPtr++;
            // 写入结果数据：
            // 1. 原始
            // rgbData[pixPtr] = pixel
            // 2. 90度旋转
            rgbData[h * i + (h - j - 1)] = pixel;
            // 3. 90度旋转+左右镜像
            // rgbData[h * i + j] = pixel;
        }
    }
    // release
    (*env)->ReleasePrimitiveArrayCritical(env, rgbOut, rgbData, 0);
    (*env)->ReleasePrimitiveArrayCritical(env, yuv420sp, yuv, 0);
}

// 灰度化处理
void Java_com_ccflying_JNIBitmap_Gray(JNIEnv *env, jobject obj,
                                      jintArray rgbData_, int w, int h, jintArray outData_) {
    //
    jint *rgbData = (jint *) ((*env)->GetPrimitiveArrayCritical(env, rgbData_,
                                                                0));
    jint *outData = (jint *) ((*env)->GetPrimitiveArrayCritical(env, outData_,
                                                                0));
    int i = 0;
    int j = 0;
    int index = 0;
    int pixel;
    int a, r, g, b;
    for (j = 0; j < h; j++) {
        for (i = 0; i < w; i++) {
            index = j * w + i;
            pixel = rgbData[index];
            a = RGBA_A(pixel);
            r = RGBA_R(pixel);
            g = RGBA_G(pixel);
            b = RGBA_B(pixel);
            // GrayScale 灰度值 (38 + 75 + 15 = 128),128 >> 7
            int gray = (r * 38 + g * 75 + b * 15) >> 7;
            outData[index] = MAKE_RGBA(gray, gray, gray, 255);
        }
    }
    (*env)->ReleasePrimitiveArrayCritical(env, rgbData_, rgbData, 0);
    (*env)->ReleasePrimitiveArrayCritical(env, outData_, outData, 0);
}

// 二值化处理
void Java_com_ccflying_JNIBitmap_BlackWhite(JNIEnv *env, jobject obj,
                                            jintArray rgbData_, int w, int h, jintArray outData_) {
    //
    jint *rgbData = (jint *) ((*env)->GetPrimitiveArrayCritical(env, rgbData_,
                                                                0));
    jint *outData = (jint *) ((*env)->GetPrimitiveArrayCritical(env, outData_,
                                                                0));
    int i = 0;
    int j = 0;
    int index = 0;
    int pixel;
    int a, r, g, b;
    int center = 136;
    for (j = 0; j < h; j++) {
        for (i = 0; i < w; i++) {
            index = j * w + i;
            pixel = rgbData[index];
            a = RGBA_A(pixel);
            r = RGBA_R(pixel);
            g = RGBA_G(pixel);
            b = RGBA_B(pixel);
            r = (r > center) ? 255 : 0;
            g = (g > center) ? 255 : 0;
            b = (b > center) ? 255 : 0;
            outData[index] = MAKE_RGBA(r, g, b, 255);
        }
    }
    (*env)->ReleasePrimitiveArrayCritical(env, rgbData_, rgbData, 0);
    (*env)->ReleasePrimitiveArrayCritical(env, outData_, outData, 0);
}

// 限制RGB值的范围为[0-255]
int limitRGBValue(int input) {
    if (input < 0)
        input = 0;
    else if (input > 255)
        input = 255;
    return input;
}
