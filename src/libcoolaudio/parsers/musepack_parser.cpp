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
 
#include "musepack_parser.h"
#include "common/mem64.h"
#include <pspkernel.h>
#include <stdlib.h>
#include <string.h>
#include <mpc/mpcdec.h>

#ifdef MPC_FIXED_POINT
static int
shift_signed(MPC_SAMPLE_FORMAT val, int shift)
{
    if (shift > 0)
        val <<= shift;
    else if (shift < 0)
        val >>= -shift;
    return (int)val;
}
#endif

static int clip_min = -1 << (16 - 1);
static int clip_max = (1 << (16 - 1)) - 1;
static int float_scale = 1 << (16 - 1);

static mpc_int32_t musepack_read(mpc_reader *p_reader, void *ptr, mpc_int32_t size) {
	buffered_reader_t* reader = (buffered_reader_t*)p_reader->data;
	return (mpc_int32_t)buffered_reader_read(reader, ptr, size);
}

static mpc_bool_t musepack_seek(mpc_reader *p_reader, mpc_int32_t offset) {
	buffered_reader_t* reader = (buffered_reader_t*)p_reader->data;
	if ( buffered_reader_seek(reader, offset) == offset )
		return MPC_TRUE;
	else
		return MPC_FALSE;
}

static mpc_int32_t musepack_tell(mpc_reader *p_reader) {
	buffered_reader_t* reader = (buffered_reader_t*)p_reader->data;
	return (mpc_int32_t)buffered_reader_position(reader);
}

static mpc_int32_t musepack_get_size(mpc_reader *p_reader) {
	buffered_reader_t* reader = (buffered_reader_t*)p_reader->data;
	return (mpc_int32_t)buffered_reader_length(reader);
}

static mpc_bool_t musepack_canseek(mpc_reader *p_reader) {
	return MPC_TRUE;
}

MusepackParser::MusepackParser() {
	initialize();
};

MusepackParser::~MusepackParser() {
	close();
};

void MusepackParser::initialize() {
	
	is_open = false;
	
	musepack_demux = 0;
	musepack_reader = 0;
	reader = 0;
	output_buffer = 0;
	
	memset(&init_data, 0, sizeof(AudioCodecInitData));
};

void MusepackParser::finalize() {
	if ( reader ) {
		buffered_reader_close(reader);
	}
	if ( musepack_reader ) {
		free(musepack_reader);
	}
	if ( musepack_demux ) {
		mpc_demux_exit((mpc_demux*)musepack_demux);
	}
	if ( output_buffer ) {
		free(output_buffer);
	}
	initialize();
};

char* MusepackParser::open(const char* filename) {
	
	close();
	
	char* result;
	
	reader = buffered_reader_open(filename, 16384, 1);
	if ( !reader ) {
		finalize();
		return "MusepackParser : open buffered_reader fail";
	}
	
	musepack_reader = malloc_64(sizeof(mpc_reader));
	if ( !musepack_reader ) {
		finalize();
		return "MusepackParser : malloc mpc_reader fail";
	}
	
	mpc_reader* p_reader = (mpc_reader*)musepack_reader;
	p_reader->read = musepack_read;
	p_reader->seek = musepack_seek;
	p_reader->tell = musepack_tell;
	p_reader->get_size = musepack_get_size;
	p_reader->canseek = musepack_canseek;
	p_reader->data = (void*)reader;
	
	mpc_demux* p_demux = mpc_demux_init(p_reader);
	if ( !p_demux ) {
		finalize();
		return "MusepackParser : mpc_demux_init fail";
	}
	musepack_demux = (void*)p_demux;
	
	mpc_streaminfo info ;
	
	mpc_demux_get_info(p_demux, &info);
	
	init_data.channels = info.channels;
	init_data.samplerate = info.sample_freq;
	init_data.sample_bits = 16;
	
	total_samples = info.samples;
	
	if ( init_data.channels < 1 || init_data.channels > 2 || init_data.sample_bits != 16 ) {
		finalize();
		return "MusepackParser : channels != 1 && channels != 2 || sample_bits != 16";
	}
	
	init_data.samples_per_frame = MPC_FRAME_LENGTH;
	init_data.samples_per_decoded = MPC_FRAME_LENGTH;
	init_data.frame_align = init_data.samples_per_frame * (16 >> 3) * 2;
	
	output_buffer = malloc_64(init_data.frame_align);
	if ( output_buffer == 0 ) {
		finalize();
		return "MusepackParser : malloc output_buffer fail";
	}
	
	is_open = true;
	return 0;
};

