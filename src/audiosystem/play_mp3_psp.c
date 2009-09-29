#include "play_mp3.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pspsdk.h>
#include <pspkernel.h>
#include <pspctrl.h>
#include <pspaudiocodec.h>
#include <pspaudio.h>
#include <string.h>
#include <malloc.h>
#include <pspdisplay.h>


static int  MP3_Load (const char *);
static int  MP3_Play (void);
static int  MP3_PlayStop(void);
static void MP3_Pause(void);
static int  MP3_Stop (void);
static void MP3_Resume(void);
static void MP3_Seek (int);
static int  MP3_Time (void);
static int  MP3_Eos  (void);
static int  MP3_Volume(int);

#define MP3_SAMPLE_COUNT 1152

static int mp3_bitrates[] = {0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320 };
static unsigned long mp3_codec_buffer[65] __attribute__((aligned(64)));
static int mp3_codec_flag = 0;
static volatile SceUID mp3_file_handle = -1;
static unsigned char mp3_header_buffer[4];
static int mp3_data_start = 0;
static int mp3_samplerate;
static int mp3_channels;
static volatile int mp3_volume = PSP_AUDIO_VOLUME_MAX;
static unsigned char* mp3_data_buffer = 0;
static int mp3_output_buffers = 0;
static short mp3_output_buffer[2][MP3_SAMPLE_COUNT*2] __attribute__((aligned(64)));
static SceUID mp3_audio_channel = -1;
static SceUID mp3_audio_thread = -1;
static volatile int mp3_init = 0;
static volatile int mp3_play = 0;
static volatile int mp3_stop = 1;
static volatile int mp3_pause = 0;
static volatile int mp3_run = 1;
static volatile int mp3_clean = 0;




static void InitGlobal() {
	mp3_codec_flag = 0;
	mp3_volume = PSP_AUDIO_VOLUME_MAX;
	mp3_audio_channel = -1;
	mp3_audio_thread = -1;
	mp3_run = 1;
	mp3_clean = 0;
}

static void InitMp3State() {
	mp3_data_buffer = 0;
	mp3_output_buffers = 0;
	
	mp3_init = 0;
	mp3_play = 0;
	mp3_stop = 1;
	mp3_pause = 0;
}

static void InitMp3() {
	mp3_file_handle = -1;
	mp3_data_start = 0;
	InitMp3State();
}
	

static void Init() {
	InitGlobal();
	InitMp3();	
}

static void CleanMp3Resource() {	
	
	if ( !(mp3_file_handle<0)) {
		sceIoClose(mp3_file_handle);
	}
		
	InitMp3();
}
	

static void CleanUp(){
		
	CleanMp3Resource();
	
	if ( mp3_audio_thread >= 0 ) {
		mp3_run = 0;
		sceKernelWaitThreadEnd(mp3_audio_thread,0);
		sceKernelDeleteThread(mp3_audio_thread);
		mp3_audio_thread = -1;
	}
	
	if ( mp3_audio_channel >= 0 ) {
		sceAudioChRelease(mp3_audio_channel);
		mp3_audio_channel = -1;
	}
	
	if ( mp3_codec_flag ) {
		sceAudiocodecReleaseEDRAM(mp3_codec_buffer);
		mp3_codec_flag = 0;
	}
	
	Init();
}

