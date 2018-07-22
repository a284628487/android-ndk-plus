package com.example.jni_camera;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.hardware.Camera.PreviewCallback;
import android.hardware.Camera.Size;
import android.os.Bundle;
import android.util.Log;
import android.view.TextureView;
import android.view.TextureView.SurfaceTextureListener;
import android.widget.ImageView;

import com.ccflying.JNIBitmap;

import java.io.IOException;
import java.nio.IntBuffer;

@SuppressLint("NewApi")
public class CameraActivity extends Activity implements SurfaceTextureListener {
	final String TAG = "CameraActivity";
	private TextureView mTextureView;
	private Camera mCamera;
	private ImageView iv1, iv2, iv3;

	private IntBuffer mIntBuffer, mOperaBuffer;

	private Bitmap bmp1, bmp2, bmp3;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		setContentView(R.layout.aty_camera);

		mTextureView = (TextureView) findViewById(R.id.textureView);
		mTextureView.setSurfaceTextureListener(this);
		iv1 = (ImageView) findViewById(R.id.iv1);
		iv2 = (ImageView) findViewById(R.id.iv2);
		iv3 = (ImageView) findViewById(R.id.iv3);
		//
	}

	private void openCameraAndPreview(final SurfaceTexture surface) {
		mCamera = Camera.open();
		mCamera.setDisplayOrientation(90);
		try {
			mCamera.setPreviewTexture(surface);
			mCamera.startPreview();
			final Size previewSize = mCamera.getParameters().getPreviewSize();
			Log.i(TAG, "size.width=" + previewSize.width + ";size.heigt="
					+ previewSize.height);
			final int width_ = previewSize.width;
			final int height_ = previewSize.height;
			mIntBuffer = IntBuffer.allocate(width_ * height_);
			mOperaBuffer = IntBuffer.allocate(width_ * height_);
			mCamera.setPreviewCallback(new PreviewCallback() {
				@Override
				public void onPreviewFrame(byte[] data, Camera camera) {
					if (null == mCamera)
						return;
					// data.length = 460800
					// width * height = 307200
					// width * height * 1.5 = data.length
					// 422 & 444
					JNIBitmap.YUVtoRBGA(data, width_, height_,
							mIntBuffer.array());
					int[] pixels = mIntBuffer.array();
					//
					if (null == bmp1) {
						// bmp = Bitmap.createBitmap(width_, height_,
						// Config.ARGB_8888);
						bmp1 = Bitmap.createBitmap(height_, width_,
								Config.ARGB_8888);
					}
					// bmp.setPixels(pixels, 0, width_, 0, 0, width_, height_);
					bmp1.setPixels(pixels, 0, height_, 0, 0, height_, width_);
					iv1.setImageBitmap(bmp1);
					// -------------------------------------------------
					JNIBitmap.Gray(pixels, width_, height_,
							mOperaBuffer.array());
					pixels = mOperaBuffer.array();
					if (null == bmp2) {
						bmp2 = Bitmap.createBitmap(height_, width_,
								Config.ARGB_8888);
					}
					bmp2.setPixels(pixels, 0, height_, 0, 0, height_, width_);
					iv2.setImageBitmap(bmp2);
					// -------------------------------------------------
					JNIBitmap.BlackWhite(pixels, width_, height_,
							mOperaBuffer.array());
					pixels = mOperaBuffer.array();
					if (null == bmp3) {
						bmp3 = Bitmap.createBitmap(height_, width_,
								Config.ARGB_8888);
					}
					bmp3.setPixels(pixels, 0, height_, 0, 0, height_, width_);
					iv3.setImageBitmap(bmp3);
					//
					camera.addCallbackBuffer(data);
				}
			});
		} catch (IOException ioe) {
			// Something bad happened
		}
	}

	private void stopCamera() {
		if (null == mCamera) {
			return;
		}
		mCamera.setPreviewCallback(null);
		mCamera.stopPreview();
		mCamera.release();
		mCamera = null;
	}

	@Override
	public void onSurfaceTextureAvailable(SurfaceTexture surface, int width,
			int height) {
		openCameraAndPreview(surface);
	}

	@Override
	public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width,
			int height) {
	}

	@Override
	public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
		stopCamera();
		return true;
	}

	@Override
	public void onSurfaceTextureUpdated(SurfaceTexture surface) {
	}

}

// https://blog.csdn.net/oShunz/article/details/49862273
// https://blog.csdn.net/ShareUs/article/details/52755809
// https://blog.csdn.net/oShunz/article/details/50537631
// https://blog.csdn.net/oShunz/article/details/50484997
