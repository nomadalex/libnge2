/**
 * @file  audio_android.h
 * @author Kun Wang <ifreedom.cn@gmail.com>
 * @date 2012/10/21 14:52:37
 *
 *  Copyright  2012  Kun Wang <ifreedom.cn@gmail.com>
 *
 */

#ifndef _AUDIO_ANDROID_H
#define _AUDIO_ANDROID_H

#include "audio_interface.h"
#include <android/log.h>

#define FALSE 0
#define TRUE 1

#define  LOG_TAG    "libcoolaudio"

#define LOG_GOTO_ERROR(str)	LOGE(str); goto error

#ifdef __cplusplus
extern "C" {
#endif

	static inline void LOGI(const char* fmt, ...) {
		va_list args;
		va_start(args, fmt);
		__android_log_vprint(ANDROID_LOG_INFO,LOG_TAG, fmt, args);
		va_end(args);
	}
	static inline void LOGE(const char* fmt, ...) {
		va_list args;
		va_start(args, fmt);
		__android_log_vprint(ANDROID_LOG_ERROR,LOG_TAG, fmt, args);
		va_end(args);
	}

	jobject createFD(JNIEnv* env, int fd);

#ifdef __cplusplus
}
#endif

#endif /* _AUDIO_ANDROID_H */
