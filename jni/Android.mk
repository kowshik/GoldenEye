LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#define OPENCV_INCLUDES and OPENCV_LIBS
include $(OPENCV_CONFIG)

LOCAL_LDLIBS += $(OPENCV_LIBS) -llog
    
LOCAL_C_INCLUDES +=  $(OPENCV_INCLUDES) 

LOCAL_MODULE    := opencv-fr

LOCAL_MODULE    := opencv-fr
LOCAL_SRC_FILES := opencv-fr.cpp gen/opencv-fr-jni_wrap.cpp

include $(BUILD_SHARED_LIBRARY)
