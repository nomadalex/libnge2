#include "audio_interface.h"
#include "common/mem64.h"
#include "parsers/pcm_parser.h"
#include "parsers/oggvorbis_parser.h"
#include "parsers/mp3_parser.h"
#include <pspkernel.h>
#include <pspctrl.h>
#include <psppower.h>
#include <pspdebug.h>
#include <psprtc.h>
#include <pspsdk.h>
#include <pspaudio.h>
#include <psputility.h>
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include "nge_io_mem.h"

#define COOLAUDIO_SIRENS2_PCM 0
#define COOLAUDIO_SIRENS2_MP3 1
#define COOLAUDIO_SIRENS2_OGG 2

typedef struct sirens2_private_t {
	AudioParser* parser;
	bool parser_open;
	bool player_open;
	
	
	int64_t start_time;
	int64_t end_time;
	
	void* cached_pcm_buffer;
	int32_t cached_samples;
	
	int32_t pcm_samples;
	void* pcm_buffer[2];
	int32_t pcm_buffer_index;
	
	SceUID play_channel;
	SceUID play_thread;
	bool play_thread_started;
	bool play_thread_running;
	
	bool play_start;
	bool play_pause;
	bool play_eof;
	bool play_seekable;
	bool play_seek;
	bool play_stop;
	
	int	play_times;
	int played_times;
	int volume;
	int64_t current;
	int64_t duration;
	int64_t seek_to;
	
} sirens2_private_t, *sirens2_private_p;

typedef struct sirens2_play_t {
	fd_load       load;
	fd_load_fp    load_fp;
	fd_load_buf   load_buf;
	fd_play       play;
	fd_playstop   playstop;
	fd_pause      pause;
	fd_stop       stop;
	fd_resume     resume;
	fd_volume     volume;
	fd_rewind     rewind;
	fd_seek       seek;
	fd_iseof      iseof;
	fd_ispaused   ispaused;
	fd_destroy    destroy;
	int 	audio_type;
	sirens2_private_p private_data;
} sirens2_play_t,*sirens2_play_p;

void Sirens2InitializeResource(volatile sirens2_private_t* p) {
	{
		p->parser = 0;
		
		p->parser_open = false;
		p->player_open = false;
	
		p->start_time = 0LL;
		p->end_time = 0LL;
	
		p->cached_pcm_buffer = 0;
		p->cached_samples = 0;
	
		p->pcm_buffer_index = 0;
		p->pcm_samples = 0;
	
		p->pcm_buffer[0] = 0;
		p->pcm_buffer[1] = 0;
		
		p->play_start = false;
		p->play_pause = false;
		p->play_eof = false;
		p->play_seek = false;
		p->play_seekable = true;
		p->play_stop = false;
		
		p->play_times = 1;
		p->played_times = 0;
		p->volume = PSP_AUDIO_VOLUME_MAX;
		p->current = 0LL;
		p->duration = 0LL;
		p->seek_to = 0LL;
	}
}

void Sirens2Initialize(sirens2_play_p p) {
	if ( p->private_data ) {
		p->private_data->play_thread_started = false;
		p->private_data->play_thread_running = false;
		p->private_data->play_thread = -1;
		p->private_data->play_channel = -1;
		Sirens2InitializeResource(p->private_data);
	}
}

void Sirens2FinalizeResource(volatile sirens2_private_t* p) {
	{
		if ( p->parser ) {
			if ( p->parser_open )
				p->parser->close();
			delete p->parser;
		}
	
		if ( p->cached_pcm_buffer ) {
			free_64(p->cached_pcm_buffer);
		}
	
		if ( p->pcm_buffer[0] ) {
			free_64(p->pcm_buffer[0]);
		}
	
		if ( p->pcm_buffer[1] ) {
			free_64(p->pcm_buffer[1]);
		}
		Sirens2InitializeResource(p);
	}
}

