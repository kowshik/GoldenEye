# The path to the NDK, requires crystax version r-4 for now, due to support
# for the standard library

# load environment from local make file
LOCAL_ENV_MK=local.env.mk
ifneq "$(wildcard $(LOCAL_ENV_MK))" ""
include $(LOCAL_ENV_MK)
else
$(shell cp sample.$(LOCAL_ENV_MK) $(LOCAL_ENV_MK))
$(info ERROR local environement not setup! try:)
$(error Please setup the $(LOCAL_ENV_MK) - the default was just created for you')
endif

ANDROID_NDK_BASE = $(ANDROID_NDK_ROOT)

$(info OPENCV_CONFIG = $(OPENCV_CONFIG))

# Java package that will consume the above shared library
GOLDENEYE_JAVA_PKG = com.android.goldeneye.core

# Find all the C++ sources in the native folder
SOURCES = $(wildcard jni/*.cpp)
HEADERS = $(wildcard jni/*.h)
SWIG_IS = $(wildcard jni/*.i)

ANDROID_MKS = $(wildcard jni/*.mk)

SWIG_MAIN = jni/goldeneye.i

SWIG_JAVA_DIR = src/com/android/goldeneye/core
SWIG_JAVA_OUT = $(wildcard $(SWIG_JAVA_DIR)/*.java)

SWIG_C_DIR = jni/gen
SWIG_C_OUT = $(SWIG_C_DIR)/goldeneye_jni_wrap.cpp

# The real native library stripped of symbols
LIB		= libs/armeabi-v7a/$(LIBNAME) libs/armeabi/$(LIBNAME)


all:	$(LIB) nogdb


#calls the ndk-build script, passing it OPENCV_ROOT and OPENCV_LIBS_DIR
$(LIB): $(SWIG_C_OUT) $(SOURCES) $(HEADERS) $(ANDROID_MKS)
	$(ANDROID_NDK_BASE)/ndk-build OPENCV_CONFIG=$(OPENCV_CONFIG) \
	PROJECT_PATH=$(PROJECT_PATH) V=$(V) $(NDK_FLAGS)


#this creates the swig wrappers
$(SWIG_C_OUT): $(SWIG_IS)
	make clean-swig &&\
	mkdir -p $(SWIG_C_DIR) &&\
	mkdir -p $(SWIG_JAVA_DIR) &&\
	swig -java -c++ -package "$(GOLDENEYE_JAVA_PKG)" \
	-outdir $(SWIG_JAVA_DIR) \
	-o $(SWIG_C_OUT) $(SWIG_MAIN)
	
	
#clean targets
.PHONY: clean  clean-swig cleanall nogdb

nogdb: $(LIB)
	rm -f libs/armeabi*/gdb*

#this deletes the generated swig java and the generated c wrapper
clean-swig:
	rm -f $(SWIG_JAVA_OUT) $(SWIG_C_OUT)
	
#does clean-swig and then uses the ndk-build clean
clean: clean-swig
	$(ANDROID_NDK_BASE)/ndk-build OPENCV_CONFIG=$(OPENCV_CONFIG) \
	PROJECT_PATH=$(PROJECT_PATH) clean V=$(V) $(NDK_FLAGS)
	