static int MP3OutputThread(SceSize args, void *argp){
	while(mp3_run) {
		if ( mp3_play ) {
			while(mp3_play) {
				if (mp3_pause) {
					sceKernelDelayThread(1000);
					continue;
				}
				if ( sceIoRead( mp3_file_handle, mp3_header_buffer, 4 ) != 4 ) {
					mp3_play = 0;
					continue;
				}
				if ( mp3_header_buffer[0] == 'T' && mp3_header_buffer[1] == 'A' && mp3_header_buffer[2] == 'G' ) {
					mp3_play = 0;
					continue;
				}
				unsigned int mp3_header = mp3_header_buffer[0];
				mp3_header = (mp3_header<<8) | mp3_header_buffer[1];
				mp3_header = (mp3_header<<8) | mp3_header_buffer[2];
				mp3_header = (mp3_header<<8) | mp3_header_buffer[3];
				
				if ( (mp3_header & 0xFFFE0000) != 0xFFFA0000) {
					sceIoLseek32(mp3_file_handle, -3, PSP_SEEK_CUR);
					continue;
				}
				
				int bitrate = (mp3_header & 0xf000) >> 12;
				int padding = (mp3_header & 0x200) >> 9;
				
				int frame_size = 144000*mp3_bitrates[bitrate]/mp3_samplerate + padding;
				
				if ( mp3_data_buffer )
					free(mp3_data_buffer);
				mp3_data_buffer = (unsigned char*)memalign(64, frame_size);
				if ( !mp3_data_buffer ) {
					sceIoLseek32(mp3_file_handle, -4, PSP_SEEK_CUR);
					continue;
				}
				sceIoLseek32(mp3_file_handle, -4, PSP_SEEK_CUR);
				if ( sceIoRead( mp3_file_handle, mp3_data_buffer, frame_size ) != frame_size ) {
					mp3_play = 0;
					continue;
				}
				memset(mp3_output_buffer[mp3_output_buffers], 0, MP3_SAMPLE_COUNT * 4);
				mp3_codec_buffer[6] = (unsigned long)mp3_data_buffer;
				mp3_codec_buffer[8] = (unsigned long)mp3_output_buffer[mp3_output_buffers];
				
				mp3_codec_buffer[7] = mp3_codec_buffer[10] = frame_size;
				mp3_codec_buffer[9] = MP3_SAMPLE_COUNT * 4;
			
				sceAudiocodecDecode(mp3_codec_buffer, 0x1002);
				
				sceAudioOutputBlocking(mp3_audio_channel, mp3_volume, mp3_output_buffer[mp3_output_buffers]);
				
				mp3_output_buffers = (mp3_output_buffers+1)%2;
				
			}
			if ( mp3_data_buffer ){
				free(mp3_data_buffer);
				mp3_data_buffer = 0;
			}
			mp3_stop = 1;
			if ( mp3_clean ) 
				CleanMp3Resource();
				
		}
		sceKernelDelayThread(1000);
	
	}
	sceKernelExitThread(0);
	return 0;
}


void MP3PlayInit(music_ops* ops){
	
	CleanUp();
		
	ops->load  =  MP3_Load;
	ops->play  =  MP3_Play;
	ops->playstop = MP3_PlayStop;
	ops->pause =  MP3_Pause;
	ops->stop  =  MP3_Stop;
	ops->resume=  MP3_Resume;
	ops->time  =  MP3_Time;
	ops->seek  =  MP3_Seek;
	ops->eos   =  MP3_Eos;
	ops->volume = MP3_Volume;
	strncpy(ops->extension,"mp3",4);
	
	memset(mp3_codec_buffer, 0, sizeof(mp3_codec_buffer) );
	if ( sceAudiocodecCheckNeedMem(mp3_codec_buffer, 0x1002) < 0 ) {
		CleanUp();
		return;
	}
	if ( sceAudiocodecGetEDRAM(mp3_codec_buffer, 0x1002) < 0 ) {
		CleanUp();
		return;
	}
	if ( sceAudiocodecInit(mp3_codec_buffer, 0x1002) < 0 ) {
		CleanUp();
		return;
	}
	mp3_codec_flag = 1;
	
	mp3_audio_channel = sceAudioChReserve(PSP_AUDIO_NEXT_CHANNEL, MP3_SAMPLE_COUNT, PSP_AUDIO_FORMAT_STEREO);
	if ( mp3_audio_channel < 0 ) {
		CleanUp();
		return;
	}
	
	mp3_audio_thread = sceKernelCreateThread("mp3_audio_thread",MP3OutputThread,0x12,0x10000,0,NULL);
	
	if ( mp3_audio_thread < 0 ) {
		CleanUp();
		return;
	}
	
	int ret = sceKernelStartThread(mp3_audio_thread, 0, 0);
	if ( ret != 0 ) {
		CleanUp();
		return;
	}
}

void MP3PlayFini(){
	CleanUp();
}
/*
 * volume (0-128)
*/
int  MP3_Volume(int volume){
	//set volume and return old volume
	float rate = 128.0/PSP_AUDIO_VOLUME_MAX;
	int oldvolume = (int)(mp3_volume*rate);
	mp3_volume = (int)(volume*1.0/rate);
	return oldvolume;
}



