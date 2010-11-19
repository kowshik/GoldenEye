#include <string>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv/cvaux.h>
#include <android/log.h>

using namespace std;

class GoldenEye {
public:
	GoldenEye();
	GoldenEye(const string& haarClassifierXml, const string& outputImgFolder);
	string convertToGreyScale(const string& imagePath);
	string detectFaces(const string& imagePath);
	~GoldenEye();
private:
	IplImage* convertToGreyScale(const IplImage *pImage);
	IplImage* detectFaces(const IplImage *pImage);

	string haarClassifierXml;
	string outputImgFolder;


};
