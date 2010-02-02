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
 
#include "oggvorbis_parser.h"
#include "common/mem64.h"
#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>
#include <string.h>
#include <pspsdk.h>
#include <pspkernel.h>

#define SIRENS2_OGGVORBIS_SAMPLES_PER_FRAME 2048


static size_t ovcb_read(void *ptr, size_t size, size_t nmemb, void *datasource) {
	return buffered_reader_read((buffered_reader_t*)datasource, ptr, size*nmemb);
}

static int ovcb_seek(void *datasource, int64_t offset, int whence) {
	if( whence == PSP_SEEK_SET )
		return buffered_reader_seek((buffered_reader_t*)datasource, offset);
	else if ( whence == PSP_SEEK_CUR )
		return buffered_reader_seek((buffered_reader_t*)datasource, 
			offset + buffered_reader_position((buffered_reader_t*)datasource) );
	else 
		return buffered_reader_seek((buffered_reader_t*)datasource,
			buffered_reader_length((buffered_reader_t*)datasource) + offset );		
}

static int ovcb_close(void *datasource) {
	buffered_reader_close( (buffered_reader_t*)datasource );
	return 1;
}

static long ovcb_tell(void *datasource){
	return buffered_reader_position((buffered_reader_t*)datasource);
}

ov_callbacks vorbis_callbacks = {
	ovcb_read,
	ovcb_seek,
	ovcb_close,
	ovcb_tell
};

OggVorbisParser::OggVorbisParser() {
	initialize();
};

OggVorbisParser::~OggVorbisParser() {
	close();
};

void OggVorbisParser::initialize() {
	is_open = false;
	
	total_samples = 0;
	
	reader = 0;
	ogg_vorbis_file = 0;
	output_buffer = 0;
	
	memset(&init_data, 0, sizeof(AudioCodecInitData));
};

void OggVorbisParser::finalize() {
	if ( ogg_vorbis_file ) {
		ov_clear((OggVorbis_File*)ogg_vorbis_file);
		free_64(ogg_vorbis_file);
	}
	else {
		if ( reader )
			buffered_reader_close(reader);
	}
	if ( output_buffer ) {
		free(output_buffer);
	}
	initialize();
};

char* OggVorbisParser::open(const char* filename) {
	
	close();
	
	reader = buffered_reader_open(filename, 262144, 1);
	if ( !reader ) {
		finalize();
		return "OggVorbisParser : open buffered_reader fail";
	}
	buffered_reader_seek(reader, 0);	
	
	ogg_vorbis_file = malloc_64( sizeof(OggVorbis_File) );
	if ( ogg_vorbis_file == 0 ) {
		finalize();
		return "OggVorbisParser : malloc OggVorbis_File fail";
	}
	
	OggVorbis_File* vf = (OggVorbis_File*)ogg_vorbis_file;
	memset(vf, 0, sizeof(OggVorbis_File));
	if (ov_open_callbacks((void*)reader, vf, NULL, 0, vorbis_callbacks) < 0) {
		free_64(ogg_vorbis_file);
		ogg_vorbis_file = 0;
		finalize();
		return "OggVorbisParser : ov_open_callbacks fail";
	}
	
	vorbis_info *vi;
	vi = ov_info(vf, -1);
	
	if (vi->channels > 2 || vi->channels <= 0) {
		finalize();
		return "OggVorbisParser : channels != 1 && channels != 2";
	}
	
	init_data.channels = vi->channels;
	init_data.samplerate = vi->rate;
	init_data.sample_bits = 16;
	init_data.samples_per_frame = SIRENS2_OGGVORBIS_SAMPLES_PER_FRAME;
	init_data.samples_per_decoded = init_data.samples_per_frame;
	init_data.frame_align = init_data.samples_per_frame * (16 >> 3) * 2;
	
	total_samples = ov_pcm_total(vf, -1);
	output_buffer = malloc_64(init_data.frame_align);
	if ( output_buffer == 0 ) {
		finalize();
		return "OggVorbisParser : malloc output_buffer fail";
	}
	
	is_open = true;
	return 0;
};

