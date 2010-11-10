#include <opencv/cv.h>
#include <opencv/highgui.h>



/*
	Tries to convert a given image into greyscale. If its not possible to convert the image, 
        the input image is cloned into the output image. The user of this function has to call 
	cvReleaseImage(..) on the output.
	
	Parameters :
	* pImage -> Pointer to IplImg object representing the input image.
	
	Returns :
	* If successful, Pointer to IplImg object containing the grey scale image.
	* If unsuccessful, a pointer to IplImg object containing a cloned copy of pImage.
*/
IplImage* convertImageToGreyscale(const IplImage *pImage) {
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

/*
	Converts an input image to greyscale.
	
	Parameters :
	* inputImgPath -> String containing absolute path of input image file.
        * greyScaleImgPath -> String containing absolute path of expected grey scale image file.
	
	Returns :
	* If successful, 1.
	* If unsuccessful, 0.
*/
int convertToGreyScale(const char *inputImgPath, const char *greyScaleImgPath) {

	IplImage * pInpImg = NULL;
	bool converted = false;

	// Load input image from file
	pInpImg = cvLoadImage(inputImgPath);
	if (!pInpImg) {
		fprintf(stderr, "failed to load input image\n");
		return 0;
	}

	IplImage *pGreyScaleImg = convertImageToGreyscale(pInpImg);
	// Write the image to a file with a different name,
	cvSaveImage(greyScaleImgPath, pGreyScaleImg);

	// Remember to free image memory after using it!
	cvReleaseImage(&pInpImg);
	cvReleaseImage(&pGreyScaleImg);

	return 1;
}



/*
	Detects faces in the input image, based on the given Haar Classifier XML data.
	
	Parameters :
	* pInputImg -> Pointer to IplImg object representing the input image.
	* haarClassifierXmlPath -> String containing absolute path to XML file to be used by the haar classifier
	
	Returns:
	* If successful, Pointer to an IplImg object representing the output image with faces detected using a greem
	  rectangular box.
	* If unsuccessful, NULL pointer.
*/
IplImage* detectImageFaces(const IplImage* pInputImg, const char *haarClassifierXmlPath) {
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
	pCascade = (CvHaarClassifierCascade *) cvLoad(haarClassifierXmlPath, 0, 0,
			0);

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

/*
	Detects faces in the input image, based on the given Haar Classifier XML data. Places the
	output in the specified file with a green rectangular box around each detected face.
	
	Parameters :
	inputImgPath -> String containing absolute path to input image file.
        greyScaleImgPath -> String containing absolute of image file with faces detected.
	
	Returns :
	* If successful, 1.
	* If unsuccessful, 0.
*/
int detectFaces(const char *inputImgPath, const char *outputImgPath, const char *haarClassifierXmlPath){
	
	IplImage * pInputImg = cvLoadImage(inputImgPath);
	if (!pInputImg) {
		fprintf(stderr, "failed to load input image\n");
		return 1;
	}
	IplImage* pOutputImg=detectImageFaces(pInputImg, haarClassifierXmlPath);

	if(pOutputImg){
		cvSaveImage(outputImgPath, pOutputImg);
		cvReleaseImage(&pInputImg);
		cvReleaseImage(&pOutputImg);
		return 1;
	}
	return 0;

}
