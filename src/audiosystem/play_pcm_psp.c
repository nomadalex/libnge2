#include "play_pcm.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pspsdk.h>
#include <pspkernel.h>
#include <pspctrl.h>
#include <pspaudio.h>
#include <string.h>
#include <malloc.h>

static int  PCM_Load (const char *);
static int  PCM_Play (void);
static int  PCM_PlayStop (void);
static void PCM_Pause(void);
static int  PCM_Stop (void);
static void PCM_Resume(void);
static void PCM_Seek (int);
static int  PCM_Time (void);
static int  PCM_Eos  (void);
static int  PCM_Volume(int);

#define PCM_SAMPLE_COUNT 4096

static SceUID pcm_file_handle = -1;
static unsigned char pcm_header_buffer[44];
static int pcm_data_start = 0;
static int pcm_samplerate;
static int pcm_channels;
static int pcm_total_sample;
static volatile int pcm_current_sample = 0;
static volatile int pcm_volume = PSP_AUDIO_VOLUME_MAX;
static int pcm_output_buffers = 0;
static short pcm_output_buffer[2][PCM_SAMPLE_COUNT*2] __attribute__((aligned(64)));
static SceUID pcm_audio_channel = -1;
static SceUID pcm_audio_thread = -1;
static volatile int pcm_init = 0;
static volatile int pcm_play = 0;
static volatile int pcm_stop = 1;
static volatile int pcm_pause = 0;
static volatile int pcm_run = 1;
static volatile int pcm_clean = 0;


static void InitGlobal() {
	pcm_volume = PSP_AUDIO_VOLUME_MAX;
	pcm_audio_channel = -1;
	pcm_audio_thread = -1;
	pcm_run = 1;
	pcm_clean = 0;
}

static void InitPCMState() {
	pcm_output_buffers = 0;
	
	pcm_init = 0;
	pcm_play = 0;
	pcm_stop = 1;
	pcm_pause = 0;
}

static void InitPCM() {
	pcm_file_handle = -1;
	pcm_data_start = 0;
	pcm_current_sample = 0;
	InitPCMState();
}
	

static void Init() {
	InitGlobal();
	InitPCM();	
}

static void CleanPCMResource() {	
	
	if ( !(pcm_file_handle<0) ) {
		sceIoClose(pcm_file_handle);
	}
	
	InitPCM();
}

static void CleanUp(){
	CleanPCMResource();
	
	if ( pcm_audio_thread >= 0 ) {
		pcm_run = 0;
		sceKernelWaitThreadEnd(pcm_audio_thread,0);
		sceKernelDeleteThread(pcm_audio_thread);
		pcm_audio_thread = -1;
	}
	
	if ( pcm_audio_channel >= 0 ) {
		sceAudioChRelease(pcm_audio_channel);
		pcm_audio_channel = -1;
	}
	
	Init();
}

static int PCMOutputThread(SceSize args, void *argp){
	while(pcm_run) {
		if ( pcm_play ) {
			while(pcm_play) {
				if (pcm_pause) {
					sceKernelDelayThread(1000);
					continue;
				}
				int need_sample = ( pcm_total_sample - pcm_current_sample > PCM_SAMPLE_COUNT ) ? PCM_SAMPLE_COUNT : (pcm_total_sample - pcm_current_sample);
				if ( need_sample <= 0 ) {
					pcm_play = 0;
					continue;
				}
				memset(pcm_output_buffer[pcm_output_buffers], 0, PCM_SAMPLE_COUNT * 4);
				if ( sceIoRead( pcm_file_handle, pcm_output_buffer[pcm_output_buffers], need_sample*2*pcm_channels ) != need_sample*2*pcm_channels ) {
					pcm_play = 0;
					continue;
				}
				if ( pcm_channels == 1)  {
					int i = 0;
					for(i=PCM_SAMPLE_COUNT-1; i>=0;i--) {
						pcm_output_buffer[pcm_output_buffers][2*i+1] = pcm_output_buffer[pcm_output_buffers][i];
						pcm_output_buffer[pcm_output_buffers][2*i] = pcm_output_buffer[pcm_output_buffers][i];
					}
						
				}
				pcm_current_sample += need_sample;
				sceAudioOutputBlocking(pcm_audio_channel, pcm_volume,pcm_output_buffer[pcm_output_buffers]);
				
				pcm_output_buffers = (pcm_output_buffers+1)%2;
				
			}
			pcm_stop = 1;
			if ( pcm_clean ) 
				CleanPCMResource();
		}
		sceKernelDelayThread(1000);
	}
	sceKernelExitThread(0);
	return 0;
}


void PCMPlayInit(music_ops* ops){
	
	ops->load  =  PCM_Load;
	ops->play  =  PCM_Play;
	ops->playstop = PCM_PlayStop;
	ops->pause =  PCM_Pause;
	ops->stop  =  PCM_Stop;
	ops->resume=  PCM_Resume;
	ops->time  =  PCM_Time;
	ops->seek  =  PCM_Seek;
	ops->eos   =  PCM_Eos;
	ops->volume = PCM_Volume;
	strncpy(ops->extension,"wav",4);
	
	pcm_audio_channel = sceAudioChReserve(PSP_AUDIO_NEXT_CHANNEL, PCM_SAMPLE_COUNT, PSP_AUDIO_FORMAT_STEREO);
	if ( pcm_audio_channel < 0 ) {
		CleanUp();
		return;
	}
	
	pcm_audio_thread = sceKernelCreateThread("pcm_audio_thread",PCMOutputThread,0x12,0x10000,0,NULL);
	
	if ( pcm_audio_thread < 0 ) {
		CleanUp();
		return;
	}
	
	int ret = sceKernelStartThread(pcm_audio_thread, 0, 0);
	if ( ret != 0 ) {
		CleanUp();
		return;
	}
}

