/**
 * @file  audio_openal_wav.c
 * @author Kun Wang <ifreedom.cn@gmail.com>
 * @date 2012/03/21 16:39:43
 *
 *  Copyright  2012  Kun Wang <ifreedom.cn@gmail.com>
 *
 */
#include "nge_debug_log.h"
#include "nge_io_file.h"
#include "audio_interface.h"
#include "audio_openal.h"
#include "nge_endian_internal.h"

#include <string.h>
#include <stdlib.h>

#define LOG_ERROR(str)							\
	if(alGetError() != AL_NO_ERROR)				\
	{											\
		nge_log(str);							\
	}

#define LOG_ERROR_RET(str, ret)					\
	if(alGetError() != AL_NO_ERROR)				\
	{											\
		nge_log(str);							\
		return ret;								\
	}

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

DECL_INIT_ENDIAN(OP_LITTLEENDIAN);

inline static int parse_wav_hdr(const char* buf, int size, int *pos, wav_info_t* data) {
    struct wav_header hdr;
	struct wav_data_hdr d_hdr;
	int offset = *pos;

	if ((size - offset) < sizeof(hdr))
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

	if ((size - offset) < sizeof(d_hdr))
		return -1;
	memcpy(&d_hdr, buf+offset, sizeof(d_hdr));
	offset += sizeof(d_hdr);

	if (memcmp(d_hdr.data_id, "data") != 0)
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
	IPlayer alPlayer;

	ALuint source;
	ALuint buffer;
	wav_info_t* info;
	int times;
	BOOLEAN isEof;
} AudioOpenALWav;

void WavCheckUpdate(IPlayer* player);
PlayerOperation WavPlayerOperation = {
	WavCheckUpdate
};

#define THIS_DEF audio_play_t* This
#define GET_AUDIO_FROM_THIS(var) AudioOpenALWav* var = (AudioOpenALWav*)(This)
#define GET_AUDIO_FROM_IPLAYER(var, handle) AudioOpenALWav* var = (AudioOpenALWav*)((char*)handle - sizeof(audio_play_t))

#define DECL_METHOD(ret, method, arg) static ret wav_##method arg

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

#undef DECL_METHOD

audio_play_p CreateWavPlayer() {
	AudioOpenALWav* a = (AudioOpenALWav*)malloc(sizeof(*a));

#define SET_METHOD(m) a->op.m = wav_##m
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

	a->alPlayer.op = &WavPlayerOperation;

	alGenSources(1, &a->source);
	LOG_ERROR_RET("Could not create source\n", NULL);

    /* Set parameters so mono sources play out the front-center speaker and
     * won't distance attenuate. */
    alSource3i(a->source, AL_POSITION, 0, 0, -1);
    alSourcei(a->source, AL_SOURCE_RELATIVE, AL_TRUE);
    alSourcei(a->source, AL_ROLLOFF_FACTOR, 0);
	LOG_ERROR_RET("Could not set source parameters\n", NULL);

	al->buffer = 0;

	a->info = NULL;
	a->times = 0;
	a->isEof = FALSE;

	return &(a->op);
}

static void audio_callback(THIS_DEF) {
	GET_AUDIO_FROM_THIS(audio);

	if (audio->times == 0) /* loop? */ {
		goto replay;
	}
	else if (audio->times == 1) {
		audio->isEof = TRUE;
		wav_stop(This);
		return;
	}
	else {
		this->times--;
		goto replay;
	}

replay:
	alSourceRewind(audio->source);
	alSourcePlay(audio->source);
}

#define _METHOD(ret, method, arg) ret wav_##method arg

_METHOD(int, load, (THIS_DEF, const char* filename)) {
	int fd = io_fopen(filename, IO_RDONLY);
	return wav_load_fp(This, fd, 1);
}

_METHOD(int, load_buf, (THIS_DEF, const char* buf, int size)) {
	GET_AUDIO_FROM_THIS(audio);
	wav_info_t *info = (wav_info_t*)malloc(sizeof(*info));
	int format, pos;

	audio->info = info;

	if (parse_wav_hdr(buf, size, &pos, info) < 0)
		goto error;

	if (info->channels == 1) {
		if (info->bps == 8)
			format = AL_FORMAT_MONO8;
		else if (info->bps == 16)
			format = AL_FORMAT_MONO16;
		else
			goto error;
	}
	else if (info->channels == 2) {
		if (info->bps == 8)
			format = AL_FORMAT_STEREO8;
		else if (info->bps == 16)
			format = AL_FORMAT_STEREO16;
		else
			goto error;
	}
	else
		goto error;

	if (audio->buffer != 0) {
		alDeleteBuffers(1, &audio->buffer);
	}
	alGenBuffers(1, &a->buffer);
	LOG_ERROR_RET("Could not create buffers\n", 0);

	alBufferData(audio->buffer, format, buf+pos, info->size, info->rate);
	LOG_ERROR_RET("Error buffering data\n", 0);

	alSourcei(audio->source, AL_BUFFER, audio->buffer);

	return 0;

error:
	free(info);
	return -1;
}