char* OggVorbisParser::open_cb(audio_callbacks cb,int handle) {
	
	close();
	
	reader = buffered_reader_open_cb(cb,handle, 262144, 1);
	if ( !reader ) {
		finalize();
		return "OggVorbisParser : open buffered_reader fail";
	}
	buffered_reader_seek(reader, 0);	
	
	ogg_vorbis_file = malloc_64( sizeof(OggVorbis_File) );
	if ( ogg_vorbis_file == 0 ) {
		finalize();
		return "OggVorbisParser : malloc OggVorbis_File fail";
	}
	
	OggVorbis_File* vf = (OggVorbis_File*)ogg_vorbis_file;
	memset(vf, 0, sizeof(OggVorbis_File));
	if (ov_open_callbacks((void*)reader, vf, NULL, 0, vorbis_callbacks) < 0) {
		free_64(ogg_vorbis_file);
		ogg_vorbis_file = 0;
		finalize();
		return "OggVorbisParser : ov_open_callbacks fail";
	}
	
	vorbis_info *vi;
	vi = ov_info(vf, -1);
	
	if (vi->channels > 2 || vi->channels <= 0) {
		finalize();
		return "OggVorbisParser : channels != 1 && channels != 2";
	}
	
	init_data.channels = vi->channels;
	init_data.samplerate = vi->rate;
	init_data.sample_bits = 16;
	init_data.samples_per_frame = SIRENS2_OGGVORBIS_SAMPLES_PER_FRAME;
	init_data.samples_per_decoded = init_data.samples_per_frame;
	init_data.frame_align = init_data.samples_per_frame * (16 >> 3) * 2;
	
	total_samples = ov_pcm_total(vf, -1);
	output_buffer = malloc_64(init_data.frame_align);
	if ( output_buffer == 0 ) {
		finalize();
		return "OggVorbisParser : malloc output_buffer fail";
	}
	
	is_open = true;
	return 0;
};

void OggVorbisParser::close() {
	if ( is_open ) {
		finalize();
	}
};

uint32_t OggVorbisParser::get_samplerate() {
	return init_data.samplerate;
};

uint32_t OggVorbisParser::get_channels() {
	return init_data.channels;
};

uint64_t OggVorbisParser::get_duration() {
	uint64_t duration = 1000000LL*total_samples/init_data.samplerate;
	return duration;
};
	
uint32_t OggVorbisParser::get_samples_per_frame() {
	return init_data.samples_per_frame;
};

uint32_t OggVorbisParser::get_max_samples_per_frame() {
	return init_data.samples_per_frame;
};

uint64_t OggVorbisParser::seek_time(uint64_t time) {
	uint32_t samples = (uint32_t)(time*init_data.samplerate/1000000LL);
	if ( samples > total_samples )
		samples = total_samples;
	ov_pcm_seek((OggVorbis_File*)ogg_vorbis_file, samples);
	uint64_t current = 1000000LL*samples/init_data.samplerate;
	return current;
};


char* OggVorbisParser::get_frame(void** pcm, uint32_t* samples) {
	
	if ( !is_open )
		return "OggVorbisParser : parser is not open";
		
	int current_section;
	const int bep = 0;
	int bytes = 0;
	memset(output_buffer, 0, init_data.frame_align);
	
	int need_bytes = init_data.samples_per_frame * (16 >> 3) * init_data.channels;
	bytes = ov_read((OggVorbis_File*)ogg_vorbis_file, 
				((char*)output_buffer),
				need_bytes, 
				bep, 2, 1,
				&current_section);
	uint32_t get_samples = (bytes>0) ? (bytes/2/init_data.channels) : 0;
	
	if ( get_samples > 0 && init_data.channels == 1 ) {
		int32_t i;
		uint16_t* p = (uint16_t*)output_buffer;
		for(i=get_samples-1; i>=0; i--) {
			p[2*i+1] = p[i];
			p[2*i] = p[i];
		} 
	}
	
	*pcm = output_buffer;
	*samples = get_samples;
	
	return 0;
};





