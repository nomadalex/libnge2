/**
 * @file  android_wav.h
 * @author Kun Wang <ifreedom.cn@gmail.com>
 * @date 2012/10/25 14:31:37
 *
 *  Copyright  2012  Kun Wang <ifreedom.cn@gmail.com>
 *
 */

#ifndef _ANDROID_WAV_H
#define _ANDROID_WAV_H

#ifdef __cplusplus
extern "C" {
#endif

	void load_wav();
	void release_wav();
	audio_play_p android_wav_create();

#ifdef __cplusplus
}
#endif

#endif /* _ANDROID_WAV_H */
