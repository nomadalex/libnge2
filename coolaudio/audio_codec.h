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

#ifndef __SIRENS2_AUDIO_CODEC_H__
#define __SIRENS2_AUDIO_CODEC_H__

#include <stdint.h>
//#include "codecs/mp3_codec.h"
typedef struct AudioCodecInitData {
	uint32_t samplerate;
	uint32_t sample_bits;
	uint32_t channels;
	uint32_t samples_per_frame;
	uint32_t samples_per_decoded;
	uint32_t frame_align;
	uint8_t* extra_data;
	uint32_t extra_data_len;
}  AudioCodecInitData;

//class Mp3Codec;

class AudioCodec {
public:
	typedef enum {ATRAC3, MP3, AAC, TTA, WMA} CodecId;
	virtual ~AudioCodec() {};
	virtual char* initialize(AudioCodecInitData* init_data) = 0;
	virtual void finalize() = 0;
	virtual int32_t decode(void* data, uint32_t data_len, void* pcm_data) = 0;
	virtual void reset() {};
	virtual bool frame_decode_finished() {return true;};
	static AudioCodec* create_audio_codec(CodecId id);	
};

#endif
