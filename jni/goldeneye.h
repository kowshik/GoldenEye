#include <string>
#include <utility>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv/cvaux.h>
#include <android/log.h>

using namespace std;

class GoldenEye {
public:
	GoldenEye();
	GoldenEye(const string& haarClassifierXml, const string& outputFolder,  const string& imgExtension);
	string convertToGreyScale(const string& imagePath);
	string detectFaces(const string& imagePath);
	int train(const string& name, int numOfImgs, const string& imagePathPrefix);
	string recognizeFace(const string& imagePath, const string& outputImgPath);
	~GoldenEye();
private:
	IplImage* convertToGreyScale(const IplImage *pImage);
	IplImage* detectFaces(const IplImage *pImage);
	int boxFaces(IplImage* pImage, CvSeq *pFaceRectSeq);
	int pca();
	int readImages(const string& path, const string& personName);
	int loadExistingImages();
	int loadImages(const string& name,int numOfImgs, const string& imagePathPrefix) ;
	bool isSpecialDir(const string& dirName);
	int addNewImages(const string& name,int numOfImgs, const string& imagePathPrefix);
	int copyFile(const string& thisTrainingImg,const string& newTrainingImage);
	int detectProminentFace(const IplImage* pInputImg,  IplImage** greyImg,  IplImage** boxedImg, CvRect* rect);
	IplImage* resizeImage(const IplImage *origImg, int newWidth, int newHeight);
	IplImage* cropImage(const IplImage *img, const CvRect region);
	int findNearestNeighbor(float * projectedTestFace, float *pConfidence);
	void storeEigenFaceImages();
	IplImage* convertFloatImageToUcharImage(const IplImage *srcImg);

	string haarClassifierXml;
	string outputImgFolder;
	string trainedImgsPath;
	CvHaarClassifierCascade *pCascade;

	void freeDataStructures();

	CvMat * projectedTrainFaceMat;
	CvMat * eigenValMat;
	IplImage **faceImageArr;
	IplImage **eigenVectArr;
	IplImage *pAvgTrainImg;
	vector<string> personNames;
	vector< pair<string, string> > trainingImages;
	vector<int> personNumbers;

	int nEigens;
	bool trainingDone;
	string imgExtension;
	int resizeWidth;
	int resizeHeight;
};

