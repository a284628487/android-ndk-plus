## jni-camera

Convert camera's raw data ( yuv ) to argb，and add some other effects and output in-time.
在`Java`层的 `PreviewCallback` 方法中，将byte类型的yuv数据，转换成argb格式的图片。

## Code

Java代码：

```Java
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
```

C代码：
```C
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
```

[Link](https://docs.microsoft.com/en-us/previous-versions/windows/embedded/ee490095(v=winembedded.60))

## ScreenShot

![ss](./screenshot.jpg)

## 原理及作用
YUV的原理是把亮度与色度分离，研究证明,人眼对亮度的敏感超过色度。利用这个原理，可以把色度信息减少一点，人眼也无法查觉这一点。
YUV主要用于优化彩色视频信号的传输，使其向后相容老式黑白电视。与RGB视频信号传输相比，它最大的优点在于只需占用极少的频宽（RGB要求三个独立的视频信号同时传输）。
其中“Y”表示明亮度（Luminance或Luma），也就是灰阶值；而“U”和“V” 表示的则是色度（Chrominance或Chroma），作用是描述影像色彩及饱和度，用于指定像素的颜色。
“亮度”是透过RGB输入信号来建立的，方法是将RGB信号的特定部分叠加到一起。
“色度”则定义了颜色的两个方面─色调与饱和度，分别用Cr和Cb来表示。

- Cr反映的是RGB输入信号红色部分与RGB信号亮度值之间的差异。
- Cb反映的是RGB输入信号蓝色部分与RGB信号亮度值之间的差异。

采用YUV色彩空间的重要性是它的亮度信号Y和色度信号U、V是分离的。如果只有Y信号分量而没有U、V分量，那么这样表示的图像就是黑白灰度图像。彩色电视采用YUV空间正是为了用亮度信号Y解决彩色电视机与黑白电视机的兼容问题，使黑白电视机也能接收彩色电视信号。

## YUV格式与RGB格式 转换
关于YUV与RGB转换，是一套标准公式

### RGB -> YUV
```Java
Y = (0.257 * R) + (0.504 * G) + (0.098 * B) + 16
Cr = V = (0.439 * R) - (0.368 * G) - (0.071 * B) + 128
Cb = U = -( 0.148 * R) - (0.291 * G) + (0.439 * B) + 128
```
简化公式
```
Y = 0.299R + 0.587G + 0.114B
U'= (BY)*0.565
V'= (RY)*0.713
```

### YUV -> RGB
```Java
B = 1.164(Y - 16) + 2.018(U - 128)
G = 1.164(Y - 16) - 0.813(V - 128) - 0.391(U - 128)
R = 1.164(Y - 16) + 1.596(V - 128)
```
简化公式
```
R = Y + 1.403U'
G = Y - 0.344V' - 0.714U'
B = Y + 1.770V'
```
> 如果只有Y信号分量而没有U、V分量，那么这样表示的图像就是黑白灰度图像。因此用YUV格式由彩色转黑白信号相当简单.

## YCbCr

YUV经常有另外的名字, YCbCr ,其中Y与YUV 中的Y含义一致，Cb , Cr 同样都指色彩，只是在表示方法上不同而已，Cb Cr 就是本来理论上的“分量/色差”的标识。C代表分量(是component的缩写)Cr、Cb分别对应r(红)、b(蓝)分量信号，Y除了g(绿)分量信号，还叠加了亮度信号。

还有一种格式是YPbPr格式,它与YCbPr格式的区别在于,其中YCbCr是隔行信号，YPbPr是逐行信号。

数字信号都是YCbCr，其应用领域很广泛，JPEG、MPEG均采用此格式。在后文中，如无特别指明，讲的YUV都是指YCbCr格式。

## 采样格式
主要的采样格式有`YCbCr 4:2:0`、`YCbCr 4:2:2`、`YCbCr 4:1:1`和`YCbCr 4:4:4`。其中`YCbCr 4:1:1`比较常用，其含义为：每个点保存一个 8bit 的亮度值(也就是Y值)，每 2 x 2 个点保存一个Cr和Cb值，图像在肉眼中的感觉不会起太大的变化。所以，原来用 RGB(R，G，B 都是 8bit unsigned) 模型，每个点需要 8x3=24 bits，而现在仅需要 8+(8/4)+(8/4)=12bits，平均每个点占12bits。这样就把图像的数据压缩了一半。
上边仅给出了理论上的示例，在实际数据存储中是有可能是不同的，下面给出几种具体的存储形式：
### YUV 4:4:4
YUV三个信道的抽样率相同，因此在生成的图像里，每个象素的三个分量信息完整（每个分量通常8比特），经过8比特量化之后，未经压缩的每个像素占用3个字节。
四个像素点：`[Y0 U0 V0] [Y1 U1 V1] [Y2 U2 V2] [Y3 U3 V3]`
存放的码流：`Y0 U0 V0 Y1 U1 V1 Y2 U2 V2 Y3 U3 V3`
映射像素点：`[Y0 U0 V0] [Y1 U1 V1] [Y2 U2 V2] [Y3 U3 V3]`

### YUV 4:2:2
每个色差信道的抽样率是亮度信道的一半，所以水平方向的色度抽样率只是4:4:4的一半。对非压缩的8比特量化的图像来说，每个由两个水平方向相邻的像素组成的宏像素需要占用4字节内存(例如下面映射出的前两个像素点只需要Y0、Y1、U0、V1四个字节)。
四个像素点：`[Y0 U0 V0] [Y1 U1 V1] [Y2 U2 V2] [Y3 U3 V3]`
存放的码流：`Y0 U0 Y1 V1 Y2 U2 Y3 V3`
映射像素点：`[Y0 U0 V1] [Y1 U0 V1] [Y2 U2 V3] [Y3 U2 V3]`

### YUV 4:1:1
4:1:1的色度抽样，是在水平方向上对色度进行4:1抽样。对于低端用户和消费类产品这仍然是可以接受的。对非压缩的8比特量化的视频来说，每个由4个水平方向相邻的像素组成的宏像素需要占用6字节内存
四个像素点：`[Y0 U0 V0] [Y1 U1 V1] [Y2 U2 V2] [Y3 U3 V3]`
存放的码流：`Y0 U0 Y1 Y2 V2 Y3`
映射像素点：`[Y0 U0 V2] [Y1 U0 V2] [Y2 U0 V2] [Y3 U0 V2]`

### YUV4:2:0
4:2:0并不意味着只有Y，Cb而没有Cr分量。它指得是对每行扫描线来说，只有一种色度分量以2:1的抽样率存储。相邻的扫描行存储不同的色度分量，也就是说，如果一行是4:2:0的话，下一行就是4:0:2，再下一行是4:2:0...以此类推。对每个色度分量来说，水平方向和竖直方向的抽样率都是2：1，所以可以说色度的抽样率是4:1。对非压缩的8比特量化的视频来说，每个由2x2个2行2列相邻的像素组成的宏像素需要占用6字节内存。

八个像素点：
```
[Y0 U0 V0] [Y1 U1 V1] [Y2 U2 V2] [Y3 U3 V3]
[Y5 U5 V5] [Y6 U6 V6] [Y7U7 V7] [Y8 U8 V8]
```
存放的码流：
```
Y0 U0 Y1 Y2 U2 Y3
Y5 V5 Y6 Y7 V7 Y8
```
映射像素点：
```
[Y0 U0 V5] [Y1 U0 V5] [Y2 U2 V7] [Y3 U2 V7]
[Y5 U0 V5] [Y6 U0 V5] [Y7U2 V7] [Y8 U2 V7]
```