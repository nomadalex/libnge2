/**
 * @file  android_log.h
 * @author Kun Wang <ifreedom.cn@gmail.com>
 * @date 2012/05/20 04:57:49
 *
 *  Copyright  2012  Kun Wang <ifreedom.cn@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _ANDROID_LOG___H
#define _ANDROID_LOG___H

#ifndef LOG_TAG
#error "NO Log tag!"
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

#define LOG_ERROR(str)							\
	if(alGetError() != AL_NO_ERROR)				\
	{											\
		LOGE(str);								\
	}

#define LOG_ERROR_RET(str, ret)					\
	if(alGetError() != AL_NO_ERROR)				\
	{											\
		LOGE(str);								\
		return ret;								\
	}

#define LOG_ERROR_GOTO(str)						\
	if(alGetError() != AL_NO_ERROR)				\
	{											\
		LOGE(str);								\
		goto error;								\
	}

#define LOG_FUN_ERR()										\
	LOGE("%s(%d)-%s\n", __FILE__, __LINE__, __FUNCTION__)

#define LOG_ASSERT(expr)							\
	do { if (!(expr)) { LOG_FUN_ERR(); } } while(0)

#endif /* _ANDROID_LOG___H */
