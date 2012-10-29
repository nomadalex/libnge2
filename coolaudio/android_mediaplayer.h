/**
 * @file  android_mediaplayer.h
 * @author Kun Wang <ifreedom.cn@gmail.com>
 * @date 2012/10/21 14:58:37
 *
 *  Copyright  2012  Kun Wang <ifreedom.cn@gmail.com>
 *
 */

#ifndef _ANDROID_MEDIAPLAYER_H
#define _ANDROID_MEDIAPLAYER_H

#ifdef __cplusplus
extern "C" {
#endif

	void load_CA();
	void release_CA();
	audio_play_p android_mediaplayer_create();

#ifdef __cplusplus
}
#endif

#endif /* _ANDROID_MEDIAPLAYER_H */
