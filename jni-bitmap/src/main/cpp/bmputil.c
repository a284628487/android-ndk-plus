#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <jni.h>
#include <android/bitmap.h>
#include <android/log.h>

#ifndef eprintf
#define eprintf(...) __android_log_print(ANDROID_LOG_ERROR, "@", __VA_ARGS__)
#endif

#define RGB565_R(p) ((((p) & 0xF800) >> 11) << 3)
#define RGB565_G(p) ((((p) & 0x7E0 ) >> 5)  << 2)
#define RGB565_B(p) ( ((p) & 0x1F  )        << 3)
#define MAKE_RGB565(r,g,b) ((((r) >> 3) << 11) | (((g) >> 2) << 5) | ((b) >> 3))

// 16进制FF为256，即为2进制的100000000，p & 相应的值再右移，即得到真实的 A/R/G/B 的值
#define RGBA_A(p) (((p) & 0xFF000000) >> 24)
#define RGBA_R(p) (((p) & 0x00FF0000) >> 16)
#define RGBA_G(p) (((p) & 0x0000FF00) >>  8)
#define RGBA_B(p)  ((p) & 0x000000FF)
#define MAKE_RGBA(r,g,b,a) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b))

void JNICALL Java_com_example_jni_bitmap_Bmp_huidu(JNIEnv *env, jclass clazz, jobject zBitmap) {
    JNIEnv J = *env;

    if (zBitmap == NULL) {
        eprintf("bitmap is null\n");
        return;
    }

    // Get bitmap info
    AndroidBitmapInfo info;
    // 清空某一段连续内存，设置为0
    memset(&info, 0, sizeof(info));
    // 获取图片信息
    AndroidBitmap_getInfo(env, zBitmap, &info);
    // Check format, only RGB565 & RGBA are supported
    if (info.width <= 0 || info.height <= 0 ||
        (info.format != ANDROID_BITMAP_FORMAT_RGB_565 && info.format != ANDROID_BITMAP_FORMAT_RGBA_8888)) {
        eprintf("invalid bitmap\n");
        J->ThrowNew(env, J->FindClass(env, "java/io/IOException"), "invalid bitmap");
        return;
    }

    // Lock the bitmap to get the buffer
    void * pixels = NULL;
    int res = AndroidBitmap_lockPixels(env, zBitmap, &pixels);
    if (pixels == NULL) {
        eprintf("fail to lock bitmap: %d\n", res);
        J->ThrowNew(env, J->FindClass(env, "java/io/IOException"), "fail to open bitmap");
        return;
    }

    eprintf("Effect: %dx%d, %d\n", info.width, info.height, info.format);

    int x = 0, y = 0;
    // From top to bottom
    for (y = 0; y < info.height; ++y) {
        // From left to right
        for (x = 0; x < info.width; ++x) {
            int a = 0, r = 0, g = 0, b = 0;
            void *pixel = NULL;
            // Get each pixel by format
            if (info.format == ANDROID_BITMAP_FORMAT_RGB_565) {
                pixel = ((uint16_t *)pixels) + y * info.width + x;
                uint16_t v = *(uint16_t *)pixel;
                r = RGB565_R(v);
                g = RGB565_G(v);
                b = RGB565_B(v);
            } else {// RGBA
                pixel = ((uint32_t *)pixels) + y * info.width + x;
                uint32_t v = *(uint32_t *)pixel;
                a = RGBA_A(v);
                r = RGBA_R(v);
                g = RGBA_G(v);
                b = RGBA_B(v);
            }

            // GrayScale 灰度值 (38 + 75 + 15 = 128),128 >> 7
            int gray = (r * 38 + g * 75 + b * 15) >> 7;
            // 二值化
            int center = 128;
            r = (r > 128) ? 255 : 0;
            g = (g > 128) ? 255 : 0;
            b = (b > 128) ? 255 : 0;

            // Write the pixel back
            if (info.format == ANDROID_BITMAP_FORMAT_RGB_565) {
                *((uint16_t *)pixel) = MAKE_RGB565(gray, gray, gray);
            } else {// RGBA
                // uint32_t v = MAKE_RGBA(gray, gray, gray, a); // 灰度处理
            	uint32_t v = MAKE_RGBA(r, g, b, a); // 二值化处理
            	*((uint32_t *)pixel) = v;
            }
        }
    }

    AndroidBitmap_unlockPixels(env, zBitmap);
}

// https://blog.csdn.net/chzphoenix/article/details/42171653
/**
	Bitmap.Config的四种枚举类型。(ARGB分别是alpha透明度和red、green、blue三色)
	ARGB_8888：分别用8位来记录4个值，所以每个像素会占用32位。
	ARGB_4444：分别用4位来记录4个值，所以每个像素会占用16位。
	RGB_565：分别用5位、6位和5位来记录RGB三色值，所以每个像素会占用16位。
	ALPHA_8：根据注释应该是不保存颜色值，只保存透明度（8位），每个像素会占用8位。
*/

/**
	a) 用 AndroidBitmap_getInfo() 函数从位图句柄（从JNI得到）获得信息（宽度、高度、像素格式）
	b) 用 AndroidBitmap_lockPixels() 对像素缓存上锁，即获得该缓存的指针。
	c) 用C/C++ 对这个缓冲区进行读写
	d) 用 AndroidBitmap_unlockPixels() 解锁
 */
