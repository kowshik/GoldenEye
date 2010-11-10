package com.android.goldeneye;

import java.io.File;

import android.os.Environment;

public interface GoldenEyeConstants {
	String BASE_PATH=Environment.getExternalStorageDirectory()+"/Android/data/"+"com.android.goldeneye"+"/files/";
	String FACEDETECTION_FOLDER=BASE_PATH+"/facedetection/";
	String LOCAL_HAAR_CLASSIFIER_XML=BASE_PATH+"/localHaarXml.xml";
	int HAAR_CLASSIFIER_XML=R.raw.haarclassifier;
	String LOG_TAG="GoldenEye";
	int FR_INP_HEIGHT=500;
	int FR_INP_WIDTH=600;
	
	
}
