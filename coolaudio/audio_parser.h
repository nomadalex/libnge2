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
 
#ifndef __SIRENS2_AUDIO_PARSER_H__
#define __SIRENS2_AUDIO_PARSER_H__

#include <stdint.h>
#include "common/buffered_reader.h"
#include "audio_codec.h"

typedef struct AudioFrameIndex {
	uint32_t frame_position;
	uint32_t frame_size;
} AudioFrameIndex;

class AudioParser {
public:
	virtual ~AudioParser() {};
	virtual char* open(const char* filename) = 0;
	virtual char* open_cb(audio_callbacks cb,int handle)=0;
	virtual void close() = 0;
	
	virtual uint32_t get_samplerate() = 0;
	virtual uint32_t get_channels() = 0;
	virtual uint64_t get_duration() = 0;
	virtual uint32_t get_samples_per_frame() = 0;
	virtual uint32_t get_max_samples_per_frame() = 0;
	
	virtual uint64_t seek_time(uint64_t time) = 0;
	virtual char* get_frame(void** pcm, uint32_t* samples) = 0;
	
};

#endif
