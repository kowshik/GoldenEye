/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.android.goldeneye;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.List;
import java.util.Random;

import android.app.Activity;
import android.content.pm.ActivityInfo;
import android.graphics.Bitmap;
import android.graphics.Bitmap.CompressFormat;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.hardware.Camera;
import android.hardware.Camera.Size;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.Window;
import android.widget.Button;

import com.android.goldeneye.core.GoldenEye;

// ----------------------------------------------------------------------

public class StartScreen extends Activity implements SurfaceHolder.Callback,
		OnClickListener {
	private static final String INPUT_FILE = GoldenEyeConstants.FACEDETECTION_FOLDER
			+ "/test.jpg";
	private static final String OUTPUT_FILE = GoldenEyeConstants.FACEDETECTION_FOLDER
			+ "/test-withfaces.jpg";

	private SurfaceView iSurfaceView;
	SurfaceHolder mHolder;
	Camera mCamera;
	private Button btn;

	private void initGoldenEye() {
		Log.i(GoldenEyeConstants.LOG_TAG, "initializing GoldenEye");
		if (Environment.MEDIA_MOUNTED.equals(Environment
				.getExternalStorageState())) {
			if (!(createPath("base path", GoldenEyeConstants.BASE_PATH) && createPath(
					"face recognition tmp folder",
					GoldenEyeConstants.FACEDETECTION_FOLDER))) {
				finish();
			}

			Log.i(GoldenEyeConstants.LOG_TAG, "attempting to create : "
					+ GoldenEyeConstants.LOCAL_HAAR_CLASSIFIER_XML);
			File localHaarXmlFile = new File(
					GoldenEyeConstants.LOCAL_HAAR_CLASSIFIER_XML);

			InputStream haarXmlReader = this.getResources().openRawResource(
					GoldenEyeConstants.HAAR_CLASSIFIER_XML);
			try {
				OutputStream haarXmlWriter = new FileOutputStream(
						localHaarXmlFile);

				int aByte = -1;
				while ((aByte = haarXmlReader.read()) != -1) {
					haarXmlWriter.write(aByte);
				}
			} catch (IOException e) {
				Log.e(GoldenEyeConstants.LOG_TAG,
						"IOException in StartScreen.initGoldenEye() while manipulating Haar Classifier XML : "
								+ GoldenEyeConstants.LOCAL_HAAR_CLASSIFIER_XML
								+ " -> " + e.getStackTrace());
				finish();
			}
		} else {
			Log.e(GoldenEyeConstants.LOG_TAG,
					"Externam storage not mounted .. dying");
			finish();
		}
		Log.i(GoldenEyeConstants.LOG_TAG, "initialized GoldenEye !");

	}

	private boolean createPath(String pathDescription, String path) {
		Log.i(GoldenEyeConstants.LOG_TAG, "attempting to create "
				+ pathDescription + " : " + path);
		File f = new File(path);
		if (f.exists()) {
			Log.i(GoldenEyeConstants.LOG_TAG, pathDescription
					+ " already exists");
			return true;
		}
		if (f.mkdirs()) {
			Log.i(GoldenEyeConstants.LOG_TAG, "successfully created : "
					+ pathDescription);
		}
		return false;
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		this.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
		setContentView(R.layout.main);

		initGoldenEye();

		btn = (Button) findViewById(R.id.btn);
		btn.setOnClickListener(this);
		iSurfaceView = (SurfaceView) findViewById(R.id.surface_view);

		mHolder = iSurfaceView.getHolder();
		mHolder.addCallback(this);
		mHolder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);

	}

	@Override
	protected void onDestroy() {
		super.onDestroy();/*
						 * File f = new File(localHaarXmlPath); if (f.delete())
						 * { Log.e("StartScreen", "Temporary file : " +
						 * localHaarXmlPath + " could not be deleted on exit");
						 * }
						 */
	}

	public void surfaceCreated(SurfaceHolder holder) {
		// The Surface has been created, acquire the camera and tell it where
		// to draw.

		mCamera = Camera.open();

		// mCamera.setPreviewCallback(iPreviewCallback);
		try {
			mCamera.setPreviewDisplay(holder);
			Camera.Parameters params = mCamera.getParameters();
			params.setPictureSize(500, 500);

			mCamera.setParameters(params);

		} catch (IOException exception) {
			mCamera.release();
			mCamera = null;
			// TODO: add more exception handling logic here
		}
	}

	public void surfaceDestroyed(SurfaceHolder holder) {
		// Surface will be destroyed when we return, so stop the preview.
		// Because the CameraDevice object is not a shared resource, it's very
		// important to release it when the activity is paused.
		mCamera.stopPreview();
		mCamera.release();
		mCamera = null;
	}

	private Size getOptimalPreviewSize(List<Size> sizes, int w, int h) {
		final double ASPECT_TOLERANCE = 0.05;
		double targetRatio = (double) w / h;
		if (sizes == null)
			return null;

		Size optimalSize = null;
		double minDiff = Double.MAX_VALUE;

		int targetHeight = h;

		// Try to find an size match aspect ratio and size
		for (Size size : sizes) {
			double ratio = (double) size.width / size.height;
			if (Math.abs(ratio - targetRatio) > ASPECT_TOLERANCE)
				continue;
			if (Math.abs(size.height - targetHeight) < minDiff) {
				optimalSize = size;
				minDiff = Math.abs(size.height - targetHeight);
			}
		}

		// Cannot find the one match the aspect ratio, ignore the requirement
		if (optimalSize == null) {
			minDiff = Double.MAX_VALUE;
			for (Size size : sizes) {
				if (Math.abs(size.height - targetHeight) < minDiff) {
					optimalSize = size;
					minDiff = Math.abs(size.height - targetHeight);
				}
			}
		}
		return optimalSize;
	}

	public void surfaceChanged(SurfaceHolder holder, int format, int w, int h) {

		// Now that the size is known, set up the camera parameters and begin
		// the preview.
		Log.i("SurfaceChanged", "changed : " + new Random().nextInt());

		Camera.Parameters parameters = mCamera.getParameters();

		List<Size> sizes = parameters.getSupportedPreviewSizes();
		Size optimalSize = getOptimalPreviewSize(sizes, w, h);
		parameters.setPreviewSize(optimalSize.width, optimalSize.height);

		mCamera.setParameters(parameters);

		mCamera.startPreview();

	}

	public void onClick(View v) {
		mCamera.takePicture(null, null, null, iPictureCallback);

	}

	Camera.PictureCallback iPictureCallback = new Camera.PictureCallback() {

		public void onPictureTaken(byte[] data, Camera camera) {

			Log.i(GoldenEyeConstants.LOG_TAG, "inside onPictureTake()");
			Bitmap bitmap = BitmapFactory.decodeByteArray(data, 0, data.length);
			int width = bitmap.getWidth();
			int height = bitmap.getHeight();

			float scaleWidth = ((float) GoldenEyeConstants.FR_INP_WIDTH)
					/ width;
			float scaleHeight = ((float) GoldenEyeConstants.FR_INP_HEIGHT)
					/ height;
			Matrix matrix = new Matrix();
			matrix.postScale(scaleWidth, scaleHeight);
			Bitmap resizedBitmap = Bitmap.createBitmap(bitmap, 0, 0, width,
					height, matrix, true);

			String fileName = String.format(INPUT_FILE,
					System.currentTimeMillis());
			FileOutputStream outStream = null;
			try {
				outStream = new FileOutputStream(fileName);
				resizedBitmap.compress(CompressFormat.JPEG, 80, outStream);
				outStream.close();
			} catch (FileNotFoundException e) {
				e.printStackTrace();
			} catch (IOException e) {
				e.printStackTrace();
			} finally {
			}

			Log.i(GoldenEyeConstants.LOG_TAG,
					"attempting opencv call to detect faces");
			int detectFaces = GoldenEye.detectFaces(INPUT_FILE, OUTPUT_FILE,
					GoldenEyeConstants.LOCAL_HAAR_CLASSIFIER_XML);
			if (detectFaces == 0) {
				Log.e(GoldenEyeConstants.LOG_TAG, "unable to detect face");
			}

			mCamera.startPreview();

		}
	};

}
