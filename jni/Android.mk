LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#define OPENCV_INCLUDES and OPENCV_LIBS
include $(OPENCV_CONFIG)

LOCAL_LDLIBS += $(OPENCV_LIBS) -llog
    
LOCAL_C_INCLUDES +=  $(OPENCV_INCLUDES) goldeneye.h

LOCAL_MODULE    := goldeneye
LOCAL_SRC_FILES :=  goldeneye.cpp gen/goldeneye_jni_wrap.cpp

include $(BUILD_SHARED_LIBRARY)
