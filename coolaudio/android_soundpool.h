/**
 * @file  android_soundpool.h
 * @author Kun Wang <ifreedom.cn@gmail.com>
 * @date 2012/11/14 15:01:18
 *
 *  Copyright  2012  Kun Wang <ifreedom.cn@gmail.com>
 *
 */

#ifndef _ANDROID_SOUNDPOOL_H
#define _ANDROID_SOUNDPOOL_H

#ifdef __cplusplus
extern "C" {
#endif

	void load_SP();
	void release_SP();
	audio_play_p android_soundpool_create();

#ifdef __cplusplus
}
#endif

#endif /* _ANDROID_SOUNDPOOL_H */
