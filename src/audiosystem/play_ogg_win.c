#include "interface.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "SDL.h"
#include "SDL_mixer.h"
static int  OGG_Load (const char *);
static int  OGG_Play (void);
static int  OGG_PlayStop(void);
static void OGG_Pause(void);
static int  OGG_Stop (void);
static void OGG_Resume(void);
static void OGG_Seek (int);
static int  OGG_Time (void);
static int  OGG_Eos  (void);
static int  OGG_Volume(int);

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


void OGGPlayInit(music_ops* ops)
{
	
    ops->load  =  OGG_Load;
    ops->play  =  OGG_Play;
	ops->playstop = OGG_PlayStop;
    ops->pause =  OGG_Pause;
    ops->stop  =  OGG_Stop;
    ops->resume=  OGG_Resume;
    ops->time  =  OGG_Time;
    ops->seek  =  OGG_Seek;
    ops->eos   =  OGG_Eos;
    ops->volume = OGG_Volume;
    strncpy(ops->extension,"MP3",4);

}

void OGGPlayFini(void)
{
	CleanUp();
	SDL_Quit();
}

int  OGG_Volume(int volume)
{
	int oldvolume = audio_volume;
	audio_volume = volume;
	/* Set the music volume */
	Mix_VolumeMusic(audio_volume);
	return oldvolume;
}



int  OGG_Load (const char *name)
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

int  OGG_Play (void)
{
	if(music)
		return Mix_FadeInMusic(music,0,2000);
	return 0;
}

int  OGG_PlayStop(void)
{
	if(music)
		return Mix_FadeInMusic(music,0,2000);
	return 0;
}

void OGG_Pause(void)
{
	Mix_PauseMusic();
}
int  OGG_Stop (void)
{
	if(music){
		Mix_FreeMusic(music);
		music = NULL;
	}
	return 1;
}
void OGG_Resume (void)
{
	Mix_ResumeMusic();
}
void OGG_Seek (int pos)
{
	printf("not supported\n");
}
int  OGG_Time (void)
{
	printf("not supported\n");
	return 0;
}
int  OGG_Eos  (void)
{
	return Mix_PlayingMusic()==1?0:1;
}