void Sirens2Finalize(sirens2_play_p p) {
	if ( p->private_data ) {
		//*/
		if ( p->private_data->play_thread_started ) {
			p->private_data->play_thread_running = false;
			sceKernelWaitThreadEnd(	p->private_data->play_thread, 0 );
		}
		if (!(p->private_data->play_thread < 0)) {
			sceKernelDeleteThread(p->private_data->play_thread);
		}
		if ( !(p->private_data->play_channel<0) ) {
			sceAudioChRelease(p->private_data->play_channel);
		}
		//*/
		Sirens2FinalizeResource(p->private_data);
		free_64(p->private_data);
		p->private_data = 0;
	}
}

static int Sirens2PlayThread(SceSize args, void *argp){
	volatile sirens2_private_t* p = (volatile sirens2_private_t*)(*((void**)argp));
	
	while(p->play_thread_running) {
		if ( p->play_start ) {
			bool parser_eof = false;
			while(p->play_start && (p->play_times==0 || p->played_times < p->play_times)) {
				
				if (p->play_pause) {
					sceKernelDelayThread(1000);
					continue;
				}
				if ( p->play_seek ) {
					p->cached_samples = 0;
					p->current = p->parser->seek_time(p->seek_to) - p->start_time;
					p->play_seek = false;
					continue;
				}
				
				if ( p->cached_samples >= p->pcm_samples ) {
					memset(p->pcm_buffer[p->pcm_buffer_index], 0, p->pcm_samples<<2);
					memcpy(p->pcm_buffer[p->pcm_buffer_index], p->cached_pcm_buffer, p->pcm_samples<<2);
					p->cached_samples -= p->pcm_samples;
					if ( p->cached_samples > 0 )
						memmove(p->cached_pcm_buffer, ((unsigned char*)p->cached_pcm_buffer)+(p->pcm_samples<<2), p->cached_samples<<2);
				
					sceAudioOutputBlocking(p->play_channel, p->volume, p->pcm_buffer[p->pcm_buffer_index]);
					p->pcm_buffer_index = (p->pcm_buffer_index + 1) % 2;
			
					p->current += p->pcm_samples*1000000LL/p->parser->get_samplerate();
				}
				else {
					if ( parser_eof ) {
						if ( p->cached_samples > 0 ) {
							memset(p->pcm_buffer[p->pcm_buffer_index], 0, p->pcm_samples<<2);
							memcpy(p->pcm_buffer[p->pcm_buffer_index], p->cached_pcm_buffer, p->cached_samples<<2);
					
							sceAudioOutputBlocking(p->play_channel, p->volume, p->pcm_buffer[p->pcm_buffer_index]);
							p->pcm_buffer_index = (p->pcm_buffer_index + 1) % 2;
			
							p->current += p->cached_samples*1000000LL/p->parser->get_samplerate();
							p->cached_samples-=p->cached_samples;
						}
						else {
							p->played_times+=1;
							if ( p->play_times == 0 || p->played_times < p->play_times ) {
								p->current = p->parser->seek_time(p->start_time) - p->start_time;
								parser_eof = false;
							}
						}
					}
					else {
						void* pcm;
						uint32_t samples;
						if ( p->current > (p->end_time-p->start_time) || p->parser->get_frame(&pcm, &samples) || samples == 0) {
							parser_eof = true;
						}
						else {
							memcpy(((unsigned char*)p->cached_pcm_buffer)+(p->cached_samples<<2), pcm, samples<<2);
							p->cached_samples+=samples;
						}
					}	
				}
			}
			p->play_start = false;
			p->play_eof = true;
			if ( p->play_stop ) 
				Sirens2FinalizeResource(p);
		}
		sceKernelDelayThread(1000);
	}
	sceKernelExitThread(0);
	return 0;
}

