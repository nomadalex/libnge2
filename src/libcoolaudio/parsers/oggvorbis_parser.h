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

#ifndef __SIRENS2_OGGVORBIS_PARSER_H__
#define __SIRENS2_OGGVORBIS_PARSER_H__

#include "audio_parser.h"
#include "common/buffered_reader.h"

class OggVorbisParser : public AudioParser {
private:
	void* ogg_vorbis_file;
	AudioCodecInitData init_data;
	bool is_open;
	uint32_t total_samples;
	buffered_reader_t* reader;
	
	void *output_buffer;
	
	void initialize();
	
	void finalize();

public:
	OggVorbisParser();
	~OggVorbisParser();
	char* open(const char* filename);
	void close();
	
	uint32_t get_samplerate();
	uint32_t get_channels();
	uint64_t get_duration();
	uint32_t get_samples_per_frame();
	uint32_t get_max_samples_per_frame();
	
	uint64_t seek_time(uint64_t time);
	char* get_frame(void** pcm, uint32_t* samples);

};

#endif
