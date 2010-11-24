 /* hello-jni.i */
 %module GoldenEye
 %{
        extern const char * convertToGreyScale(const char *inputImgPath);
        extern const char * detectFaces(const char *inputImgPath);
        extern int train(const char *name, int numImages, const char *imgPathPrefix);
        extern const char * recognizeFace(const char *inputImgPath, const char * outputImgPath);
        extern void init(const char * haarClassifierXml, const char * outputFolder, const char* imgExtension);
       	extern void destroy();
 %}	
 


%pragma(java) jniclasscode=%{
  static {
    try {
  	     System.loadLibrary("goldeneye");
    } catch (UnsatisfiedLinkError e) {
    	throw e;
    }
  }
%}

extern const char * convertToGreyScale(const char *inputImgPath);
extern const char * detectFaces(const char *inputImgPath);  
extern int train(const char *name, int numImages, const char *imgPathPrefix);
extern const char * recognizeFace(const char *inputImgPath, const char * outputImgPath);
extern void init(const char * haarClassifierXml, const char * outputFolder, const char* imgExtension);
extern void destroy();