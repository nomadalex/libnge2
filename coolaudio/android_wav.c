/**
 * @file  android_wav.c
 * @author Kun Wang <ifreedom.cn@gmail.com>
 * @date 2012/10/25 14:31:47
 *
 *  Copyright  2012  Kun Wang <ifreedom.cn@gmail.com>
 *
 */
#include <string.h>
#include <jni.h>
#include "nge_android_jni.h"
#include "audio_android.h"

#include "audio_interface.h"

#include "android_wav.h"

#include "android_rawaudio.h"

#include "nge_endian_internal.h"
#include "nge_io_file.h"

typedef unsigned char BOOLEAN;
#define FALSE 0
#define TRUE 1

#define WARN_UN_IMP() LOGI("%s not implementation!\n", __FUNCTION__)

#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164

#define FORMAT_PCM 1

struct wav_header {
	uint8_t riff_id[4];
	uint32_t riff_sz;
	uint8_t riff_fmt[4];
	uint8_t fmt_id[4];
	uint32_t fmt_sz;
	uint16_t audio_format;
	uint16_t num_channels;
	uint32_t sample_rate;
	uint32_t byte_rate;       /* sample_rate * num_channels * bps / 8 */
	uint16_t block_align;     /* num_channels * bps / 8 */
	uint16_t bits_per_sample;
};

struct wav_data_hdr {
	uint8_t data_id[4];
	uint32_t data_sz;
};

typedef struct
{
	uint32_t rate;
	uint16_t channels;
	uint16_t bps;

    uint32_t size;
} wav_info_t;

inline static int parse_wav_hdr(const char* buf, int size, int *pos, wav_info_t* data) {
    struct wav_header hdr;
	struct wav_data_hdr d_hdr;
	DECL_ENDIAN();
	int offset = *pos;

	SET_ENDIAN(OP_LITTLEENDIAN);
	if ((size - offset) < (int)sizeof(hdr))
		return -1;
	memcpy(&hdr, buf+offset, sizeof(hdr));
	offset += sizeof(hdr);

	DO_ENDIAN(uint32_t, &hdr.fmt_sz);
	DO_ENDIAN(uint16_t, &hdr.audio_format);

    if ((memcmp(hdr.riff_id, "RIFF", 4) != 0) ||
        (memcmp(hdr.riff_fmt, "WAVE", 4) != 0) ||
        (memcmp(hdr.fmt_id, "fmt ", 4) != 0) ||
		(hdr.fmt_sz != 16) ||
		(hdr.audio_format != FORMAT_PCM)) {
        return -1;
    }

	if ((size - offset) < (int)sizeof(d_hdr))
		return -1;
	memcpy(&d_hdr, buf+offset, sizeof(d_hdr));
	offset += sizeof(d_hdr);

	if (memcmp(d_hdr.data_id, "data", 4) != 0)
		return -1;

	DO_ENDIAN(uint16_t, &hdr.num_channels);
	DO_ENDIAN(uint32_t, &hdr.sample_rate);
	DO_ENDIAN(uint16_t, &hdr.bits_per_sample);
	DO_ENDIAN(uint32_t, &d_hdr.data_sz);

    data->channels = hdr.num_channels;
    data->rate =  hdr.sample_rate;
    data->bps = hdr.bits_per_sample;

	data->size = d_hdr.data_sz;

	*pos = offset;

	return 0;
}

typedef struct {
	audio_play_t op;

	void* obj;
	wav_info_t info;
	int maxFrames;
	BOOLEAN isEof;
} android_wav;

#define _METHOD(method) wav_##method

#define DECL_METHOD(ret, method, arg) static ret _METHOD(method) arg
#define MAKE_METHOD(ret, method, arg) ret _METHOD(method) arg

#define THIS_DEF android_wav* This

DECL_METHOD(int, load, (THIS_DEF, const char* filename));
DECL_METHOD(int, load_buf, (THIS_DEF, const char* buf, int size));
DECL_METHOD(int, load_fp, (THIS_DEF, int handle, char closed_by_me));
DECL_METHOD(int, play, (THIS_DEF, int times, int free_when_stop));
DECL_METHOD(int, playstop, (THIS_DEF));
DECL_METHOD(void, pause, (THIS_DEF));
DECL_METHOD(int, stop, (THIS_DEF));
DECL_METHOD(void, resume, (THIS_DEF));
DECL_METHOD(int, volume, (THIS_DEF, int volume));
DECL_METHOD(void, rewind, (THIS_DEF));
DECL_METHOD(void, seek, (THIS_DEF, int ms, int flag));
DECL_METHOD(int, iseof, (THIS_DEF));
DECL_METHOD(int, ispaused, (THIS_DEF));
DECL_METHOD(int, destroy, (THIS_DEF));

void load_wav()
{
	ngeRA_init();
}

void release_wav()
{
	ngeRA_release();
}

audio_play_p android_wav_create()
{
	android_wav* a = (android_wav*)malloc(sizeof(android_wav));

#define SET_METHOD(m) a->op.m = (fd_##m)wav_##m
	SET_METHOD(load);
	SET_METHOD(load_buf);
	SET_METHOD(load_fp);
	SET_METHOD(play);
	SET_METHOD(playstop);
	SET_METHOD(pause);
	SET_METHOD(stop);
	SET_METHOD(resume);
	SET_METHOD(volume);
	SET_METHOD(rewind);
	SET_METHOD(seek);
	SET_METHOD(iseof);
	SET_METHOD(ispaused);
	SET_METHOD(destroy);
#undef SET_METHOD

	a->obj = NULL;
	a->isEof = FALSE;

	return (audio_play_p)a;
}

