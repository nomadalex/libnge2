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

#ifndef __SIRENS2_MP3_CODEC_H__
#define __SIRENS2_MP3_CODEC_H__

#include "audio_codec.h"



class Mp3Codec : public AudioCodec {
private:
	uint32_t me_audio_codec_buffer[65] __attribute__((aligned(64)));
	uint32_t audio_type;
	uint32_t audio_samplerate;
	uint32_t audio_samples_per_frame;
	
	bool init_flag;
public:
	Mp3Codec();
	~Mp3Codec();
	char* initialize(AudioCodecInitData* init_data);
	void finalize();
	int32_t decode(void* data, uint32_t data_len, void* pcm_data);
};

#endif
