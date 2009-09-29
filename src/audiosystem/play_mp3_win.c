#include "interface.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "SDL.h"
#include "SDL_mixer.h"
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


void MP3PlayInit(music_ops* ops)
{
	
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
    strncpy(ops->extension,"MP3",4);

}

void MP3PlayFini(void)
{
	CleanUp();
	SDL_Quit();
}

int  MP3_Volume(int volume)
{
	int oldvolume = audio_volume;
	audio_volume = volume;
	/* Set the music volume */
	Mix_VolumeMusic(audio_volume);
	return oldvolume;
}



int  MP3_Load (const char *name)
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

int  MP3_Play (void)
{
	if(music)
		return Mix_FadeInMusic(music,0,2000);
	return 0;
}

int  MP3_PlayStop(void)
{
	if(music)
		return Mix_FadeInMusic(music,0,2000);
	return 0;
}

void MP3_Pause(void)
{
	Mix_PauseMusic();
}
int  MP3_Stop (void)
{
	if(music){
		Mix_FreeMusic(music);
		music = NULL;
	}
	return 1;
}
void MP3_Resume (void)
{
	Mix_ResumeMusic();
}
void MP3_Seek (int pos)
{
	printf("not supported\n");
}
int  MP3_Time (void)
{
	printf("not supported\n");
	return 0;
}
int  MP3_Eos  (void)
{
	return Mix_PlayingMusic()==1?0:1;
}