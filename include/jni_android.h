#ifndef JNI_ANDROID_H_
#define JNI_ANDROID_H_
#include <jni.h>
#include "nge_common.h"

#ifdef __cplusplus
extern "C"{
#endif

NGE_API JavaVM* GetJaveVM();

#ifdef __cplusplus
}
#endif

#endif
