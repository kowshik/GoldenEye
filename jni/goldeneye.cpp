#include <goldeneye.h>
#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <fstream>

GoldenEye::GoldenEye() {

}

GoldenEye::~GoldenEye() {
	__android_log_write(ANDROID_LOG_INFO, "NativeCode", "Inside destructor");
	if (this->pCascade) {
		__android_log_write(ANDROID_LOG_INFO, "NativeCode",
				"Releasing pCascade");
		cvReleaseHaarClassifierCascade(&(this->pCascade));
	}
	freeDataStructures();
}

GoldenEye::GoldenEye(const string& haarClassifierXml,
		const string& outputFolder, const string& imgExtension) {

	this->pCascade = NULL;

	this->projectedTrainFaceMat = NULL;
	this->eigenValMat = NULL;
	this->faceImageArr = NULL;
	this->eigenVectArr = NULL;
	this->pAvgTrainImg = NULL;

	this->imgExtension = imgExtension;
	this->haarClassifierXml = haarClassifierXml;
	this->outputImgFolder = outputFolder;
	this->trainedImgsPath = this->outputImgFolder + "/trained_imgs";
	this->pCascade = (CvHaarClassifierCascade *) cvLoad(
			haarClassifierXml.c_str(), 0, 0, 0);
	this->trainingDone = false;
	this->resizeWidth = this->resizeHeight = 0;

	ostringstream oss;
	oss << endl << "Output img folder : " << this->outputImgFolder;
	__android_log_write(ANDROID_LOG_INFO, "NativeCode", oss.str().c_str());

	int status = mkdir(trainedImgsPath.c_str(), S_IRWXU | S_IRGRP | S_IXGRP
			| S_IROTH | S_IXOTH);
}
string GoldenEye::convertToGreyScale(const string& imagePath) {
	IplImage * pInpImg = NULL;
	bool converted = false;

	// Load input image from file
	pInpImg = cvLoadImage(imagePath.c_str());
	if (!pInpImg) {
		__android_log_write(ANDROID_LOG_ERROR, "NativeCode",
				"Failed to load input image. Can't convert to greyscale");
		return "";
	}

	IplImage *pGreyScaleImg = this->convertToGreyScale(pInpImg);

	// Write the image to a file with a different name
	string greyScaleImgPath = this->outputImgFolder + "/greyscale."
			+ this->imgExtension;
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

//Draw a green rectangular box around the faces
//Returns no. of boxed faces
int GoldenEye::boxFaces(IplImage* pImage, CvSeq *pFaceRectSeq) {

	for (int i = 0; i < (pFaceRectSeq ? pFaceRectSeq->total : 0); i++) {
		CvRect *r = (CvRect*) cvGetSeqElem(pFaceRectSeq, i);
		CvPoint pt1 = { r->x, r->y };
		CvPoint pt2 = { r->x + r->width, r->y + r->height };
		cvRectangle(pImage, pt1, pt2, CV_RGB(0, 255, 0), 3, 4, 0);
	}
	return pFaceRectSeq->total;
}

int GoldenEye::detectProminentFace(const IplImage* pInputImg,
		IplImage** greyImg, IplImage** boxedImg, CvRect* rect) {

	*greyImg = convertToGreyScale(pInputImg);
	CvMemStorage *pStorage = cvCreateMemStorage(0);
	cvClearMemStorage(pStorage);

	if (!pStorage || !pCascade || !pInputImg) {
		ostringstream oss;
		oss
				<< "\nInitialization failed. pStorage or pCascade or pInputImage is NULL. Can't detect prominent face.\n";
		__android_log_write(ANDROID_LOG_INFO, "NativeCode", oss.str().c_str());
		return 0;
	}

	//Run the face detection algorithm
	CvSeq *pFaceRectSeq = cvHaarDetectObjects(pInputImg, pCascade, pStorage,
			1.1, 3, CV_HAAR_FIND_BIGGEST_OBJECT | CV_HAAR_DO_ROUGH_SEARCH,
			cvSize(0, 0));

	if (pFaceRectSeq->total > 0) {
		*rect = *(CvRect*) cvGetSeqElem(pFaceRectSeq, 0);
	} else {
		*rect = cvRect(-1, -1, -1, -1);
	}
	*boxedImg = cvCloneImage(pInputImg);
	this->boxFaces(*boxedImg, pFaceRectSeq);
	cvReleaseMemStorage(&pStorage);
	return 1;

}

IplImage* GoldenEye::detectFaces(const IplImage* pInputImg) {
	ostringstream oss;

	if (!pInputImg) {

		oss << "pInpImg is NULL. Can't detect faces.\n";
		__android_log_write(ANDROID_LOG_INFO, "NativeCode", oss.str().c_str());
		return NULL;
	}

	IplImage* pOutputImg = cvCloneImage(pInputImg);
	CvMemStorage *pStorage = cvCreateMemStorage(0);
	cvClearMemStorage(pStorage);

	if (!pOutputImg || !pStorage || !pCascade) {
		oss.str();
		oss
				<< "\nCan't detect faces. pOutputImg or pStorage or pCascade is null. Initialization failed\n";
		__android_log_write(ANDROID_LOG_INFO, "NativeCode", oss.str().c_str());
		return NULL;
	}

	//Run the face detection algorithm
	CvSeq *pFaceRectSeq = cvHaarDetectObjects(pOutputImg, pCascade, pStorage,
			1.1, 3, CV_HAAR_FIND_BIGGEST_OBJECT | CV_HAAR_DO_ROUGH_SEARCH,
			cvSize(0, 0));

	this->boxFaces(pOutputImg, pFaceRectSeq);

	cvReleaseMemStorage(&pStorage);

	return pOutputImg;

}

string GoldenEye::detectFaces(const string& imagePath) {
	IplImage * pInputImg = cvLoadImage(imagePath.c_str());
	if (!pInputImg) {
		__android_log_write(ANDROID_LOG_INFO, "NativeCode",
				"Failed to load input image. Can't detect faces");
		return "";
	}
	IplImage* pOutputImg = detectFaces(pInputImg);
	string outputImgPath = this->outputImgFolder + "/detectedfaces."
			+ this->imgExtension;
	if (pOutputImg) {
		cvSaveImage(outputImgPath.c_str(), pOutputImg);
		cvReleaseImage(&pInputImg);
		cvReleaseImage(&pOutputImg);
		return outputImgPath;
	}
	return "";
}

bool GoldenEye::isSpecialDir(const string& dirName) {
	if (dirName == "." || dirName == "..") {
		return true;
	}
	return false;
}

int GoldenEye::copyFile(const string& src, const string& target) {

	ostringstream oss;

	IplImage* pInpImage = cvLoadImage(src.c_str());
	IplImage* greyImage, *boxedImage;
	CvRect faceRect;

	if (!detectProminentFace(pInpImage, &greyImage, &boxedImage, &faceRect)
			|| faceRect.width <= 0) {
		oss.str("");
		oss << "\nSkipping image. No face found in : " << src;
		__android_log_write(ANDROID_LOG_ERROR, "NativeCode", oss.str().c_str());
		cvReleaseImage(&pInpImage);
		cvReleaseImage(&greyImage);
		cvReleaseImage(&boxedImage);
		return 0;
	}

	//crop image
	IplImage* croppedImage = cropImage(greyImage, faceRect);

	if (resizeWidth == 0) {
		resizeWidth = croppedImage->width;
		resizeHeight = croppedImage->height;
	}

	// Get the detected face image.
	// Make sure the image is the same dimensions as the training images.
	IplImage* sizedImg = resizeImage(croppedImage, resizeWidth, resizeHeight);

	struct stat statInfo;
	if (stat(target.c_str(), &statInfo) == 0) {
		oss.str("");
		oss<<"Removing existing file : "<<target.c_str();
		__android_log_write(ANDROID_LOG_INFO, "NativeCode", oss.str().c_str());
		if(remove(target.c_str())!=0){
			oss.str("");
			oss<<"Attempting to overwrite as file couldn't be removed : "<<target.c_str();
			__android_log_write(ANDROID_LOG_ERROR, "NativeCode", oss.str().c_str());
		}
	}
	cvSaveImage(target.c_str(), sizedImg);

	cvReleaseImage(&pInpImage);
	cvReleaseImage(&greyImage);
	cvReleaseImage(&boxedImage);
	cvReleaseImage(&croppedImage);
	cvReleaseImage(&sizedImg);

	return 1;
}

int GoldenEye::readImages(const string& path, const string& personName) {
	ostringstream oss;
	oss << "\nAttempting to load images for person : " << path;
	__android_log_write(ANDROID_LOG_INFO, "NativeCode", oss.str().c_str());
	DIR *imageDir = opendir(path.c_str());
	struct dirent* epImages;
	int atleastOneImage = -1;

	if (imageDir) {
		while (epImages = readdir(imageDir)) {
			if (epImages->d_type == DT_REG) {
				oss.str("");
				string fullPath = path + "/" + epImages->d_name;
				this->trainingImages.push_back(pair<string, string> (
						personName, fullPath));
				oss << "\tFound : " << fullPath;
				__android_log_write(ANDROID_LOG_INFO, "NativeCode",
						oss.str().c_str());
				if (resizeWidth == 0) {
					__android_log_write(ANDROID_LOG_INFO, "NativeCode",
							"Setting values for resize widths heights");
					IplImage* img = cvLoadImage(fullPath.c_str(),
							CV_LOAD_IMAGE_GRAYSCALE);

					if (!img) {
						__android_log_write(ANDROID_LOG_INFO, "NativeCode",
								"img is NULL :-(");
						return 0;
					}
					resizeWidth = img->width;
					resizeHeight = img->height;
					cvReleaseImage(&img);
					oss.str("");
					oss << "Setting resize width, heights to : " << resizeWidth
							<< "," << resizeHeight;
					__android_log_write(ANDROID_LOG_INFO, "NativeCode",
							oss.str().c_str());
				}
				atleastOneImage = 1;
			}
		}
	} else {
		oss.str("");
		oss << "\nCouldn't open the directory : " << path << "\n";
		__android_log_write(ANDROID_LOG_INFO, "NativeCode", oss.str().c_str());
		return 0;
	}
	closedir(imageDir);
	return atleastOneImage;

}

int GoldenEye::loadExistingImages() {
	ostringstream oss;
	oss << "\nAttempting to load existing images";
	__android_log_write(ANDROID_LOG_INFO, "NativeCode", oss.str().c_str());
	DIR *dirImgs, *dirPersonImgs;
	struct dirent *epImgs;
	int atleastOneImage = -1;

	dirImgs = opendir(this->trainedImgsPath.c_str());
	if (dirImgs != NULL) {
		while (epImgs = readdir(dirImgs)) {
			if (epImgs->d_type == DT_DIR && !isSpecialDir(epImgs->d_name)) {
				personNames.push_back(epImgs->d_name);
				ostringstream oss;
				oss << this->trainedImgsPath << "/" << epImgs->d_name;
				int readImagesResult = this->readImages(oss.str(),
						epImgs->d_name);
				if (!readImagesResult) {
					return 0;
				} else if (readImagesResult == 1) {
					atleastOneImage = 1;
				}
			}

		}
		closedir(dirImgs);
	} else {
		oss.str("");
		oss << "\nCouldn't open person name directory" << this->trainedImgsPath
				<< "\n";
		__android_log_write(ANDROID_LOG_INFO, "NativeCode", oss.str().c_str());
		return 0;
	}

	return atleastOneImage;
}

int GoldenEye::addNewImages(const string& name, int numOfImgs,
		const string& imagePathPrefix) {

	ostringstream oss;
	bool isFolderCreated = false;
	oss << "\nAttempting to copy new training images";
	__android_log_write(ANDROID_LOG_INFO, "NativeCode", oss.str().c_str());
	for (int i = 1; i <= numOfImgs; i++) {
		if (i == 1) {
			this->personNames.push_back(name);
		}
		oss.str("");
		oss << imagePathPrefix << i << "." << this->imgExtension;
		string thisTrainingImg = oss.str();
		struct stat statInfo;
		if (stat(oss.str().c_str(), &statInfo) == 0) {
			string newFolder = this->trainedImgsPath + "/" + name;
			if (!isFolderCreated) {
				if (stat(newFolder.c_str(), &statInfo) != 0) {
					int status = mkdir(newFolder.c_str(), S_IRWXU | S_IRGRP
							| S_IXGRP | S_IROTH | S_IXOTH);
				}
				isFolderCreated = true;
			}
			oss.str("");
			oss << newFolder << "/" << i << "." << this->imgExtension;
			string newTrainingImage = oss.str();
			if (this->copyFile(thisTrainingImg, newTrainingImage)) {

				this->trainingImages.push_back(pair<string, string> (name,
						newTrainingImage));
				oss.str("");
				oss << "\n\tAdded new image : " << name << ","
						<< newTrainingImage;
				__android_log_write(ANDROID_LOG_INFO, "NativeCode",
						oss.str().c_str());
			} else {
				oss.str("");
				oss << "\nUnable to copy file : " << thisTrainingImg << " to "
						<< newTrainingImage;
				__android_log_write(ANDROID_LOG_ERROR, "NativeCode",
						oss.str().c_str());
			}

		} else {
			oss.str("");
			oss << "\nUnable to open training image : " << oss.str() << endl;
			__android_log_write(ANDROID_LOG_ERROR, "NativeCode",
					oss.str().c_str());
			return 0;
		}
	}
	return 1;
}
int GoldenEye::loadImages(const string& name, int numOfImgs,
		const string& imagePathPrefix) {
	ostringstream oss;
	oss << "\nAttempting to load images";
	__android_log_write(ANDROID_LOG_INFO, "NativeCode", oss.str().c_str());
	int resultLoadImgs = this->loadExistingImages();
	if (resultLoadImgs == -1) {
		oss.str("");
		oss << "\nDidn't find any existing images to load\n";
		__android_log_write(ANDROID_LOG_INFO, "NativeCode", oss.str().c_str());
	}

	int resultAddImgs = this->addNewImages(name, numOfImgs, imagePathPrefix);
	if ((resultLoadImgs == -1 || resultLoadImgs == 0) && (!resultAddImgs)) {
		return 0;
	}

	int numImages = this->trainingImages.size();

	// array of face images to be loaded
	this->faceImageArr = (IplImage **) cvAlloc(numImages * sizeof(IplImage *));

	// store the face images in the array
	for (int iFace = 0; iFace < numImages; iFace++) {
		string imgFileName = this->trainingImages[iFace].second;
		this->faceImageArr[iFace] = cvLoadImage(imgFileName.c_str(),
				CV_LOAD_IMAGE_GRAYSCALE);
		if (!this->faceImageArr[iFace]) {
			ostringstream oss;
			oss.str("");
			oss << "Can't create IplImage : " << imgFileName;
			__android_log_write(ANDROID_LOG_ERROR, "NativeCode",
					oss.str().c_str());
			return 0;
		}
		oss.str("");
		oss << endl << "Loaded IplImage : " << trainingImages[iFace].first
				<< "," << imgFileName;
		__android_log_write(ANDROID_LOG_INFO, "NativeCode", oss.str().c_str());
	}
	oss.str("");
	oss << endl << "Total images loaded : " << numImages << endl;
	__android_log_write(ANDROID_LOG_INFO, "NativeCode", oss.str().c_str());
	return 1;
}

// Creates a new image copy that is of a desired size.
// Remember to free the new image later.
IplImage* GoldenEye::resizeImage(const IplImage *origImg, int newWidth,
		int newHeight) {
	IplImage *outImg = 0;
	int origWidth;
	int origHeight;
	if (origImg) {
		origWidth = origImg->width;
		origHeight = origImg->height;
	}
	if (newWidth <= 0 || newHeight <= 0 || origImg == 0 || origWidth <= 0
			|| origHeight <= 0) {
		printf("ERROR in resizeImage: Bad desired image size of %dx%d\n.",
				newWidth, newHeight);
		exit(1);
	}

	// Scale the image to the new dimensions, even if the aspect ratio will be changed.
	outImg = cvCreateImage(cvSize(newWidth, newHeight), origImg->depth,
			origImg->nChannels);
	if (newWidth > origImg->width && newHeight > origImg->height) {
		// Make the image larger
		cvResetImageROI((IplImage*) origImg);
		cvResize(origImg, outImg, CV_INTER_LINEAR); // CV_INTER_CUBIC or CV_INTER_LINEAR is good for enlarging
	} else {
		// Make the image smaller
		cvResetImageROI((IplImage*) origImg);
		cvResize(origImg, outImg, CV_INTER_AREA); // CV_INTER_AREA is good for shrinking / decimation, but bad at enlarging.
	}

	return outImg;
}

// Returns a new image that is a cropped version of the original image.
IplImage* GoldenEye::cropImage(const IplImage *img, const CvRect region) {
	ostringstream oss;
	IplImage *imageTmp;
	IplImage *imageRGB;
	CvSize size;
	size.height = img->height;
	size.width = img->width;

	if (img->depth != IPL_DEPTH_8U) {
		oss.str("");
		oss
				<< "ERROR in cropImage: Unknown image depth of %d given in cropImage() instead of 8 bits per pixel.\n"
				<< img->depth;
		__android_log_write(ANDROID_LOG_ERROR, "NativeCode", oss.str().c_str());
		return NULL;
	}

	// First create a new (color or greyscale) IPL Image and copy contents of img into it.
	imageTmp = cvCreateImage(size, IPL_DEPTH_8U, img->nChannels);
	cvCopy(img, imageTmp, NULL);

	// Create a new image of the detected region
	// Set region of interest to that surrounding the face
	cvSetImageROI(imageTmp, region);
	// Copy region of interest (i.e. face) into a new iplImage (imageRGB) and return it
	size.width = region.width;
	size.height = region.height;
	imageRGB = cvCreateImage(size, IPL_DEPTH_8U, img->nChannels);
	cvCopy(imageTmp, imageRGB, NULL); // Copy just the region.

	cvReleaseImage(&imageTmp);
	return imageRGB;
}

// Find the most likely person based on a detection. Returns the index, and stores the confidence value into pConfidence.
int GoldenEye::findNearestNeighbor(float * projectedTestFace,
		float *pConfidence) {
	ostringstream oss;
	//double leastDistSq = 1e12;
	double leastDistSq = DBL_MAX;
	int i, iTrain, iNearest = 0;

	for (iTrain = 0; iTrain < this->trainingImages.size(); iTrain++) {
		double distSq = 0;
		cerr << "\niTrain : " << iTrain << "\n";
		for (i = 0; i < this->nEigens; i++) {
			float d_i = projectedTestFace[i]
					- projectedTrainFaceMat->data.fl[iTrain * nEigens + i];
#ifdef USE_MAHALANOBIS_DISTANCE
			distSq += d_i*d_i / eigenValMat->data.fl[i]; // Mahalanobis distance (might give better results than Eucalidean distance)
#else
			distSq += d_i * d_i; // Euclidean distance.
#endif
		}

		if (distSq < leastDistSq) {
			leastDistSq = distSq;
			iNearest = iTrain;
		}
	}

	// Return the confidence level based on the Euclidean distance,
	// so that similar images should give a confidence between 0.5 to 1.0,
	// and very different images should give a confidence between 0.0 to 0.5.
	*pConfidence = 1.0f - sqrt(leastDistSq
			/ (float) (this->trainingImages.size() * this->nEigens)) / 255.0f;
	oss.str("");
	oss << "\nconfidence : " << *pConfidence << "\n";
	__android_log_write(ANDROID_LOG_INFO, "NativeCode", oss.str().c_str());
	// Return the found index.
	oss.str("");
	oss << "\niNearest : " << iNearest << "\n";
	__android_log_write(ANDROID_LOG_INFO, "NativeCode", oss.str().c_str());
	return iNearest;
}

string GoldenEye::recognizeFace(const string& imagePath,
		const string& outputImgPath) {
	ostringstream oss;
	IplImage* pInpImage = cvLoadImage(imagePath.c_str());
	if (!pInpImage) {
		oss.str("");
		oss << "\nFailed to load input image : " << imagePath;
		__android_log_write(ANDROID_LOG_ERROR, "NativeCode", oss.str().c_str());
		return "";
	}

	if (!trainingDone) {
		if (!this->train("", 0, "")) {
			oss.str("");
			oss << "\nCan't recognize face. Couldn't finish training.\n";
			__android_log_write(ANDROID_LOG_ERROR, "NativeCode",
					oss.str().c_str());
			return "";
		}
	}
	float * projectedTestFace = (float *) cvAlloc(nEigens * sizeof(float));
	IplImage* greyImage, *boxedImage;

	CvRect faceRect;
	if (!detectProminentFace(pInpImage, &greyImage, &boxedImage, &faceRect)
			|| faceRect.width <= 0) {
		oss.str("");
		oss << "\nCan't recognize face. Image contains no face : " << imagePath;
		__android_log_write(ANDROID_LOG_ERROR, "NativeCode", oss.str().c_str());

		return "";
	}

	int faceWidth = pAvgTrainImg->width;
	int faceHeight = pAvgTrainImg->height;

	//save grey image
	string greyImgOutput = this->outputImgFolder + "/greyImg."
			+ this->imgExtension;
	cvSaveImage(greyImgOutput.c_str(), greyImage);
	oss.str("");
	oss << "\nCreated grey image : " << greyImgOutput << "\n";
	__android_log_write(ANDROID_LOG_INFO, "NativeCode", oss.str().c_str());

	//crop image
	IplImage* croppedImage = cropImage(greyImage, faceRect);
	string croppedImageOutput = this->outputImgFolder + "/croppedImg."
			+ this->imgExtension;
	cvSaveImage(croppedImageOutput.c_str(), croppedImage);
	oss.str("");
	oss << "\nCreated cropped image : " << croppedImageOutput << "\n";
	__android_log_write(ANDROID_LOG_INFO, "NativeCode", oss.str().c_str());

	// Get the detected face image.
	// Make sure the image is the same dimensions as the training images.
	IplImage* sizedImg = resizeImage(croppedImage, faceWidth, faceHeight);
	string sizedImageOutput = this->outputImgFolder + "/sizedImg."
			+ this->imgExtension;
	cvSaveImage(sizedImageOutput.c_str(), sizedImg);
	oss.str("");
	oss << "\nCreated sized image : " << sizedImageOutput << "\n";
	__android_log_write(ANDROID_LOG_INFO, "NativeCode", oss.str().c_str());

	// Give the image a standard brightness and contrast, in case it was too dark or low contrast.
	// Create an empty greyscale image
	IplImage* equalizedImg = cvCreateImage(cvGetSize(sizedImg), 8, 1);
	cvEqualizeHist(sizedImg, equalizedImg);
	string eqImageOutput = this->outputImgFolder + "/eqImg."
			+ this->imgExtension;
	cvSaveImage(eqImageOutput.c_str(), sizedImg);
	oss.str("");
	oss << "\nCreated equalized image : " << eqImageOutput << "\n";
	__android_log_write(ANDROID_LOG_INFO, "NativeCode", oss.str().c_str());

	IplImage* processedFaceImg = equalizedImg;
	if (!processedFaceImg) {
		oss.str("");
		oss << "\nCan't recognize face. processedFaceImg is empty.\n";
		__android_log_write(ANDROID_LOG_ERROR, "NativeCode", oss.str().c_str());
		return "";
	}

	string person = "";
	if (this->nEigens > 0) {
		float confidence;

		// project the test image onto the PCA subspace
		cvEigenDecomposite(processedFaceImg, nEigens, eigenVectArr, 0, 0,
				pAvgTrainImg, projectedTestFace);

		__android_log_write(ANDROID_LOG_INFO, "NativeCode",
				"\nProjected test image onto PCA subspace\n");

		// Check which person it is most likely to be.
		int iNearest = findNearestNeighbor(projectedTestFace, &confidence);
		oss.str("");
		oss << "Most likely person in camera: '"
				<< trainingImages[iNearest].first << "', confidence="
				<< confidence << "\n";
		person = trainingImages[iNearest].first;
		__android_log_write(ANDROID_LOG_INFO, "NativeCode", oss.str().c_str());

	}//endif nEigens

	cvReleaseImage(&equalizedImg);
	cvReleaseImage(&sizedImg);
	cvReleaseImage(&croppedImage);
	cvReleaseImage(&greyImage);
	cvReleaseImage(&pInpImage);

	//save final image
	string boxedImgOutput = outputImgPath;
	cvSaveImage(boxedImgOutput.c_str(), boxedImage);
	oss.str("");
	oss << "\nCreated boxed image : " << boxedImgOutput << "\n";
	__android_log_write(ANDROID_LOG_INFO, "NativeCode", oss.str().c_str());
	cvReleaseImage(&boxedImage);

	return person;

}

int GoldenEye::train(const string& name, int numOfImgs,
		const string& imagePathPrefix) {
	ostringstream oss;
	int offset, i;

	__android_log_write(ANDROID_LOG_INFO, "NativeCode", "\nInside train(..)\n");
	this->freeDataStructures();
	if (this->loadImages(name, numOfImgs, imagePathPrefix)) {
		__android_log_write(ANDROID_LOG_INFO, "NativeCode",
				"\nSuccessfully gathered training images\n");

		if (pca()) {
			string avgImgPath = this->outputImgFolder + "/avgimage."
					+ this->imgExtension;
			cvSaveImage(avgImgPath.c_str(), this->pAvgTrainImg);

			this->projectedTrainFaceMat = cvCreateMat(
					this->trainingImages.size(), this->nEigens, CV_32FC1);
			offset = projectedTrainFaceMat->step / sizeof(float);
			for (i = 0; i < this->trainingImages.size(); i++) {
				cvEigenDecomposite(faceImageArr[i], this->nEigens,
						this->eigenVectArr, 0, 0, this->pAvgTrainImg,
						this->projectedTrainFaceMat->data.fl + i * offset);
			}
			this->storeEigenFaceImages();
		} else {
			__android_log_write(ANDROID_LOG_ERROR, "NativeCode",
					"\nError in PCA\n");
			return 0;
		}

	} else {
		__android_log_write(ANDROID_LOG_ERROR, "NativeCode",
				"\nSome error while loading training faces\n");
		return 0;
	}

	this->trainingDone = true;
	return 1;
}

// Do the Principal Component Analysis, finding the average image
// and the eigenfaces that represent any image in the given dataset.
int GoldenEye::pca() {
	__android_log_write(ANDROID_LOG_INFO, "NativeCode",
			"Inside GoldenEye::pca()");
	int i;
	CvTermCriteria calcLimit;
	CvSize faceImgSize;

	int numOfImgs = this->trainingImages.size();

	// set the number of eigenvalues to use. it should be greater than 2
	this->nEigens = numOfImgs - 1;

	if (this->nEigens < 1) {
		__android_log_write(ANDROID_LOG_ERROR, "NativeCode",
				"Can't do PCA. nEigens < 1.");
		return 0;
	}

	// allocate the eigenvector images
	faceImgSize.width = this->faceImageArr[0]->width;
	faceImgSize.height = this->faceImageArr[0]->height;

	this->eigenVectArr
			= (IplImage**) cvAlloc(sizeof(IplImage*) * this->nEigens);
	__android_log_write(ANDROID_LOG_INFO, "NativeCode",
			"Allocated eigenVectArr");

	for (i = 0; i < this->nEigens; i++) {
		this->eigenVectArr[i] = cvCreateImage(faceImgSize, IPL_DEPTH_32F, 1);
		__android_log_write(ANDROID_LOG_INFO, "NativeCode",
				"Added to eigenVectArr");
	}
	__android_log_write(ANDROID_LOG_INFO, "NativeCode", "Loaded eigenVectArr");
	// allocate the eigenvalue array
	this->eigenValMat = cvCreateMat(1, this->nEigens, CV_32FC1);

	__android_log_write(ANDROID_LOG_INFO, "NativeCode", "Loaded eigenValMat");
	// allocate the averaged image
	this->pAvgTrainImg = cvCreateImage(faceImgSize, IPL_DEPTH_32F, 1);

	__android_log_write(ANDROID_LOG_INFO, "NativeCode",
			"Allocated pAvgTrainImg");
	// set the PCA termination criterion
	calcLimit = cvTermCriteria(CV_TERMCRIT_ITER, this->nEigens, 1);

	// compute average image, eigenvalues, and eigenvectors
	cvCalcEigenObjects(numOfImgs, (void*) this->faceImageArr,
			(void*) this->eigenVectArr, CV_EIGOBJ_NO_CALLBACK, 0, 0,
			&calcLimit, pAvgTrainImg, eigenValMat->data.fl);

	cvNormalize(eigenValMat, eigenValMat, 1, 0, CV_L1, 0);
	return 1;
}

void GoldenEye::freeDataStructures() {

	__android_log_write(ANDROID_LOG_INFO, "NativeCode",
			"Inside freeDataStructures()");
	if (faceImageArr) {
		__android_log_write(ANDROID_LOG_INFO, "NativeCode",
				"Freeing faceImageArr");
		cvFree( faceImageArr);
	}
	if (eigenVectArr) {
		__android_log_write(ANDROID_LOG_INFO, "NativeCode",
				"Freeing eigenVectArr");
		cvFree( eigenVectArr);
	}
	if (pAvgTrainImg) {
		__android_log_write(ANDROID_LOG_INFO, "NativeCode",
				"Freeing pAvgTrainImg");
		cvReleaseImage(&pAvgTrainImg);
	}
	if (projectedTrainFaceMat) {
		__android_log_write(ANDROID_LOG_INFO, "NativeCode",
				"Freeing projectedTrainFaceMat");
		cvFree(&projectedTrainFaceMat); // projected training faces
	}
	if (eigenValMat) {
		__android_log_write(ANDROID_LOG_INFO, "NativeCode",
				"Freeing eigenValMat");
		cvFree(&eigenValMat); // projected training faces
	}
	personNames.clear();
	trainingImages.clear();
	personNumbers.clear();
}

// Save all the eigenvectors as images, so that they can be checked.
void GoldenEye::storeEigenFaceImages() {

	ostringstream oss;
	// Create a large image made of many eigenface images.
	// Must also convert each eigenface image to a normal 8-bit UCHAR image instead of a 32-bit float image.
	if (nEigens > 0) {
		// Put all the eigenfaces next to each other.
		int COLUMNS = 8; // Put upto 8 images on a row.
		int nCols = min(nEigens, COLUMNS);
		int nRows = 1 + (nEigens / COLUMNS); // Put the rest on new rows.
		int w = eigenVectArr[0]->width;
		int h = eigenVectArr[0]->height;
		CvSize size;
		size = cvSize(nCols * w, nRows * h);
		IplImage *bigImg = cvCreateImage(size, IPL_DEPTH_8U, 1); // 8-bit Greyscale UCHAR image
		for (int i = 0; i < nEigens; i++) {
			// Get the eigenface image.
			IplImage *byteImg = convertFloatImageToUcharImage(eigenVectArr[i]);
			// Paste it into the correct position.
			int x = w * (i % COLUMNS);
			int y = h * (i / COLUMNS);
			CvRect ROI = cvRect(x, y, w, h);
			cvSetImageROI(bigImg, ROI);
			cvCopyImage(byteImg, bigImg);
			cvResetImageROI(bigImg);
			cvReleaseImage(&byteImg);
		}

		//Save eigen face image
		string eigenFaceImgPath = this->outputImgFolder + "/eigenfaces."
				+ this->imgExtension;
		cvSaveImage(eigenFaceImgPath.c_str(), bigImg);
		oss << "\nSaved eigen faces image : " << eigenFaceImgPath;
		__android_log_write(ANDROID_LOG_INFO, "NativeCode", oss.str().c_str());
		cvReleaseImage(&bigImg);
	}
}

// Get an 8-bit equivalent of the 32-bit Float image.
// Returns a new image, so remember to call 'cvReleaseImage()' on the result.
IplImage* GoldenEye::convertFloatImageToUcharImage(const IplImage *srcImg) {
	IplImage *dstImg = 0;
	if ((srcImg) && (srcImg->width > 0 && srcImg->height > 0)) {

		// Spread the 32bit floating point pixels to fit within 8bit pixel range.
		double minVal, maxVal;
		cvMinMaxLoc(srcImg, &minVal, &maxVal);

		//cout << "FloatImage:(minV=" << minVal << ", maxV=" << maxVal << ")." << endl;

		// Deal with NaN and extreme values, since the DFT seems to give some NaN results.
		if (cvIsNaN(minVal) || minVal < -1e30)
			minVal = -1e30;
		if (cvIsNaN(maxVal) || maxVal > 1e30)
			maxVal = 1e30;
		if (maxVal - minVal == 0.0f)
			maxVal = minVal + 0.001; // remove potential divide by zero errors.

		// Convert the format
		dstImg = cvCreateImage(cvSize(srcImg->width, srcImg->height), 8, 1);
		cvConvertScale(srcImg, dstImg, 255.0 / (maxVal - minVal), -minVal
				* 255.0 / (maxVal - minVal));
	}
	return dstImg;
}