static inline void exactStop(void* obj)
{
	int state;

	state = ngeRA_getPlayState(obj);

	if (state == NGE_RA_STATE_PAUSED || state == NGE_RA_STATE_PLAYING)
	{
		ngeRA_stop(obj);
	}
}

MAKE_METHOD(int, load, (THIS_DEF, const char* filename)) {
	int fd = io_fopen(filename, IO_RDONLY);
	int ret = _METHOD(load_fp)(This, fd, TRUE);
	LOGI("Load %s\n", filename);
	return ret;
}

MAKE_METHOD(int, load_buf, (THIS_DEF, const char* buf, int size)) {
	wav_info_t* info = &This->info;
	int channel, format, pos = 0;

	if (parse_wav_hdr(buf, size, &pos, info) < 0)
		goto error;

	if (info->bps == 8)
		format = NGE_RA_FORMAT_PCM_8;
	else if (info->bps == 16)
		format = NGE_RA_FORMAT_PCM_16;
	else
		goto error;

	if (info->channels == 1)
		channel = NGE_RA_CHANNEL_MONO;
	else if (info->channels == 2)
		channel = NGE_RA_CHANNEL_STEREO;
	else
		goto error;

	if (This->obj != NULL)
	{
		exactStop(This->obj);
		ngeRA_delete(This->obj);
	}

	This->obj = ngeRA_new(info->rate, channel, format, info->size, NGE_RA_MODE_STATIC);

	LOGI("rate %d, channel %d, format %d, size %d\n",
		 info->rate, channel, format, info->size);

	This->maxFrames = info->size/(info->bps/8)/info->channels;

	LOGI("Write buf %p size %d MaxFrames %d\n", buf+pos, info->size, This->maxFrames);
	ngeRA_write(This->obj, buf+pos, 0, info->size);

	return 0;

error:
	return -1;
}

MAKE_METHOD(int, load_fp, (THIS_DEF, int fd, char closed_by_me)) {
	int size = io_fsize(fd);
	char* buf = (char*)malloc(size);
	int ret;

	io_fread(buf, size, 1, fd);
	if (closed_by_me == 1)
		io_fclose(fd);

	ret = _METHOD(load_buf)(This, buf, size);
	if (ret < 0)
		free(buf);
	return ret;
}

MAKE_METHOD(int, play, (THIS_DEF, int times, int free_when_stop)) {
	int state;

	state = ngeRA_getPlayState(This->obj);
	if (state == NGE_RA_STATE_PLAYING) {
		exactStop(This->obj);
		ngeRA_reloadStaticData(This->obj);
	}

	/* 0 is loop forever */
	ngeRA_setLoopPoints(This->obj, 0, This->maxFrames, times-1);
	ngeRA_play(This->obj);
	LOGI("wav playing play %d times, state %d\n", times, state);

	This->isEof = FALSE;

	return 0;
}

MAKE_METHOD(int, playstop, (THIS_DEF)) {
	return _METHOD(play)(This, 1, 0);
}

MAKE_METHOD(void, pause, (THIS_DEF)) {
	ngeRA_pause(This->obj);
}

MAKE_METHOD(int, stop, (THIS_DEF)) {
	exactStop(This->obj);
	return 0;
}

MAKE_METHOD(void, resume, (THIS_DEF)) {
	ngeRA_play(This->obj);
}

MAKE_METHOD(int, volume, (THIS_DEF, int volume)) {
	float v;

	v = ngeRA_getVolume(This->obj);
	ngeRA_setVolume(This->obj, (float)volume/128);

	return (int)(v*128);
}

MAKE_METHOD(void, rewind, (THIS_DEF)) {
	exactStop(This->obj);
	ngeRA_reloadStaticData(This->obj);
	ngeRA_play(This->obj);
}

MAKE_METHOD(void, seek, (THIS_DEF, int ms, int flag)) {
	wav_info_t* info = &This->info;
	int offset = info->rate * ms / 1000;
	int cur;
	int maxFrames = info->size/(info->bps/8)/info->channels;

	ngeRA_pause(This->obj);

	switch (flag) {
	case AUDIO_SEEK_SET:
		goto seek;
	case AUDIO_SEEK_CUR:
		cur = ngeRA_getPosition(This->obj);
		offset += cur;
		goto seek;
	case AUDIO_SEEK_END:
		offset = maxFrames - offset;
		goto seek;
	}
	goto play;

seek:
	if (offset < 0)
		offset = 0;
	else if (offset > maxFrames)
		offset = maxFrames;

	ngeRA_setPosition(This->obj, offset);

play:
	ngeRA_play(This->obj);
}

MAKE_METHOD(int, iseof, (THIS_DEF)) {
	int pos;

	if (This->isEof) return This->isEof;

	pos = ngeRA_getPosition(This->obj);
	if (pos >= This->maxFrames)
	{
		This->isEof = TRUE;
	}
	LOGI("Position %d\n", pos);
	return This->isEof;
}

MAKE_METHOD(int, ispaused, (THIS_DEF)) {
	int state;

	state = ngeRA_getPlayState(This->obj);
	return state == NGE_RA_STATE_PAUSED;
}

MAKE_METHOD(int, destroy, (THIS_DEF)) {
	exactStop(This->obj);
	ngeRA_delete(This->obj);
	free(This);

	return 0;
}