int Sirens2Load(struct audio_play* This,const char* filename) {
	sirens2_play_p p = (sirens2_play_p)This;
	
	if ( p->private_data->player_open )
		return -1;
	
	switch(p->audio_type) {
		case COOLAUDIO_SIRENS2_PCM: {
			p->private_data->parser = new PcmParser();
			if ( !p->private_data->parser ) {
				Sirens2FinalizeResource(p->private_data);
				return -2;
			}
		}
		break;
		case COOLAUDIO_SIRENS2_MP3: {
			p->private_data->parser = new Mp3Parser();
			if ( !p->private_data->parser ) {
				Sirens2FinalizeResource(p->private_data);
				return -2;
			}
		}
		break;
		case COOLAUDIO_SIRENS2_OGG: {
			p->private_data->parser = new OggVorbisParser();
			if ( !p->private_data->parser ) {
				Sirens2FinalizeResource(p->private_data);
				return -2;
			}
		}
		break;
		default: {
			Sirens2FinalizeResource(p->private_data);
			return -2;
		}
		break;
	}
	
	char* result = 0;
	result = p->private_data->parser->open(filename);
	if ( result ) {
		Sirens2FinalizeResource(p->private_data);
		return -3;
	}
	
	p->private_data->parser_open = true;
	
	p->private_data->start_time = 0LL;
	p->private_data->end_time = p->private_data->parser->get_duration();
	
	p->private_data->duration = (p->private_data->end_time - p->private_data->start_time);
	
	p->private_data->pcm_samples = p->private_data->parser->get_samples_per_frame();
	
	p->private_data->pcm_buffer[0] = malloc_64( p->private_data->pcm_samples<<2 );
	if ( p->private_data->pcm_buffer[0] == 0 ) {
		Sirens2FinalizeResource(p->private_data);
		return -4;
	}
	
	p->private_data->pcm_buffer[1] = malloc_64( p->private_data->pcm_samples<<2 );
	if ( p->private_data->pcm_buffer[1] == 0 ) {
		Sirens2FinalizeResource(p->private_data);
		return -4;
	}
	
	p->private_data->cached_pcm_buffer = malloc_64( (p->private_data->pcm_samples+p->private_data->parser->get_max_samples_per_frame())<<2 );
	if ( p->private_data->cached_pcm_buffer == 0 ) {
		Sirens2FinalizeResource(p->private_data);
		return -5;
	}
	
	sceAudioSetChannelDataLen(p->private_data->play_channel, p->private_data->pcm_samples);
	
	p->private_data->player_open = true;
	
	return 1;
}
//callbacks
int io_myread(void * ptr, int count, int size, void * source)
{
	return io_mread(ptr,count,size,(int)source);
}

int io_myseek(void * source, int offset, int whence)
{
	return io_mseek((int)source,(int)offset,whence);
}


long io_mytell(void * source)
{
	return io_mtell((int)source);
}

int io_myclose(void* source)
{
	return io_mclose((int)source);
}

static audio_callbacks CALLBACKS_MY = {
  (int (*)(void *, int, int, int))		io_myread,
  (int (*)(int, int, int))           io_myseek,
  (int (*)(int))                             io_myclose,
  (long (*)(int))                            io_mytell
};



