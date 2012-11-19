/**
 * @file  wav.h
 * @author Kun Wang <ifreedom.cn@gmail.com>
 * @date 2012/10/25 14:31:37
 *
 *  Copyright  2012  Kun Wang <ifreedom.cn@gmail.com>
 *
 */

#ifndef _ANDROID_WAV_H
#define _ANDROID_WAV_H

#define MIN_WAV_HEADER_SIZE 44

typedef struct wav_info
{
	uint32_t rate;
	uint16_t channels;
	uint16_t bps;

    uint32_t size;
} wav_info_t;

#ifdef __cplusplus
extern "C" {
#endif

	int parse_wav_hdr(const char* buf, int size, int *pos, wav_info_t* data);

#ifdef __cplusplus
}
#endif

#endif /* _ANDROID_WAV_H */
