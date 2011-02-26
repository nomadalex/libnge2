/*
 *	 _____  _                                 _____
 *	/  ___|(_)                               / __  \
 *	\ `--.  _  _ __  ___  _ __   ___  __   __`' / /'
 *	 `--. \| || '__|/ _ \| '_ \ / __| \ \ / /  / /
 *	/\__/ /| || |  |  __/| | | |\__ \  \ V / ./ /___
 *	\____/ |_||_|   \___||_| |_||___/   \_/  \_____/
 *
 *
 *	Copyright (C) 2007 cooleyes
 *	eyes.cooleyes@gmail.com
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */
#include "mp3_codec.h"
#include <stdlib.h>
#include <string.h>
#include <pspaudiocodec.h>

Mp3Codec::Mp3Codec() {
	init_flag = false;
};

Mp3Codec::~Mp3Codec() {
};

char* Mp3Codec::initialize(AudioCodecInitData* init_data) {
	audio_samplerate = init_data->samplerate;
	audio_samples_per_frame = init_data->samples_per_frame;
	audio_type = 0x1002;

	memset(me_audio_codec_buffer, 0, 65*sizeof(uint32_t));
	if ( sceAudiocodecCheckNeedMem((long unsigned int*) me_audio_codec_buffer, audio_type) < 0 )
		return "Mp3Codec : sceAudiocodecCheckNeedMem fail";
	if ( sceAudiocodecGetEDRAM((long unsigned int*)me_audio_codec_buffer, audio_type) < 0 )
		return "Mp3Codec : sceAudiocodecGetEDRAM fail";
	if ( sceAudiocodecInit((long unsigned int*)me_audio_codec_buffer, audio_type) < 0 ) {
		sceAudiocodecReleaseEDRAM((long unsigned int*)me_audio_codec_buffer);
		return "Mp3Codec : sceAudiocodecInit fail";
	}
	init_flag = true;
	return 0;
};

void Mp3Codec::finalize() {
	if( init_flag ) {
		sceAudiocodecReleaseEDRAM((long unsigned int*)me_audio_codec_buffer);
		init_flag = false;
	}
};

int32_t Mp3Codec::decode(void* data, uint32_t data_len, void* pcm_data) {
	if( init_flag ) {
		me_audio_codec_buffer[6] = (uint32_t)data;
		me_audio_codec_buffer[8] = (uint32_t)pcm_data;
		me_audio_codec_buffer[7] = data_len;
		me_audio_codec_buffer[10] = data_len;
		me_audio_codec_buffer[9] = audio_samples_per_frame << 2;
		if ( sceAudiocodecDecode((long unsigned int*)me_audio_codec_buffer, audio_type) < 0 )
			;//return 0;
		return audio_samples_per_frame;
	}
	else
		return -1;
};
