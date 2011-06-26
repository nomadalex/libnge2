LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libcooleye
LOCAL_CFLAGS += -O2 -Wall -DBUILD_STANDALONE -DCPU_ARM -finline-functions -fPIC -D__ARM_EABI__=1
LOCAL_SRC_FILES := audio_android.c
LOCAL_ARM_MODE := arm
LOCAL_LDLIBS := -llog
include $(BUILD_SHARED_LIBRARY)
