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
 
#include "mp3_parser.h"
#include "common/mem64.h"
#include <string.h>

static uint32_t mp3_samplerates[3][4] = {
	{44100, 48000, 32000, 0},
	{22050, 24000, 16000, 0},
	{11025, 12000, 8000, 0}
};
static uint32_t mp3_bitrates[9][16] = {
	{0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 0},
	{0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384, 0},
	{0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 0},
	{0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256, 0},
	{0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 0},
	{0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 0},
	{0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256, 0},
	{0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 0},
	{0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 0}
};
static uint32_t mp3_samples_per_frames[9] = {384, 1152, 1152, 384, 1152, 576, 384, 1152, 576};

Mp3Parser::Mp3Parser() {
	initialize();
};

Mp3Parser::~Mp3Parser() {
	close();
};

void Mp3Parser::initialize() {
	is_open = false;
	
	total_frames = 0;
	total_samples = 0;
	current_frame = 0;
	
	frame_index = 0;
	reader = 0;
	output_buffer = 0;
	read_buffer = 0;
	
	codec = 0;
	codec_initialize = false;
	
	memset(&init_data, 0, sizeof(AudioCodecInitData));
};

void Mp3Parser::finalize() {
	if ( reader ) {
		buffered_reader_close(reader);
	}
	if ( frame_index ) {
		free(frame_index);
	}
	if ( output_buffer ) {
		free(output_buffer);
	}
	if ( read_buffer ) {
		free(read_buffer);
	}
	if ( codec ) {
		if ( codec_initialize )
			codec->finalize();
		delete codec;
	}
	initialize();
};


char* Mp3Parser::skip_id3v2_and_get_header_info() {
	uint8_t mp3_header_buffer[4];
	if ( buffered_reader_read(reader, mp3_header_buffer, 4) != 4 ) {
		finalize();
		return "Mp3Parser : read MP3 header fail";
	}
	while(1) {
		if ( mp3_header_buffer[0] == 'I' && mp3_header_buffer[1] == 'D' && mp3_header_buffer[2] == '3' ) {
			uint8_t id3v2_buffer[6];
			if ( buffered_reader_read(reader, id3v2_buffer, 6) != 6 ) {
				finalize();
				return "Mp3Parser : read ID3v2 fail";
			}
			uint32_t id3v2_size, temp_value;
			temp_value = id3v2_buffer[2];
			temp_value = ( temp_value & 0x7F ) << 21;
			id3v2_size = temp_value;
			temp_value = id3v2_buffer[3];
			temp_value = ( temp_value & 0x7F ) << 14;
			id3v2_size = id3v2_size | temp_value;
			temp_value = id3v2_buffer[4];
			temp_value = ( temp_value & 0x7F ) << 7;
			id3v2_size = id3v2_size | temp_value;
			temp_value = id3v2_buffer[5];
			temp_value = ( temp_value & 0x7F ) ;
			id3v2_size = id3v2_size | temp_value;
			buffered_reader_seek(reader, buffered_reader_position(reader)+id3v2_size);
			if ( buffered_reader_read(reader, mp3_header_buffer, 4) != 4 ) {
				finalize();
				return "Mp3Parser : read MP3 header fail";
			}
			continue;
		}
		uint32_t mp3_header = mp3_header_buffer[0];
		mp3_header = (mp3_header<<8) | mp3_header_buffer[1];
		mp3_header = (mp3_header<<8) | mp3_header_buffer[2];
		mp3_header = (mp3_header<<8) | mp3_header_buffer[3];
		
		if ( ((mp3_header >> 21) & 0x7FF) != 0x7FF) {
			buffered_reader_seek(reader, buffered_reader_position(reader)-3);
			if ( buffered_reader_read(reader, mp3_header_buffer, 4 ) != 4 ) {
				finalize();
				return "Mp3Parser : read MP3 header fail";
			}
			continue;
		}
		switch((mp3_header_buffer[1] >> 3) & 0x03) {
			case 0:
				mp3_version = 2; // MPEG 2.5
				break;
			case 2:
				mp3_version = 1; // MPEG 2
				break;
			case 3:
				mp3_version = 0; // MPEG 1
				break;
			default:
				mp3_version = 0;
				break;
		}
		mp3_level = 3 - ((mp3_header_buffer[1] >> 1) & 0x03);
		if ( mp3_level == 3 ) 
			mp3_level--;
			
		init_data.samplerate = (mp3_header & 0x0C00) >> 10;
		init_data.samplerate = mp3_samplerates[mp3_version][init_data.samplerate];
	
		init_data.channels = (mp3_header & 0x00C0) >> 6;
		init_data.channels = (init_data.channels == 0x3)?1:2;
		
		init_data.sample_bits = 0x10;
		init_data.samples_per_frame = mp3_samples_per_frames[mp3_version*3+mp3_level];
		init_data.samples_per_decoded = init_data.samples_per_frame;
	
		mp3_data_start = buffered_reader_seek(reader, buffered_reader_position(reader)-4);
		break;
	}
	return 0;	
};

