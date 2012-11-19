/**
 * @file  wav.c
 * @author Kun Wang <ifreedom.cn@gmail.com>
 * @date 2012/10/25 14:31:47
 *
 *  Copyright  2012  Kun Wang <ifreedom.cn@gmail.com>
 *
 */
#include <string.h>
#include "nge_common.h"
#include "wav.h"
#include "nge_endian_internal.h"

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

int parse_wav_hdr(const char* buf, int size, int *pos, wav_info_t* data) {
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
