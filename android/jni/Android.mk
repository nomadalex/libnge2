LOCAL_PATH := $(call my-dir)

# library for Android < 9
include $(CLEAR_VARS)
LOCAL_MODULE := libcooleye8
LOCAL_CFLAGS += -O2 -Wall -DBUILD_STANDALONE -DCPU_ARM -DAVSREMOTE -finline-functions -fPIC -D__ARM_EABI__=1 -DOLD_LOGDH 
LOCAL_C_INCLUDES += $(LOCAL_PATH)/Android/include
LOCAL_SRC_FILES := audio_android.cpp
LOCAL_ARM_MODE := arm
LOCAL_LDLIBS := -llog \
 $(LOCAL_PATH)/Android/lib/libutils.so $(LOCAL_PATH)/Android/lib/libmedia.so
include $(BUILD_SHARED_LIBRARY)

# library for Android api >= 9
include $(CLEAR_VARS)
LOCAL_MODULE := libcooleye9
LOCAL_CFLAGS += -O2 -Wall -DBUILD_STANDALONE -DCPU_ARM -DAVSREMOTE -finline-functions -fPIC -D__ARM_EABI__=1 -DOLD_LOGDH
LOCAL_CFLAGS += -DBUILD_GINGER
LOCAL_C_INCLUDES += $(LOCAL_PATH)/Android/include
LOCAL_SRC_FILES := audio_android.cpp
LOCAL_ARM_MODE := arm
LOCAL_LDLIBS := -llog \
 $(LOCAL_PATH)/Android/lib/gingerbread/libutils.so $(LOCAL_PATH)/Android/lib/gingerbread/libmedia.so
include $(BUILD_SHARED_LIBRARY)
