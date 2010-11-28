package com.android.goldeneye;

import java.io.File;

import android.os.Environment;

public interface GoldenEyeConstants {
	String BASE_PATH=Environment.getExternalStorageDirectory()+"/Android/data/"+"com.android.goldeneye"+"/files/";
	String IMG_EXTENSION="jpg";
	String FACEDETECTION_FOLDER=BASE_PATH+"/facedetection/";
	String LOCAL_HAAR_CLASSIFIER_XML=BASE_PATH+"/haarclassifier.xml";
	int HAAR_CLASSIFIER_XML=R.raw.haarclassifier;
	int TRAINING_SNAP_COUNT = 5;
	int TRAINING_TIMER =3;
	String LOG_TAG="GoldenEye";
	int FR_INP_HEIGHT=500;
	int FR_INP_WIDTH=900;
	String SNAP_TIMER_KEY = "snap_timer_value";
	String TRAIN_TIMER_KEY = "train_timer_value";
	String PROGRESSBAR_STATE_KEY = "progressbar_state";
	int NAME_INPUT_DIALOG=0;
	int SHOW_NAME_DIALOG=1;
	int TRAIN_SELECT=1;
	int SNAP_SELECT=2;
}