char* Mp3Parser::open(const char* filename) {
	
	close();
	
	reader = buffered_reader_open(filename, 65536, 1);
	if ( !reader ) {
		finalize();
		return "Mp3Parser : open buffered_reader fail";
	}
	
	char* result;
	buffered_reader_seek(reader, 0);
	result = skip_id3v2_and_get_header_info();
	if ( result != 0 )
		return result;
		
	uint32_t file_size = buffered_reader_length(reader);
	buffered_reader_seek(reader, mp3_data_start);
	uint32_t max_frames = (file_size / 0x1000 + 1) * (init_data.samplerate / init_data.samples_per_frame + 1);
	
	frame_index = (AudioFrameIndex*)malloc_64(max_frames * sizeof(AudioFrameIndex));
	if ( frame_index == 0 ) {
		finalize();
		return "Mp3Parser : malloc seek_table fail";
	}   
	
	uint8_t mp3_header_buffer[4];
	total_frames = 0;
	init_data.frame_align = 0;
	while(1) {
		if ( buffered_reader_read(reader, mp3_header_buffer, 4) != 4 ) {
			break;
		}
		uint32_t mp3_header = mp3_header_buffer[0];
		mp3_header = (mp3_header<<8) | mp3_header_buffer[1];
		mp3_header = (mp3_header<<8) | mp3_header_buffer[2];
		mp3_header = (mp3_header<<8) | mp3_header_buffer[3];
		if ( ((mp3_header >> 21) & 0x7FF) != 0x7FF)  {
			break;
		}
		uint32_t bitrate = (mp3_header & 0xf000) >> 12;
		uint32_t padding = (mp3_header & 0x200) >> 9;
		
		uint32_t frame_size;
		if ( mp3_level == 0 )
			frame_size = (init_data.samples_per_frame/8*1000)*mp3_bitrates[mp3_version*3+mp3_level][bitrate]/init_data.samplerate + padding<<2;
		else
		 	frame_size = (init_data.samples_per_frame/8*1000)*mp3_bitrates[mp3_version*3+mp3_level][bitrate]/init_data.samplerate + padding;
		
		if ( total_frames == 0 ) {
			frame_index[total_frames].frame_position = mp3_data_start;
			frame_index[total_frames].frame_size = frame_size;
		}
		else {
			frame_index[total_frames].frame_position = frame_index[total_frames-1].frame_position + frame_index[total_frames-1].frame_size;
			frame_index[total_frames].frame_size = frame_size;
		}		
		total_frames++;
		if ( frame_size > init_data.frame_align )
			init_data.frame_align = frame_size;
		buffered_reader_seek(reader, buffered_reader_position(reader)+frame_size-4);
	}
	if ( total_frames == 0 ) {
		finalize();
		return "Mp3Parser : total_frames = 0";
	}
	buffered_reader_seek(reader, mp3_data_start);
	total_samples = total_frames * init_data.samples_per_frame;
	
	output_buffer = malloc_64(init_data.samples_per_frame<<2);
	if ( output_buffer == 0 ) {
		finalize();
		return "Mp3Parser : malloc read_buffer fail";
	}
	
	read_buffer = malloc_64(init_data.frame_align);
	if ( read_buffer == 0 ) {
		finalize();
		return "Mp3Parser : malloc output_buffer fail";
	}
	
	codec = AudioCodec::create_audio_codec(AudioCodec::MP3);
	if ( !codec ) {
		finalize();
		return "Mp3Parser : create audio codec fail";
	}
	result = codec->initialize(&init_data);
	if ( result ) {
		finalize();
		return result;
	}
	codec_initialize = true;
	
	is_open = true;
	return 0;
};