int Sirens2LoadBuf(struct audio_play* This,const char* buf,int size) {
	sirens2_play_p p = (sirens2_play_p)This;
	
	if ( p->private_data->player_open )
		return -1;
	
	switch(p->audio_type) {
		case COOLAUDIO_SIRENS2_PCM: {
			p->private_data->parser = new PcmParser();
			if ( !p->private_data->parser ) {
				Sirens2FinalizeResource(p->private_data);
				return -2;
			}
		}
		break;
		case COOLAUDIO_SIRENS2_MP3: {
			p->private_data->parser = new Mp3Parser();
			if ( !p->private_data->parser ) {
				Sirens2FinalizeResource(p->private_data);
				return -2;
			}
		}
		break;
		case COOLAUDIO_SIRENS2_OGG: {
			p->private_data->parser = new OggVorbisParser();
			if ( !p->private_data->parser ) {
				Sirens2FinalizeResource(p->private_data);
				return -2;
			}
		}
		break;
		default: {
			Sirens2FinalizeResource(p->private_data);
			return -2;
		}
		break;
	}
	
	char* result = 0;
	int handle = io_mopen(buf,size,IO_RDONLY);
	result = p->private_data->parser->open_cb(CALLBACKS_MY,handle);
	if ( result ) {
		Sirens2FinalizeResource(p->private_data);
		return -3;
	}
	
	p->private_data->parser_open = true;
	
	p->private_data->start_time = 0LL;
	p->private_data->end_time = p->private_data->parser->get_duration();
	
	p->private_data->duration = (p->private_data->end_time - p->private_data->start_time);
	
	p->private_data->pcm_samples = p->private_data->parser->get_samples_per_frame();
	
	p->private_data->pcm_buffer[0] = malloc_64( p->private_data->pcm_samples<<2 );
	if ( p->private_data->pcm_buffer[0] == 0 ) {
		Sirens2FinalizeResource(p->private_data);
		return -4;
	}
	
	p->private_data->pcm_buffer[1] = malloc_64( p->private_data->pcm_samples<<2 );
	if ( p->private_data->pcm_buffer[1] == 0 ) {
		Sirens2FinalizeResource(p->private_data);
		return -4;
	}
	
	p->private_data->cached_pcm_buffer = malloc_64( (p->private_data->pcm_samples+p->private_data->parser->get_max_samples_per_frame())<<2 );
	if ( p->private_data->cached_pcm_buffer == 0 ) {
		Sirens2FinalizeResource(p->private_data);
		return -5;
	}
	
	sceAudioSetChannelDataLen(p->private_data->play_channel, p->private_data->pcm_samples);
	
	p->private_data->player_open = true;
	
	return 1;
}


int Sirens2Destroy(struct audio_play* This) {
	sirens2_play_p p = (sirens2_play_p)This;
	Sirens2Finalize(p);
	free_64(p);
}

int Sirens2Play(struct audio_play* This, int times, int free_when_stop) {
	sirens2_play_p p = (sirens2_play_p)This;
	volatile sirens2_private_t* private_data = (volatile sirens2_private_t*)(p->private_data);
	if ( !private_data->player_open )
		return -1;
	if ( private_data->play_eof )
		return -1;
	if ( private_data->play_start )
		return 1;
	
	private_data->played_times = 0;
	private_data->play_times = times;
	private_data->play_stop = free_when_stop;
	private_data->play_start = true;
	return 1;
}

int Sirens2PlayStop(struct audio_play* This) {
	return Sirens2Play(This, 1, 1);
}

int Sirens2Stop(struct audio_play* This) {
	sirens2_play_p p = (sirens2_play_p)This;
	volatile sirens2_private_t* private_data = (volatile sirens2_private_t*)(p->private_data);
	if ( !private_data->player_open )
		return 1;
	private_data->play_start = false;
	if ( private_data->play_stop ) {
		while( private_data->player_open ) {
			sceKernelDelayThread(1000);
		}
	}
	else {
		while( !private_data->play_eof ) {
			sceKernelDelayThread(1000);
		}
		Sirens2FinalizeResource(private_data);
	}
	return 1;
}

void Sirens2Pause(struct audio_play* This) {
	sirens2_play_p p = (sirens2_play_p)This;
	volatile sirens2_private_t* private_data = (volatile sirens2_private_t*)(p->private_data);
	private_data->play_pause = true;
}

void Sirens2Resume(struct audio_play* This) {
	sirens2_play_p p = (sirens2_play_p)This;
	volatile sirens2_private_t* private_data = (volatile sirens2_private_t*)(p->private_data);
	private_data->play_pause = false;
}

int Sirens2Volume(struct audio_play* This,int volume) {
	sirens2_play_p p = (sirens2_play_p)This;
	volatile sirens2_private_t* private_data = (volatile sirens2_private_t*)(p->private_data);
	int old_volume = private_data->volume;
	if(volume < 0)
		volume = 0;
	if(volume > 255)
		volume = 255;
	private_data->volume = volume*256;
	return old_volume/256;
}

