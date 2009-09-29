#include "interface.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "SDL.h"
#include "SDL_mixer.h"
static int  PCM_Load (const char *);
static int  PCM_Play (void);
static int  PCM_PlayStop(void);
static void PCM_Pause(void);
static int  PCM_Stop (void);
static void PCM_Resume(void);
static void PCM_Seek (int);
static int  PCM_Time (void);
static int  PCM_Eos  (void);
static int  PCM_Volume(int);

static int audio_rate;
static Uint16 audio_format;
static int audio_channels;
static int audio_buffers;
static int audio_volume = MIX_MAX_VOLUME;
static int audio_open = 0;
static Mix_Music *music = NULL;

static void CleanUp()
{
	if ( music ) {
		Mix_FreeMusic(music);
		music = NULL;
	}
	if ( audio_open ) {
		Mix_CloseAudio();
		audio_open = 0;
	}
}


void PCMPlayInit(music_ops* ops)
{
	
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
    strncpy(ops->extension,"MP3",4);

}

void PCMPlayFini(void)
{
	CleanUp();
	SDL_Quit();
}

int  PCM_Volume(int volume)
{
	int oldvolume = audio_volume;
	audio_volume = volume;
	/* Set the music volume */
	Mix_VolumeMusic(audio_volume);
	return oldvolume;
}



int  PCM_Load (const char *name)
{
	/* Initialize variables */
	audio_rate = 22050;
	audio_format = AUDIO_S16;
	audio_channels = 2;
	audio_buffers = 4096;
	
	/* Open the audio device */
	if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) < 0) {
		fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
		return(2);
	} else {
		Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels);
		/*printf("Opened audio at %d Hz %d bit %s (%s), %d bytes audio buffer\n", audio_rate,
			(audio_format&0xFF),
			(audio_channels > 2) ? "surround" : (audio_channels > 1) ? "stereo" : "mono", 
			(audio_format&0x1000) ? "BE" : "LE",
			audio_buffers );*/
	}
	audio_open = 1;

	/* Set the music volume */
	Mix_VolumeMusic(audio_volume);

	/* Set the external music player, if any */
	Mix_SetMusicCMD(getenv("MUSIC_CMD"));
	if(music){
		Mix_FreeMusic(music);
		music = NULL;
	}
	music = Mix_LoadMUS(name);
	if ( music == NULL ) {
		fprintf(stderr, "Couldn't load %s,%s\n",name, SDL_GetError());
		CleanUp();
		exit(0);
	}
	return 1;
}

int  PCM_Play (void)
{
	if(music)
		return Mix_FadeInMusic(music,0,2000);
	return 0;
}

int  PCM_PlayStop(void)
{
	if(music)
		return Mix_FadeInMusic(music,0,2000);
	return 0;
}

void PCM_Pause(void)
{
	Mix_PauseMusic();
}
int  PCM_Stop (void)
{
	if(music){
		Mix_FreeMusic(music);
		music = NULL;
	}
	return 1;
}
void PCM_Resume (void)
{
	Mix_ResumeMusic();
}
void PCM_Seek (int pos)
{
	printf("not supported\n");
}
int  PCM_Time (void)
{
	printf("not supported\n");
	return 0;
}
int  PCM_Eos  (void)
{
	return Mix_PlayingMusic()==1?0:1;
}