_METHOD(int, load_fp, (THIS_DEF, int fd, char closed_by_me)) {
	int size = io_fsize(fd);
	char* buf = (char*)malloc(size);
	int ret;

	io_fread(buf, size, 1, fd);
	if (closed_by_me == 1)
		io_fclose(fd);

	ret = wav_load_buf(this, buf, size);
	if (ret < 0)
		free(buf);
	return ret;
}

#define CHECK_BUFFER(var)						\
	if (var->buffer == 0)						\
		return -1

#define CHECK_BUFFER_V(var)						\
	if (var->buffer == 0)						\
		return

_METHOD(int, play, (THIS_DEF, int times, int free_when_stop)) {
	GET_AUDIO_FROM_THIS(audio);
	CHECK_BUFFER(audio);

	audio->times = times;

	audio->isEof = FALSE;

	alSourcePlay(audio->source);
	AddActivePlayer(&audio->alPlayer);

	return 0;
}

_METHOD(int, playstop, (THIS_DEF)) {
	return wav_play(This, 1, 0);
}

_METHOD(void, pause, (THIS_DEF)) {
	GET_AUDIO_FROM_THIS(audio);
	CHECK_BUFFER_V(audio);

	RemoveActivePlayer(&audio->alPlayer);
	alSourcePause(audio->source);
}

_METHOD(int, stop, (THIS_DEF)) {
	GET_AUDIO_FROM_THIS(audio);
	CHECK_BUFFER(audio);

	RemoveActivePlayer(&audio->alPlayer);
	alSourceStop(audio->source);

	return 0;
}

_METHOD(void, resume, (THIS_DEF)) {
	GET_AUDIO_FROM_THIS(audio);
	CHECK_BUFFER_V(audio);

	alSourcePlay(audio->source);
	AddActivePlayer(&audio->alPlayer);
}

_METHOD(int, volume, (THIS_DEF, int volume)) {
	GET_AUDIO_FROM_THIS(audio);
	float v;

	alGetSourcef(audio->source, AL_GAIN, &v);
	alSourcef(audio->source, AL_GAIN, (float)volume/128);
	return (int)(v*128);
}

_METHOD(void, rewind, (THIS_DEF)) {
	GET_AUDIO_FROM_THIS(audio);
	CHECK_BUFFER_V(audio);

	alSourceRewind(audio->source);
}

_METHOD(void, seek, (THIS_DEF, int ms, int flag)) {
	GET_AUDIO_FROM_THIS(audio);
	struct wav_info* info = audio->info;
	ALint offset = (info->bps/8) * info->channels * ms * (info->rate/1000);
	ALint cur;

	CHECK_BUFFER_V(audio);

	alSourcePause(audio->source);
	switch (flag) {
	case AUDIO_SEEK_SET:
		goto seek;
	case AUDIO_SEEK_CUR:
		alGetSourcei(audio->source, AL_BYTE_OFFSET, &cur);
		offset += cur;
		goto seek;
	case AUDIO_SEEK_END:
		offset = info->size - offset;
		goto seek;
	}
	goto play;

seek:
	if (offset < 0)
		offset = 0;
	else if (offset > info->size)
		offset = info->size;

	alSourcei(audio->source, AL_BYTE_OFFSET, offset);

play:
	alSourcePlay(audio->source);
}

_METHOD(int, iseof, (THIS_DEF)) {
	GET_AUDIO_FROM_THIS(audio);
	return audio->isEof;
}

_METHOD(int, ispaused, (THIS_DEF)) {
	GET_AUDIO_FROM_THIS(audio);
	ALint state = 0;

	CHECK_BUFFER(audio);

	alGetSourcei(audio->source, AL_SOURCE_STATE, &state);
	return state == AL_PAUSED ? TRUE : FALSE;
}

_METHOD(int, destroy, (THIS_DEF)) {
	GET_AUDIO_FROM_THIS(audio);

	alDeleteSources(1, &audio->source);
	if (audio->buffer != 0)
		alDeleteBuffers(1, &audio->buffer);

	free(audio->info);
	free(audio);

	return 0;
}

#undef _METHOD

void WavCheckUpdate(IPlayer* player) {
	GET_AUDIO_FROM_IPLAYER(audio, player);
	ALint state = 0;

	alGetSourcei(audio->source, AL_SOURCE_STATE, &state);
	if (state == AL_STOPPED)
		audio_callback(&audio->op);
}
