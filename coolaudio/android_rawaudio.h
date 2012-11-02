/**
 * @file  android_rawaudio.h
 * @author Kun Wang <ifreedom.cn@gmail.com>
 * @date 2012/10/24 15:12:55
 *
 *  Copyright  2012  Kun Wang <ifreedom.cn@gmail.com>
 *
 */

#ifndef _ANDROID_RAWAUDIO_H
#define _ANDROID_RAWAUDIO_H

#define NGE_RA_MODE_STATIC 0
#define NGE_RA_MODE_STREAM 1

#define NGE_RA_CHANNEL_MONO 4
#define NGE_RA_CHANNEL_STEREO 12

#define NGE_RA_FORMAT_PCM_8 3
#define NGE_RA_FORMAT_PCM_16 2

#define NGE_RA_STATE_STOPPED 1
#define NGE_RA_STATE_PAUSED 2
#define NGE_RA_STATE_PLAYING 3

typedef void (*ngeRA_notify_fn)(void* pCookie);

#ifdef __cplusplus
extern "C" {
#endif

	void ngeRA_init();
	void ngeRA_release();

	int ngeRA_getMinBufferSize(int sampleRateInHz, int channelConfig, int audioFormat);

	void* ngeRA_new(int sampleRateInHz, int channelConfig, int audioFormat, int bufferSizeInBytes, int mode);
	void ngeRA_delete(void* audio);

	void ngeRA_play(void* audio);
	void ngeRA_stop(void* audio);
	void ngeRA_pause(void* audio);

	int ngeRA_getPlayState(void* audio);
	int ngeRA_reloadStaticData(void* audio);
	int ngeRA_setLoopPoints(void* audio, int startInFrames, int endInFrames, int loopCount);

	/* byteArray is java object byte[], for performance */
	int ngeRA_writeArray(void* audio, void* byteArray, int offsetInBytes, int sizeInBytes);
	int ngeRA_write(void* audio, void* pbuffer, int offsetInBytes, int sizeInBytes);

	int ngeRA_setVolume(void* audio, float volume);
	float ngeRA_getVolume(void* audio);

	int ngeRA_setMarker(void* audio, int markerInFrames);
	int ngeRA_getMarker(void* audio);

	int ngeRA_setPosition(void* audio, int frames);
	int ngeRA_getPosition(void* audio);

	void ngeRA_setListener(void* audio, ngeRA_notify_fn cbPtr, void* pCookie);
	void ngeRA_clearListener(void* audio);

#ifdef __cplusplus
}
#endif

#endif /* _ANDROID_RAWAUDIO_H */
