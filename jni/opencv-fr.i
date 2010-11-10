 /* hello-jni.i */
 %module GoldenEye
 %{
        extern int convertToGreyScale(const char *inputImgPath, const char *greyScaleImgPath);
	extern int detectFaces(const char *inputImgPath, const char *outputImgPath, const char *haarClassifierXmlPath);
 %}
 


%pragma(java) jniclasscode=%{
  static {
    try {
    	//load the library, make sure that libandroid-opencv.so is in your <project>/libs/armeabi directory
    	//so that android sdk automatically installs it along with the app.
        System.loadLibrary("opencv-fr");
    } catch (UnsatisfiedLinkError e) {
    	//badness
    	throw e;
     
    }
  }
%}

extern int convertToGreyScale(const char *inputImgPath, const char *greyScaleImgPath);
extern int detectFaces(const char *inputImgPath, const char *outputImgPath, const char *haarClassifierXmlPath);