char* MusepackParser::open_cb(audio_callbacks cb,int handle) {
	
	close();
	
	char* result;
	
	reader = buffered_reader_open_cb(cb,handle, 16384, 1);
	if ( !reader ) {
		finalize();
		return "MusepackParser : open buffered_reader fail";
	}
	
	musepack_reader = malloc_64(sizeof(mpc_reader));
	if ( !musepack_reader ) {
		finalize();
		return "MusepackParser : malloc mpc_reader fail";
	}
	
	mpc_reader* p_reader = (mpc_reader*)musepack_reader;
	p_reader->read = musepack_read;
	p_reader->seek = musepack_seek;
	p_reader->tell = musepack_tell;
	p_reader->get_size = musepack_get_size;
	p_reader->canseek = musepack_canseek;
	p_reader->data = (void*)reader;
	
	mpc_demux* p_demux = mpc_demux_init(p_reader);
	if ( !p_demux ) {
		finalize();
		return "MusepackParser : mpc_demux_init fail";
	}
	musepack_demux = (void*)p_demux;
	
	mpc_streaminfo info ;
	
	mpc_demux_get_info(p_demux, &info);
	
	init_data.channels = info.channels;
	init_data.samplerate = info.sample_freq;
	init_data.sample_bits = 16;
	
	total_samples = info.samples;
	
	if ( init_data.channels < 1 || init_data.channels > 2 || init_data.sample_bits != 16 ) {
		finalize();
		return "MusepackParser : channels != 1 && channels != 2 || sample_bits != 16";
	}
	
	init_data.samples_per_frame = MPC_FRAME_LENGTH;
	init_data.samples_per_decoded = MPC_FRAME_LENGTH;
	init_data.frame_align = init_data.samples_per_frame * (16 >> 3) * 2;
	
	output_buffer = malloc_64(init_data.frame_align);
	if ( output_buffer == 0 ) {
		finalize();
		return "MusepackParser : malloc output_buffer fail";
	}
	
	is_open = true;
	return 0;
};


void MusepackParser::close() {
	if ( is_open ) {
		finalize();
	}
};
	
uint32_t MusepackParser::get_samplerate() {
	return init_data.samplerate;
};

uint32_t MusepackParser::get_channels() {
	return init_data.channels;
};

uint64_t MusepackParser::get_duration() {
	uint64_t duration = 1000000LL*total_samples/init_data.samplerate;
	return duration;
};
	
uint32_t MusepackParser::get_samples_per_frame() {
	return init_data.samples_per_frame;
};

uint32_t MusepackParser::get_max_samples_per_frame() {
	return init_data.samples_per_frame;
};

uint64_t MusepackParser::seek_time(uint64_t time) {
	uint32_t samples = (uint32_t)(time*init_data.samplerate/1000000LL);
	if ( samples > total_samples )
		samples = total_samples;
	mpc_demux_seek_sample((mpc_demux*)musepack_demux, samples);
	uint64_t current = 1000000LL*samples/init_data.samplerate;
	return current;
};

char* MusepackParser::get_frame(void** pcm, uint32_t* samples) {
	
	if ( !is_open )
		return "MusepackParser : parser is not open";
	
	while(1) {
		memset(output_buffer, 0, init_data.frame_align);
	
		mpc_frame_info frame;
		MPC_SAMPLE_FORMAT sample_buffer[MPC_FRAME_LENGTH*2];
		frame.buffer = sample_buffer;
		mpc_demux_decode((mpc_demux*)musepack_demux, &frame) ;
		if (frame.bits == -1) {
			return "MusepackParser : mpc_demux_decode fail";
		}
	
		*samples = frame.samples;
		if ( *samples > 0 ) {
			int i;
			uint16_t* output = (uint16_t*)output_buffer;
			for(i = 0; i < (*samples)*init_data.channels; i++) {
				int val;
#ifdef MPC_FIXED_POINT
				val = shift_signed(frame.buffer[i], 16 - MPC_FIXED_POINT_SCALE_SHIFT);
#else
				val = (int)(frame.buffer[i] * float_scale);
#endif
				if (val < clip_min)
					val = clip_min;
				else if (val > clip_max)
					val = clip_max;
				output[i] = val;
			}
			if (init_data.channels == 1 ) {
				uint16_t* p = (uint16_t*)output_buffer;
				for(i=*samples-1; i>=0; i--) {
					p[2*i+1] = p[i];
					p[2*i] = p[i];
				} 
			}
	
			*pcm = output_buffer;
			return 0;
		}
	}
	return 0;
};