int  MP3_Load (const char *name){
	if ( !mp3_codec_flag || mp3_audio_channel<0 || mp3_audio_thread<0 )
		return -1;
	if ( mp3_init )
		return -2;
	mp3_file_handle = sceIoOpen(name, PSP_O_RDONLY, 0777);
	if ( mp3_file_handle<0 ) 
		return -3;
	if ( sceIoRead( mp3_file_handle, mp3_header_buffer, 4 ) != 4 ) {
		CleanMp3Resource();
		return -4;
	}
	while(1) {
		if ( mp3_header_buffer[0] == 'I' && mp3_header_buffer[1] == 'D' && mp3_header_buffer[2] == '3' ) {
			unsigned char id3v2_buffer[6];
			if ( sceIoRead( mp3_file_handle, id3v2_buffer, 6 ) != 6 ) {
				CleanMp3Resource();
				return -5;
			}
			int id3v2_size = (int)(id3v2_buffer[2] & 0x7F) << 21 
				| (int)(id3v2_buffer[3] & 0x7F) << 14
				| (int)(id3v2_buffer[4] & 0x7F) << 7
				| (int)(id3v2_buffer[5] & 0x7F);
			sceIoLseek32(mp3_file_handle, id3v2_size, PSP_SEEK_CUR);
			if ( sceIoRead(mp3_file_handle, mp3_header_buffer, 4 ) != 4 ) {
				CleanMp3Resource();
				return -5;
			}
			continue;
		}
		unsigned int mp3_header = mp3_header_buffer[0];
		mp3_header = (mp3_header<<8) | mp3_header_buffer[1];
		mp3_header = (mp3_header<<8) | mp3_header_buffer[2];
		mp3_header = (mp3_header<<8) | mp3_header_buffer[3];
	
		if ( (mp3_header & 0xFFFE0000) != 0xFFFA0000) {
			sceIoLseek32(mp3_file_handle, -3, PSP_SEEK_CUR);
			if ( sceIoRead(mp3_file_handle, mp3_header_buffer, 4 ) != 4 ) {
				CleanMp3Resource();
				return -5;
			}
			continue;
		}
		mp3_samplerate = (mp3_header & 0x0C00) >> 10;
		if ( mp3_samplerate != 0 && mp3_samplerate != 1 ) {
			CleanMp3Resource();
			return -6;
		}
		mp3_samplerate = (mp3_samplerate == 0)?44100:48000;
	
		mp3_channels = (mp3_header & 0x00C0) >> 6;
		mp3_channels = (mp3_channels == 3)?1:2;
	
		mp3_data_start = sceIoLseek32(mp3_file_handle, -4, PSP_SEEK_CUR);
		
		break;
	}
	
	InitMp3State();
	
	mp3_init = 1;
	
	return 1;
}


int  MP3_Play (void) {
	if ( !mp3_init )
		return 0;
	if ( mp3_play )
		return mp3_play;
	sceIoLseek32(mp3_file_handle, mp3_data_start, PSP_SEEK_SET);
	mp3_clean = 0;
	mp3_pause = 0;
	mp3_stop = 0;
	mp3_play = 1;
	return 1;
}

int  MP3_PlayStop (void) {
	if ( !mp3_init )
		return 0;
	if ( mp3_play )
		return mp3_play;
	sceIoLseek32(mp3_file_handle, mp3_data_start, PSP_SEEK_SET);
	mp3_clean = 1;
	mp3_pause = 0;
	mp3_stop = 0;
	mp3_play = 1;
	return 1;
}

void MP3_Pause(void){
	mp3_pause = 1;
}

int  MP3_Stop (void){
	if ( !mp3_init )
		return 1;
	mp3_play = 0;
	if ( mp3_clean ) {
		while( mp3_init ) {
			sceKernelDelayThread(1000);
		}
	}
	else {
		while( !mp3_stop ) {
			sceKernelDelayThread(1000);
		}
		CleanMp3Resource();
	}
	return 1;
}

void MP3_Resume (void){
	mp3_pause = 0;
}

void MP3_Seek (int pos){
	printf("not supported\n");
}

int  MP3_Time (void){
	printf("not supported\n");
	return 0;
}

int  MP3_Eos  (void){
	return mp3_stop;
}
