LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_CFLAGS    += -DANDROID -D_DEBUG_LOG
LOCAL_CXXFLAGS  += -DANDROID -D_DEBUG_LOG -fexceptions
LOCAL_MODULE    := nge2app-jni
LOCAL_SRC_FILES := nge_jni.cpp
LOCAL_C_INCLUDES += jni jni/depends/include jni/depends/libnge2/include
LOCAL_LDLIBS := -Ljni/depends/libs -Ljni/libnge2/libs -ljpeg -lfreetype -lpng -lz -lungif -llog -lGLESv1_CM
LOCAL_STATIC_LIBRARIES := libnge2
include $(BUILD_SHARED_LIBRARY)
