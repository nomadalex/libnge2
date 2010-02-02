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

#include "pcm_parser.h"
#include "common/mem64.h"
#include <string.h>

#define SIRENS2_PCM_SAMPLES_PER_FRAME 1024


PcmParser::PcmParser() {
	initialize();
};

PcmParser::~PcmParser() {
	close();
};

void PcmParser::initialize() {
	
	is_open = false;
	
	total_samples = 0;
	current_samples = 0;
	
	reader = 0;
	output_buffer = 0;
	
	memset(&init_data, 0, sizeof(AudioCodecInitData));
};

void PcmParser::finalize() {
	if ( reader ) {
		buffered_reader_close(reader);
	}
	if ( output_buffer ) {
		free(output_buffer);
	}
	initialize();
};

char* PcmParser::open(const char* filename) {
	
	reader = buffered_reader_open(filename, 131072, 1);
	if ( !reader ) {
		finalize();
		return "PcmParser : open buffered_reader fail";
	}
	
	buffered_reader_seek(reader, 0);
	uint8_t header_buffer[20];
	if ( buffered_reader_read(reader, header_buffer, 20) != 20 ) {
		finalize();
		return "PcmParser : read PCM header fail";
	}
	if ( *((uint32_t*)(&header_buffer[0x00])) != 0x46464952 ) { // "RIFF" tag
		finalize();
		return "PcmParser : PCM RIFF header error";
	}
	if ( ( *((uint32_t*)(&header_buffer[0x08])) != 0x45564157 ) || ( *((uint32_t*)(&header_buffer[0x0C])) != 0x20746D66 ) ) { // "WAVEfmt " tag
		finalize();
		return "PcmParser : PCM WAVEfmt header error";
	}
	uint32_t wavefmt_size = *((uint32_t*)(&header_buffer[0x10]));
	uint8_t* wavefmt_buffer = (uint8_t*)malloc(wavefmt_size);
	if ( wavefmt_buffer == 0 ) {
		finalize();
		return "PcmParser : malloc WaveFMT buffer fail";
	}
	if ( buffered_reader_read(reader, wavefmt_buffer, wavefmt_size) != wavefmt_size ) {
		free(wavefmt_buffer);
		finalize();
		return "PcmParser : read WaveFMT header fail";
	}
	if ( *((uint16_t*)(&wavefmt_buffer[0x00])) != 0x01 ) { // PCM Format tag = 0x0001
		free(wavefmt_buffer);
		finalize();
		return "PcmParser : PCM Format Tag error";
	}
	init_data.channels = *((uint16_t*)(&wavefmt_buffer[0x02]));
	init_data.samplerate = *((uint32_t*)(&wavefmt_buffer[0x04]));
	init_data.sample_bits = *((uint16_t*)(&wavefmt_buffer[0x0E]));
	free(wavefmt_buffer);
	init_data.samples_per_frame = SIRENS2_PCM_SAMPLES_PER_FRAME;
	init_data.samples_per_decoded = init_data.samples_per_frame;
	
	
	if ( init_data.channels < 1 || init_data.channels > 2 || init_data.sample_bits != 16 ) {
		finalize();
		return "PcmParser : channels != 1 && channels != 2 || sample_bits != 16";
	}
	
	init_data.frame_align = init_data.samples_per_frame * (16 >> 3) * 2;
	
	uint32_t data_header[2];
	if ( buffered_reader_read(reader, data_header, 8) != 8 ) {
		finalize();
		return "PcmParser : read data header fail";
	}
	while(data_header[0] != 0x61746164 ) {
		buffered_reader_seek(reader, buffered_reader_position(reader)+data_header[1]);
		if ( buffered_reader_read(reader, data_header, 8) != 8 ) { 
			finalize();
			return "PcmParser : read data header fail";
		}
	}
	
	pcm_data_start = buffered_reader_position(reader);
	pcm_data_length = data_header[1];
	
	total_samples = pcm_data_length / (init_data.sample_bits >> 3) / init_data.channels;
	
	output_buffer = malloc_64(init_data.frame_align);
	if ( output_buffer == 0 ) {
		finalize();
		return "PcmParser : malloc output_buffer fail";
	}
	
	is_open = true;
	return 0;
};