char* Mp3Parser::open_cb(audio_callbacks cb,int handle) {
	
	close();
	
	reader = buffered_reader_open_cb(cb,handle, 65536, 1);
	if ( !reader ) {
		finalize();
		return "Mp3Parser : open buffered_reader fail";
	}
	
	char* result;
	buffered_reader_seek(reader, 0);
	result = skip_id3v2_and_get_header_info();
	if ( result != 0 )
		return result;
		
	uint32_t file_size = buffered_reader_length(reader);
	buffered_reader_seek(reader, mp3_data_start);
	uint32_t max_frames = (file_size / 0x1000 + 1) * (init_data.samplerate / init_data.samples_per_frame + 1);
	
	frame_index = (AudioFrameIndex*)malloc_64(max_frames * sizeof(AudioFrameIndex));
	if ( frame_index == 0 ) {
		finalize();
		return "Mp3Parser : malloc seek_table fail";
	}   
	
	uint8_t mp3_header_buffer[4];
	total_frames = 0;
	init_data.frame_align = 0;
	while(1) {
		if ( buffered_reader_read(reader, mp3_header_buffer, 4) != 4 ) {
			break;
		}
		uint32_t mp3_header = mp3_header_buffer[0];
		mp3_header = (mp3_header<<8) | mp3_header_buffer[1];
		mp3_header = (mp3_header<<8) | mp3_header_buffer[2];
		mp3_header = (mp3_header<<8) | mp3_header_buffer[3];
		if ( ((mp3_header >> 21) & 0x7FF) != 0x7FF)  {
			break;
		}
		uint32_t bitrate = (mp3_header & 0xf000) >> 12;
		uint32_t padding = (mp3_header & 0x200) >> 9;
		
		uint32_t frame_size;
		if ( mp3_level == 0 )
			frame_size = (init_data.samples_per_frame/8*1000)*mp3_bitrates[mp3_version*3+mp3_level][bitrate]/init_data.samplerate + padding<<2;
		else
		 	frame_size = (init_data.samples_per_frame/8*1000)*mp3_bitrates[mp3_version*3+mp3_level][bitrate]/init_data.samplerate + padding;
		
		if ( total_frames == 0 ) {
			frame_index[total_frames].frame_position = mp3_data_start;
			frame_index[total_frames].frame_size = frame_size;
		}
		else {
			frame_index[total_frames].frame_position = frame_index[total_frames-1].frame_position + frame_index[total_frames-1].frame_size;
			frame_index[total_frames].frame_size = frame_size;
		}		
		total_frames++;
		if ( frame_size > init_data.frame_align )
			init_data.frame_align = frame_size;
		buffered_reader_seek(reader, buffered_reader_position(reader)+frame_size-4);
	}
	if ( total_frames == 0 ) {
		finalize();
		return "Mp3Parser : total_frames = 0";
	}
	buffered_reader_seek(reader, mp3_data_start);
	total_samples = total_frames * init_data.samples_per_frame;
	
	output_buffer = malloc_64(init_data.samples_per_frame<<2);
	if ( output_buffer == 0 ) {
		finalize();
		return "Mp3Parser : malloc read_buffer fail";
	}
	
	read_buffer = malloc_64(init_data.frame_align);
	if ( read_buffer == 0 ) {
		finalize();
		return "Mp3Parser : malloc output_buffer fail";
	}
	
	codec = AudioCodec::create_audio_codec(AudioCodec::MP3);
	if ( !codec ) {
		finalize();
		return "Mp3Parser : create audio codec fail";
	}
	result = codec->initialize(&init_data);
	if ( result ) {
		finalize();
		return result;
	}
	codec_initialize = true;
	
	is_open = true;
	return 0;
};


void Mp3Parser::close() {
	if ( is_open ) {
		finalize();
	}
};
	
uint32_t Mp3Parser::get_samplerate() {
	return init_data.samplerate;
};

uint32_t Mp3Parser::get_channels() {
	return init_data.channels;
};

uint64_t Mp3Parser::get_duration() {
	uint64_t duration = 1000000LL*total_samples/init_data.samplerate;
	return duration;
};
	
uint32_t Mp3Parser::get_samples_per_frame() {
	return init_data.samples_per_frame;
};

uint32_t Mp3Parser::get_max_samples_per_frame() {
	return init_data.samples_per_frame;
};

uint64_t Mp3Parser::seek_time(uint64_t time) {
	uint32_t samples = (uint32_t)(time*init_data.samplerate/1000000LL);
	if ( samples > total_samples )
		samples = total_samples;
	
	current_frame = samples/init_data.samples_per_frame;
	
	if (current_frame >= total_frames)
		current_frame = total_frames - 1;
	
	buffered_reader_seek(reader, frame_index[current_frame].frame_position);
	
	uint64_t current = 1000000LL*current_frame*init_data.samples_per_frame/init_data.samplerate;
	return current;
};

char* Mp3Parser::get_frame(void** pcm, uint32_t* samples) {
	
	if ( !is_open )
		return "Mp3Parser : parser is not open";
	
	if ( current_frame >= total_frames ) {
		*pcm = output_buffer;
		*samples = 0;
	}
	else {
		memset(read_buffer, 0, init_data.frame_align);
		uint32_t read_size;
		read_size = buffered_reader_read(reader, read_buffer, frame_index[current_frame].frame_size);
		if ( read_size != (frame_index[current_frame].frame_size) ) {
			*pcm = output_buffer;
			*samples = 0;
		}
		else {
			current_frame++;
			memset(output_buffer, 0, init_data.samples_per_frame << 2);
			int32_t get_samples = codec->decode(read_buffer, read_size, output_buffer);
			*samples = (get_samples<0) ? 0 : get_samples;
			*pcm = output_buffer;
		}
	}
	return 0;
};


