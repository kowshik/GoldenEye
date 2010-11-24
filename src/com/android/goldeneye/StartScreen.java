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

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.List;
import java.util.Random;
import java.util.Vector;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.DialogInterface.OnDismissListener;
import android.content.pm.ActivityInfo;
import android.graphics.Bitmap;
import android.graphics.Bitmap.CompressFormat;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.hardware.Camera;
import android.hardware.Camera.Size;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;

import com.android.goldeneye.core.GoldenEye;

// ----------------------------------------------------------------------

public class StartScreen extends Activity implements SurfaceHolder.Callback,
		OnClickListener {
	private static final String INPUT_FILE = GoldenEyeConstants.FACEDETECTION_FOLDER
			+ "/fr-input.jpg";
	private static final String OUTPUT_FILE = GoldenEyeConstants.FACEDETECTION_FOLDER
			+ "/fr-output.jpg";

	private static final String TRAIN_FILE_PREFIX = GoldenEyeConstants.FACEDETECTION_FOLDER
			+ "/train-";

	private ImageView imageView;
	private SurfaceView iSurfaceView;
	private SurfaceHolder mHolder;
	private Camera mCamera;
	private Button btnSnap;
	private Button btnTrain;

	private boolean pictureTaken;
	private Handler snapTimerHandler;
	private SnapTimerThread snapTimerThread;
	private TrainTimerThread trainTimerThread;
	private Handler trainTimerHandler;
	private NameInputDialogListener nameInputListener;
	private transient String personName;
	private boolean trainTimerHandlerFlag;

	private class TrainTimerThread implements Runnable {

		private boolean timerStatus;
		private Handler trainTimerHandler;
		private Thread t;

		public TrainTimerThread(Handler trainTimerHandler) {
			this.trainTimerHandler = trainTimerHandler;
			t = new Thread(this);
			t.start();
		}

		public void startTimer() {
			timerStatus = true;
		}

		public void stopTimer() {
			timerStatus = false;
		}

		public void run() {
			while (true) {

				for (int snapCount = 1; snapCount <= GoldenEyeConstants.TRAINING_SNAP_COUNT
						&& timerStatus; snapCount++) {
					for (int timer = GoldenEyeConstants.TRAINING_TIMER; timer >= 0; timer--) {

						Message msg = trainTimerHandler.obtainMessage();
						Bundle aBundle = new Bundle();
						aBundle.putIntArray(GoldenEyeConstants.TRAIN_TIMER_KEY,
								new int[] { snapCount, timer });
						msg.setData(aBundle);
						trainTimerHandlerFlag = false;
						trainTimerHandler.sendMessage(msg);
						try {
							Thread.sleep(1000);
						} catch (InterruptedException e) {
							e.printStackTrace();
						}
						while (!trainTimerHandlerFlag) {

						}

					}
					if (snapCount == GoldenEyeConstants.TRAINING_SNAP_COUNT) {
						stopTimer();
					}
				}

			}

		}

	}

	private class SnapTimerThread implements Runnable {

		private boolean timerStatus;
		private Handler snapTimerHandler;
		private Thread t;

		public SnapTimerThread(Handler snapTimerHandler) {
			this.snapTimerHandler = snapTimerHandler;
			t = new Thread(this);
			t.start();
		}

		public void startTimer() {
			timerStatus = true;
		}

		public void stopTimer() {
			timerStatus = false;
		}

		public void run() {
			while (true) {

				for (int i = GoldenEyeConstants.TRAINING_TIMER; i >= 0
						&& timerStatus; i--) {

					Message msg = snapTimerHandler.obtainMessage();
					Bundle aBundle = new Bundle();
					aBundle.putInt(GoldenEyeConstants.SNAP_TIMER_KEY, i);
					msg.setData(aBundle);
					snapTimerHandler.sendMessage(msg);
					try {
						Thread.sleep(1000);
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
					if (i == 0) {
						stopTimer();
					}
				}

			}

		}

	}

	private void initGoldenEye() {
		pictureTaken = false;

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
			if (!localHaarXmlFile.exists()) {
				Log.i(GoldenEyeConstants.LOG_TAG, "local haar xml doesnt exist");
				BufferedInputStream haarXmlReader = new BufferedInputStream(
						this.getResources().openRawResource(
								GoldenEyeConstants.HAAR_CLASSIFIER_XML));
				Log.i(GoldenEyeConstants.LOG_TAG,
						"opened file handle for reading local haar xml");
				try {

					BufferedOutputStream haarXmlWriter = new BufferedOutputStream(
							new FileOutputStream(localHaarXmlFile));
					Log.i(GoldenEyeConstants.LOG_TAG,
							"opened file handle for writing local haar xml");
					byte[] buffer = new byte[10000];
					int numBytes = -1;
					while ((numBytes = haarXmlReader.read(buffer)) != -1) {
						haarXmlWriter.write(buffer, 0, numBytes);
					}
					haarXmlReader.close();
					haarXmlWriter.close();
					Log.i(GoldenEyeConstants.LOG_TAG, "closed haar xml handles");
				} catch (IOException e) {
					Log.e(GoldenEyeConstants.LOG_TAG,
							"IOException in StartScreen.initGoldenEye() while manipulating Haar Classifier XML : "
									+ GoldenEyeConstants.LOCAL_HAAR_CLASSIFIER_XML
									+ " -> " + e.getStackTrace());
					finish();
				}
			} else {
				Log.i(GoldenEyeConstants.LOG_TAG,
						"skipping creation of existing file : "
								+ GoldenEyeConstants.LOCAL_HAAR_CLASSIFIER_XML);
			}
		} else {
			Log.e(GoldenEyeConstants.LOG_TAG,
					"External storage not mounted .. dying");
			finish();
		}

		// -------------
		GoldenEye.init(GoldenEyeConstants.LOCAL_HAAR_CLASSIFIER_XML,
				GoldenEyeConstants.FACEDETECTION_FOLDER,
				GoldenEyeConstants.IMG_EXTENSION);
		// -------------

		snapTimerHandler = new Handler() {
			public void handleMessage(Message msg) {
				int timerValue = msg.getData().getInt(
						GoldenEyeConstants.SNAP_TIMER_KEY);

				if (timerValue == 0) {
					btnSnap.setText("...");
					mCamera.takePicture(null, null, null, snapCallback);
					return;
				}
				btnSnap.setText("" + timerValue);

			}
		};

		snapTimerThread = new SnapTimerThread(snapTimerHandler);

		trainTimerHandler = new Handler() {
			public void handleMessage(Message msg) {
				int[] values = msg.getData().getIntArray(
						GoldenEyeConstants.TRAIN_TIMER_KEY);
				int snapCount = values[0];
				int timerValue = values[1];

				if (timerValue == 0) {
					btnTrain.setText("...");
					trainCallBack.setSnapCount(snapCount);
					if (snapCount == GoldenEyeConstants.TRAINING_SNAP_COUNT) {
						trainCallBack.moveToTrainPhase();
					}
					mCamera.takePicture(null, null, null, trainCallBack);
					
				} else {
					btnTrain.setText("" + timerValue);
				}
				trainTimerHandlerFlag = true;
				return;
			}
		};
		trainTimerThread = new TrainTimerThread(trainTimerHandler);

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
		btnSnap = (Button) findViewById(R.id.btnSnap);
		btnSnap.setOnClickListener(this);
		btnTrain = (Button) findViewById(R.id.btnTrain);
		btnTrain.setOnClickListener(this);
		iSurfaceView = (SurfaceView) findViewById(R.id.surface_view);
		imageView = (ImageView) findViewById(R.id.image_view);

		mHolder = iSurfaceView.getHolder();
		mHolder.addCallback(this);
		mHolder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);

	}

	@Override
	protected void onDestroy() {
		super.onDestroy();

		File localHaarXmlFile = new File(
				GoldenEyeConstants.LOCAL_HAAR_CLASSIFIER_XML);
		if (!localHaarXmlFile.delete()) {
			Log.e("StartScreen", "Temporary file : " + localHaarXmlFile
					+ " could not be deleted on exit");
		}
		GoldenEye.destroy();
	}

	public void surfaceCreated(SurfaceHolder holder) {
		// The Surface has been created, acquire the camera and tell it where
		// to draw.

		mCamera = Camera.open();
		try {
			mCamera.setPreviewDisplay(holder);

		} catch (IOException exception) {
			mCamera.release();
			mCamera = null;
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

		if (v.getId() == R.id.btnSnap) {
			if (!pictureTaken) {
				snapTimerThread.startTimer();
			} else {
				btnSnap.setText("Snap");
				pictureTaken = false;
				iSurfaceView.setVisibility(View.VISIBLE);
				imageView.setVisibility(View.INVISIBLE);
				mCamera.startPreview();

			}
		} else if (v.getId() == R.id.btnTrain) {
			iSurfaceView.setVisibility(View.VISIBLE);
			imageView.setVisibility(View.INVISIBLE);
			Log.i(GoldenEyeConstants.LOG_TAG, "train timer started");
			mCamera.startPreview();
			trainTimerThread.startTimer();
		}

	}

	Camera.PictureCallback snapCallback = new Camera.PictureCallback() {

		public void onPictureTaken(byte[] data, Camera camera) {

			Log.i(GoldenEyeConstants.LOG_TAG, "inside onPictureTake()");
			Bitmap bitmap = BitmapFactory.decodeByteArray(data, 0, data.length);
			int width = bitmap.getWidth();
			int height = bitmap.getHeight();

			float scaleWidth = ((float) iSurfaceView.getWidth()) / width;
			float scaleHeight = ((float) iSurfaceView.getHeight()) / height;
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

			personName = GoldenEye.recognizeFace(INPUT_FILE, OUTPUT_FILE);

			Log.e(GoldenEyeConstants.LOG_TAG, "personName detected : "
					+ personName);

			if (!personName.equals("")) {
				imageView.setImageBitmap(BitmapFactory.decodeFile(OUTPUT_FILE));
			}
			btnSnap.setVisibility(View.VISIBLE);
			iSurfaceView.setVisibility(View.INVISIBLE);
			imageView.setVisibility(View.VISIBLE);
			btnSnap.setText("Snap Again ?");
			showDialog(GoldenEyeConstants.SHOW_NAME_DIALOG);
			pictureTaken = true;
		}
	};

	private TrainCallBack trainCallBack = new TrainCallBack();

	private class TrainCallBack implements Camera.PictureCallback {
		private int snapCount;
		private boolean trainPhase;
		private List<String> trainImgFilePaths;

		public TrainCallBack() {
			this.snapCount = -1;
			this.trainPhase = false;
			trainImgFilePaths = new Vector<String>();
		}

		public int getSnapCount() {
			return snapCount;
		}

		public void setSnapCount(int snapCount) {
			this.snapCount = snapCount;
		}

		public void moveToTrainPhase() {
			this.trainPhase = true;
		}

		public void onPictureTaken(byte[] data, Camera camera) {

			Log.i(GoldenEyeConstants.LOG_TAG, "writing training image to file");
			Bitmap bitmap = BitmapFactory.decodeByteArray(data, 0, data.length);
			int width = bitmap.getWidth();
			int height = bitmap.getHeight();

			float scaleWidth = ((float) iSurfaceView.getWidth()) / width;
			float scaleHeight = ((float) iSurfaceView.getHeight()) / height;
			Matrix matrix = new Matrix();
			matrix.postScale(scaleWidth, scaleHeight);
			Bitmap resizedBitmap = Bitmap.createBitmap(bitmap, 0, 0, width,
					height, matrix, true);

			String fileName = String.format(TRAIN_FILE_PREFIX + ""
					+ getSnapCount() + ".jpg", System.currentTimeMillis());
			new File(fileName).delete();
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

			this.trainImgFilePaths.add(fileName);
			if (this.trainPhase) {

				this.trainPhase = false;
				nameInputListener = new NameInputDialogListener(
						trainImgFilePaths);
				showDialog(GoldenEyeConstants.NAME_INPUT_DIALOG);
			} else {
				mCamera.startPreview();
				trainTimerThread.startTimer();
			}
		}

	}

	@Override
	protected Dialog onCreateDialog(int id) {
		Dialog dialog = null;
		AlertDialog.Builder builder = null;
		switch (id) {
		case GoldenEyeConstants.NAME_INPUT_DIALOG:
			builder = new AlertDialog.Builder(this);
			final EditText input = new EditText(this);
			nameInputListener.setInputText(input);
			builder.setMessage("").setTitle("Enter Name").setView(input)
					.setCancelable(false)
					.setPositiveButton("Okay", nameInputListener);
			dialog = builder.create();
			break;
		case GoldenEyeConstants.SHOW_NAME_DIALOG:
			builder = new AlertDialog.Builder(this);
			builder.setTitle("Face Recognition result").setPositiveButton(
					"Okay", null);
			Log.i(GoldenEyeConstants.LOG_TAG, "person name in Dialog : "
					+ personName);
			if (personName.equals("")) {
				builder.setMessage("Unable to recognize face");
			} else {
				builder.setMessage("Identified user : " + personName);
			}
			dialog = builder.create();
			dialog.setOnDismissListener(new OnDismissListener() {

				@Override
				public void onDismiss(DialogInterface dialog) {
					removeDialog(GoldenEyeConstants.SHOW_NAME_DIALOG);

				}
			});

			break;
		}
		return dialog;
	}

	private class NameInputDialogListener implements
			DialogInterface.OnClickListener {

		private EditText input;
		private List<String> trainImgFilePaths;

		public NameInputDialogListener(List<String> trainImgFilePaths) {
			this.trainImgFilePaths = trainImgFilePaths;
		}

		public void setInputText(EditText input) {
			this.input = input;
		}

		@Override
		public void onClick(DialogInterface dialog, int which) {
			dialog.cancel();
			String personName = input.getText().toString();
			input.setText("");
			Log.i(GoldenEyeConstants.LOG_TAG, "Got name : " + personName);
			Log.i(GoldenEyeConstants.LOG_TAG, "training now");

			GoldenEye.train(personName, GoldenEyeConstants.TRAINING_SNAP_COUNT,
					TRAIN_FILE_PREFIX);
			btnTrain.setText("Train");
			mCamera.startPreview();

		}
	}

}