char* PcmParser::open_cb(audio_callbacks cb,int handle) {
	
	reader = buffered_reader_open_cb(cb,handle,131072, 1);
	if ( !reader ) {
		finalize();
		return "PcmParser : open buffered_reader fail";
	}
	
	buffered_reader_seek(reader, 0);
	uint8_t header_buffer[20];
	if ( buffered_reader_read(reader, header_buffer, 20) != 20 ) {
		finalize();
		return "PcmParser : read PCM header fail";
	}
	if ( *((uint32_t*)(&header_buffer[0x00])) != 0x46464952 ) { // "RIFF" tag
		finalize();
		return "PcmParser : PCM RIFF header error";
	}
	if ( ( *((uint32_t*)(&header_buffer[0x08])) != 0x45564157 ) || ( *((uint32_t*)(&header_buffer[0x0C])) != 0x20746D66 ) ) { // "WAVEfmt " tag
		finalize();
		return "PcmParser : PCM WAVEfmt header error";
	}
	uint32_t wavefmt_size = *((uint32_t*)(&header_buffer[0x10]));
	uint8_t* wavefmt_buffer = (uint8_t*)malloc(wavefmt_size);
	if ( wavefmt_buffer == 0 ) {
		finalize();
		return "PcmParser : malloc WaveFMT buffer fail";
	}
	if ( buffered_reader_read(reader, wavefmt_buffer, wavefmt_size) != wavefmt_size ) {
		free(wavefmt_buffer);
		finalize();
		return "PcmParser : read WaveFMT header fail";
	}
	if ( *((uint16_t*)(&wavefmt_buffer[0x00])) != 0x01 ) { // PCM Format tag = 0x0001
		free(wavefmt_buffer);
		finalize();
		return "PcmParser : PCM Format Tag error";
	}
	init_data.channels = *((uint16_t*)(&wavefmt_buffer[0x02]));
	init_data.samplerate = *((uint32_t*)(&wavefmt_buffer[0x04]));
	init_data.sample_bits = *((uint16_t*)(&wavefmt_buffer[0x0E]));
	free(wavefmt_buffer);
	init_data.samples_per_frame = SIRENS2_PCM_SAMPLES_PER_FRAME;
	init_data.samples_per_decoded = init_data.samples_per_frame;
	
	
	if ( init_data.channels < 1 || init_data.channels > 2 || init_data.sample_bits != 16 ) {
		finalize();
		return "PcmParser : channels != 1 && channels != 2 || sample_bits != 16";
	}
	
	init_data.frame_align = init_data.samples_per_frame * (16 >> 3) * 2;
	
	uint32_t data_header[2];
	if ( buffered_reader_read(reader, data_header, 8) != 8 ) {
		finalize();
		return "PcmParser : read data header fail";
	}
	while(data_header[0] != 0x61746164 ) {
		buffered_reader_seek(reader, buffered_reader_position(reader)+data_header[1]);
		if ( buffered_reader_read(reader, data_header, 8) != 8 ) { 
			finalize();
			return "PcmParser : read data header fail";
		}
	}
	
	pcm_data_start = buffered_reader_position(reader);
	pcm_data_length = data_header[1];
	
	total_samples = pcm_data_length / (init_data.sample_bits >> 3) / init_data.channels;
	
	output_buffer = malloc_64(init_data.frame_align);
	if ( output_buffer == 0 ) {
		finalize();
		return "PcmParser : malloc output_buffer fail";
	}
	
	is_open = true;
	return 0;
};


void PcmParser::close() {
	if ( is_open ) {
		finalize();
	}
};

uint32_t PcmParser::get_samplerate() {
	return init_data.samplerate;
};

uint32_t PcmParser::get_channels() {
	return init_data.channels;
};

uint64_t PcmParser::get_duration() {
	uint64_t duration = 1000000LL*total_samples/init_data.samplerate;
	return duration;
};
	
uint32_t PcmParser::get_samples_per_frame() {
	return init_data.samples_per_frame;
};

uint32_t PcmParser::get_max_samples_per_frame() {
	return init_data.samples_per_frame;
};

uint64_t PcmParser::seek_time(uint64_t time) {
	uint32_t samples = (uint32_t)(time*init_data.samplerate/1000000LL);
	if ( samples > total_samples )
		samples = total_samples;
	
	uint32_t offset = pcm_data_start + samples*(init_data.sample_bits >> 3)*init_data.channels;
	buffered_reader_seek(reader, offset);
	current_samples = samples;
	
	uint64_t current = 1000000LL*samples/init_data.samplerate;
	return current;
};

char* PcmParser::get_frame(void** pcm, uint32_t* samples) {
	
	if ( !is_open )
		return "PcmParser : parser is not open";
	
	memset(output_buffer, 0, init_data.frame_align);
	
	uint32_t get_samples = total_samples-current_samples;
	get_samples = (get_samples>init_data.samples_per_frame) ? init_data.samples_per_frame : get_samples;
	uint32_t frame_size = get_samples*(init_data.sample_bits >> 3)*init_data.channels;
	
	if ( buffered_reader_read(reader, output_buffer, frame_size) != frame_size ) {
		get_samples = 0;
	}
	if ( get_samples > 0 && init_data.channels == 1 ) {
		int32_t i;
		uint16_t* p = (uint16_t*)output_buffer;
		for(i=get_samples-1; i>=0; i--) {
			p[2*i+1] = p[i];
			p[2*i] = p[i];
		} 
	}
	current_samples+=get_samples;
	*pcm = output_buffer;
	*samples = get_samples;
	return 0;
};