void Sirens2Seek(struct audio_play* This, int ms, int flag) {
	sirens2_play_p p = (sirens2_play_p)This;
	volatile sirens2_private_t* private_data = (volatile sirens2_private_t*)(p->private_data);
	
	if ( ! private_data->play_seekable )
		return;

	int64_t seek_time = 0;
	if ( flag == AUDIO_SEEK_SET )
		seek_time = ms * 1000LL;
	else if ( flag == AUDIO_SEEK_CUR )
		seek_time = private_data->current + ms*1000LL;
	else
		seek_time = private_data->duration - ms*1000LL;
	if ( seek_time < 0 )
		seek_time = 0LL;
	if ( seek_time >  private_data->duration )
		seek_time =  private_data->duration;
	
	 private_data->seek_to =  private_data->start_time + seek_time;
	 private_data->play_eof = false;
	 private_data->play_seek = true;
}

void Sirens2Rewind(struct audio_play* This) {
	Sirens2Seek(This, 0, AUDIO_SEEK_SET);	
}

int Sirens2IsEof(struct audio_play* This) {
	sirens2_play_p p = (sirens2_play_p)This;
	volatile sirens2_private_t* private_data = (volatile sirens2_private_t*)(p->private_data);
	return private_data->play_eof ? 1 : 0;
}

int Sirens2IsPaused(struct audio_play* This) {
	sirens2_play_p p = (sirens2_play_p)This;
	volatile sirens2_private_t* private_data = (volatile sirens2_private_t*)(p->private_data);
	return private_data->play_pause ? 1 : 0;
}

static sirens2_play_p CreateSirens2Player() {
	sirens2_play_p p = (sirens2_play_p)malloc_64(sizeof(sirens2_play_t));
	if( p ) {
		memset(p, 0, sizeof(sirens2_play_t));
		p->private_data = (sirens2_private_p)malloc_64(sizeof(sirens2_private_t));
		if ( !p->private_data ) {
			free_64(p);
			return 0;
		}
		
		memset(p->private_data, 0, sizeof(sirens2_private_t));
		Sirens2Initialize(p);
		
		p->private_data->play_channel = sceAudioChReserve(PSP_AUDIO_NEXT_CHANNEL, 1024, PSP_AUDIO_FORMAT_STEREO);
		if ( p->private_data->play_channel < 0 ) {
			Sirens2Finalize(p);
			free_64(p);
			return 0;
		}
			
		p->private_data->play_thread = sceKernelCreateThread("play_thread", Sirens2PlayThread, 0x12, 0x10000, PSP_THREAD_ATTR_USER, 0);
		if (p->private_data->play_thread < 0){
			Sirens2Finalize(p);
			free_64(p);
			return 0;
		}
		
		p->private_data->play_thread_running = true;
		int ret = sceKernelStartThread(p->private_data->play_thread, 4, &p->private_data);
		if ( ret != 0 ) {
			Sirens2Finalize(p);
			free_64(p);
			return 0;
		}
		p->private_data->play_thread_started = true;
		
		p->load = Sirens2Load;
		p->load_buf = Sirens2LoadBuf;
		p->play = Sirens2Play;
		p->playstop = Sirens2PlayStop;
		p->pause = Sirens2Pause;
		p->stop = Sirens2Stop;
		p->resume = Sirens2Resume;
		p->volume = Sirens2Volume;
		p->rewind = Sirens2Rewind;
		p->seek = Sirens2Seek;
		p->iseof = Sirens2IsEof;
		p->ispaused = Sirens2IsPaused;
		p->destroy = Sirens2Destroy;
	}
	return p;
}
//MP3
audio_play_p CreateMp3Player() {
	sirens2_play_p p = CreateSirens2Player();
	if (p != 0) {
		p->audio_type = COOLAUDIO_SIRENS2_MP3;
	}
	return (audio_play_p)p;
}
//wav
audio_play_p CreateWavPlayer() {
	sirens2_play_p p = CreateSirens2Player();
	if (p != 0) {
		p->audio_type = COOLAUDIO_SIRENS2_PCM;
	}
	return (audio_play_p)p;
}
//ogg
audio_play_p CreateOggPlayer() {
	sirens2_play_p p = CreateSirens2Player();
	if (p != 0) {
		p->audio_type = COOLAUDIO_SIRENS2_OGG;
	}
	return (audio_play_p)p;
}
