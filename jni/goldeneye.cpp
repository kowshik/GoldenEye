#include <goldeneye.h>
#include <stdio.h>

GoldenEye::GoldenEye() {

}
GoldenEye::GoldenEye(const string& haarClassifierXml,
		const string& outputImgFolder) {
	this->haarClassifierXml = haarClassifierXml;
	this->outputImgFolder = outputImgFolder;
}
string GoldenEye::convertToGreyScale(const string& imagePath) {
	IplImage * pInpImg = NULL;
	bool converted = false;

	// Load input image from file
	pInpImg = cvLoadImage(imagePath.c_str());
	if (!pInpImg) {
		fprintf(stderr, "failed to load input image\n");
		return "";
	}

	IplImage *pGreyScaleImg = this->convertToGreyScale(pInpImg);

	// Write the image to a file with a different name
	string greyScaleImgPath = this->outputImgFolder + "/greyscale.jpg";
	cvSaveImage(greyScaleImgPath.c_str(), pGreyScaleImg);

	// Remember to free image memory after using it!
	cvReleaseImage(&pInpImg);
	cvReleaseImage(&pGreyScaleImg);

	return greyScaleImgPath;
}

IplImage* GoldenEye::convertToGreyScale(const IplImage *pImage) {
	IplImage *pImageGrey;
	/* Either convert the image to greyscale, or make a copy of the existing greyscale image.
	 This is to make sure that the user can always call cvReleaseImage() on the output, whether
	 it was greyscale or not.
	 */
	if (pImage->nChannels == 3) {
		pImageGrey = cvCreateImage(cvGetSize(pImage), IPL_DEPTH_8U, 1);
		cvCvtColor(pImage, pImageGrey, CV_BGR2GRAY);
	} else {
		pImageGrey = cvCloneImage(pImage);
	}
	return pImageGrey;
}

IplImage* GoldenEye::detectFaces(const IplImage* pInputImg) {
	if(!pInputImg) {
			fprintf(stderr, "pInpImg is NULL\n");
			return NULL;
		}
		CvHaarClassifierCascade *pCascade = NULL;
		CvMemStorage *pStorage = NULL;
		CvSeq *pFaceRectSeq = NULL;
		int i;

		IplImage* pOutputImg = cvCloneImage(pInputImg);

		pStorage = cvCreateMemStorage(512);
		__android_log_write(ANDROID_LOG_INFO,"Native Code","Loading Haar xml");
		pCascade = (CvHaarClassifierCascade *) cvLoad(haarClassifierXml.c_str(), 0, 0,
				0);
		__android_log_write(ANDROID_LOG_INFO,"Native Code","Loaded Haar xml");
		if (!pOutputImg || !pStorage || !pCascade) {
			fprintf(stderr, "Initialization failed\n");
			return NULL;
		}

		//Run the face detection algorithm
		pFaceRectSeq = cvHaarDetectObjects(pOutputImg, pCascade, pStorage, 1.1, 3,
				CV_HAAR_DO_CANNY_PRUNING, cvSize(0, 0));

		//Draw a green rectangular box around the faces
		for (i = 0; i < (pFaceRectSeq ? pFaceRectSeq->total : 0); i++) {
			CvRect *r = (CvRect*) cvGetSeqElem(pFaceRectSeq, i);
			CvPoint pt1 = { r->x, r->y };
			CvPoint pt2 = { r->x + r->width, r->y + r->height };
			cvRectangle(pOutputImg, pt1, pt2, CV_RGB(0, 255, 0), 3, 4, 0);
		}

		return pOutputImg;

}

string GoldenEye::detectFaces(const string& imagePath) {
	IplImage * pInputImg = cvLoadImage(imagePath.c_str());
	if (!pInputImg) {
		fprintf(stderr, "failed to load input image\n");
		return "";
	}
	IplImage* pOutputImg = detectFaces(pInputImg);
	string outputImgPath = this->outputImgFolder + "/detectedfaces.jpg";
	if (pOutputImg) {
		cvSaveImage(outputImgPath.c_str(), pOutputImg);
		cvReleaseImage(&pInputImg);
		cvReleaseImage(&pOutputImg);
		return "";
	}
	return 0;
}

GoldenEye::~GoldenEye() {
}