void PCMPlayFini(){
	CleanUp();
}
/*
 * volume (0-128)
*/
int  PCM_Volume(int volume){
	//set volume and return old volume
	float rate = 128.0/PSP_AUDIO_VOLUME_MAX;
	int oldvolume = (int)(pcm_volume*rate);
	pcm_volume = (int)(volume*1.0/rate);
	return oldvolume;
}



int  PCM_Load (const char *name){
	
	if ( pcm_audio_channel<0 || pcm_audio_thread<0 )
		return -1;
	if ( pcm_init )
		return -1;
	
	pcm_file_handle = sceIoOpen(name, PSP_O_RDONLY, 0777);
	if ( pcm_file_handle<0 ) 
		return -1;
	if ( sceIoRead( pcm_file_handle, pcm_header_buffer, 44 ) != 44 ) {
		CleanPCMResource();
		return -2;
	}
	pcm_data_start = sceIoLseek32(pcm_file_handle, 0, PSP_SEEK_CUR);
	
	if ( pcm_header_buffer[0] != 'R' || pcm_header_buffer[1] != 'I' || pcm_header_buffer[2] != 'F' || pcm_header_buffer[3] != 'F') {
		CleanPCMResource();
		return -31;
	}
	if ( pcm_header_buffer[8] != 'W' || pcm_header_buffer[9] != 'A' || pcm_header_buffer[10] != 'V' || pcm_header_buffer[11] != 'E'
		|| pcm_header_buffer[12] != 'f' || pcm_header_buffer[13] != 'm' || pcm_header_buffer[14] != 't' || pcm_header_buffer[15] != ' ') {
		CleanPCMResource();
		return -32;
	}
	if ( pcm_header_buffer[16] != 0x10 || pcm_header_buffer[17] != 0x00 || pcm_header_buffer[18] != 0x00 || pcm_header_buffer[19] != 0x00) {
		CleanPCMResource();
		return -33;
	}
	if ( pcm_header_buffer[20] != 0x01 || pcm_header_buffer[21] != 0x00 ) {
		CleanPCMResource();
		return -34;
	}
	pcm_channels = (pcm_header_buffer[23] << 8) | pcm_header_buffer[22];
	pcm_samplerate = (pcm_header_buffer[25] << 8) | pcm_header_buffer[24];
	if ( pcm_samplerate != 44100 && pcm_samplerate != 48000 ) { // only support 44.1khz and 48khz pcm
		CleanPCMResource();
		return -35;
	}
	if ( pcm_header_buffer[34] != 0x10 || pcm_header_buffer[35] != 0x00 ) { // only support 16bit pcm
		CleanPCMResource();
		return -36;
	}
	unsigned int pcm_total_data = pcm_header_buffer[43];
	pcm_total_data = (pcm_total_data << 8 ) | pcm_header_buffer[42];
	pcm_total_data = (pcm_total_data << 8 ) | pcm_header_buffer[41];
	pcm_total_data = (pcm_total_data << 8 ) | pcm_header_buffer[40];
	
	pcm_total_sample = pcm_total_data/2/pcm_channels;
	
	InitPCMState();
	
	pcm_init = 1;
	
	return 1;
}


int  PCM_Play (void) {
	if ( !pcm_init )
		return 0;
	if ( pcm_play )
		return pcm_play;
	sceIoLseek32(pcm_file_handle, pcm_data_start, PSP_SEEK_SET);
	pcm_current_sample = 0;
	pcm_clean = 0;
	pcm_pause = 0;
	pcm_stop = 0;
	pcm_play = 1;
	return 1;
}

int  PCM_PlayStop (void) {
	if ( !pcm_init )
		return 0;
	if ( pcm_play )
		return pcm_play;
	sceIoLseek32(pcm_file_handle, pcm_data_start, PSP_SEEK_SET);
	pcm_current_sample = 0;
	pcm_clean = 1;
	pcm_pause = 0;
	pcm_stop = 0;
	pcm_play = 1;
	return 1;
}

void PCM_Pause(void){
	pcm_pause = 1;
}

int  PCM_Stop (void){
	if ( !pcm_init )
		return 1;
	pcm_play = 0;
	if ( pcm_clean ) {
		while( pcm_init ) {
			sceKernelDelayThread(1000);
		}
	}
	else {
		while( !pcm_stop ) {
			sceKernelDelayThread(1000);
		}
		CleanPCMResource();
	}
	return 1;
}

void PCM_Resume (void){
	pcm_pause = 0;
}

void PCM_Seek (int pos){
	printf("not supported\n");
}

int  PCM_Time (void){
	printf("not supported\n");
	return 0;
}

int  PCM_Eos  (void){
	return pcm_stop;
}
