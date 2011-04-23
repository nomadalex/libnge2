/* nge_audio.cpp
 */

#if defined(WIN32) || defined(__linux__)
#include <stdio.h>
#include <stdlib.h>
#include "SDL/SDL_mixer.h"
#include "audio_interface.h"

class AudioPlayer {
public:
	virtual ~AudioPlayer()
		{
		}

	virtual int load(const char * fn)
		{
			return load_RW(SDL_RWFromFile(fn, "rb"));
		}

	virtual int loadBuf(const char * buf, int size)
		{
			return load_RW(SDL_RWFromConstMem(buf, size));
		}

	virtual int loadFP(int handle,char closed_by_me)
		{
			printf("not supported!");
			return 0;
		}


	virtual int play(int times, bool free_when_stop)
		{
			printf("not supported!");
			return 0;
		}

	virtual void pause()
		{
			printf("not supported!");
		}

	virtual int stop()
		{
			printf("not supported!");
			return 0;
		}

	virtual void resume()
		{
			printf("not supported!");
		}

	virtual void rewind()
		{
			printf("not supported!");
		}

	virtual void seek(int ms, int flag)
		{
			printf("not supported!");
		}


	virtual int setVolume(int volume)
		{
			printf("not supported!");
			return 1;
		}


	virtual bool isEof()
		{
			printf("not supported!");
			return false;
		}

	virtual bool isPaused()
		{
			printf("not supported!");
			return false;
		}

	virtual int load_RW(SDL_RWops * rw) = 0;
};

class MUSPlayer : public AudioPlayer {
public:
	static MUSPlayer* instance();
	virtual ~MUSPlayer();

	virtual int play(int times, bool free_when_stop)
		{
			if(itsMusic)
				return Mix_PlayMusic(itsMusic, times);
			return 0;
		}

	virtual void pause()
		{
			Mix_PauseMusic();
		}

	virtual int stop()
		{
			return Mix_HaltMusic();
		}

	virtual void resume() {
		Mix_ResumeMusic();
	}

	virtual int setVolume(int volume)
		{
			int oldvolume = itsVolume;
			itsVolume = volume;
			/* Set the music volume */
			Mix_VolumeMusic(itsVolume);
			return oldvolume;
		}

	virtual void rewind()
		{
			Mix_RewindMusic();
		}

	virtual void seek(int ms, int flag)
		{
			Mix_SetMusicPosition(((double)ms) / 1000);
		}

	virtual bool isEof()
		{
			return !Mix_PlayingMusic();
		}

	virtual bool isPaused()
		{
			return Mix_PausedMusic();
		}

private:
	MUSPlayer();
	virtual int load_RW(SDL_RWops * rw)
		{
			if(itsMusic){
				Mix_FreeMusic(itsMusic);
			}
			itsMusic = Mix_LoadMUS_RW(rw);
			if ( !itsMusic ) {
				fprintf(stderr, "Couldn't load music, %s\n", Mix_GetError());
				return 0;
			}
			return 1;
		}

	static MUSPlayer* p_instance;
	int itsVolume;
	Mix_Music * itsMusic;
};

inline MUSPlayer::MUSPlayer() :  itsVolume(MIX_MAX_VOLUME), itsMusic(NULL)
{
}

inline MUSPlayer::~MUSPlayer()
{
	Mix_FreeMusic(itsMusic);
	p_instance = NULL;
}

MUSPlayer* MUSPlayer::p_instance = NULL;
inline MUSPlayer* MUSPlayer::instance()
{
	if (p_instance == NULL)
		p_instance = new MUSPlayer();
	return p_instance;
}

class WAVPlayer : public AudioPlayer {
public:
	WAVPlayer();
	virtual ~WAVPlayer();

	virtual int play(int times, bool free_when_stop)
		{
			if(itsChunk) {
				itsChannel = Mix_PlayChannel(-1, itsChunk, times);
				if (itsChannel)
					return 1;
			}
			return 0;
		}

	virtual void pause()
		{
			Mix_Pause(itsChannel);
		}

	virtual bool isPaused()
		{
			return Mix_Paused(itsChannel);
		}

	virtual int stop()
		{
			return Mix_HaltChannel(itsChannel);
		}

	virtual void resume()
		{
			Mix_Resume(itsChannel);
		}

	virtual int setVolume(int volume)
		{
			int oldvolume = itsVolume;
			itsVolume = volume;
			/* Set the music volume */
			Mix_VolumeChunk(itsChunk, itsVolume);
			return oldvolume;
		}

	virtual bool isEof()
		{
			return !Mix_Playing(itsChannel);
		}

private:
	virtual int load_RW(SDL_RWops * rw)
		{
			if(itsChunk){
				Mix_FreeChunk(itsChunk);
			}
			itsChunk = Mix_LoadWAV_RW(rw, 1);
			if ( !itsChunk ) {
				fprintf(stderr, "Couldn't load music, %s\n", Mix_GetError());
				return 0;
			}
			return 1;
		}

	int itsVolume;
	int itsChannel;
	Mix_Chunk * itsChunk;
};

inline WAVPlayer::WAVPlayer() :  itsVolume(MIX_MAX_VOLUME), itsChannel(0), itsChunk(NULL)
{
}

inline WAVPlayer::~WAVPlayer()
{
	Mix_FreeChunk(itsChunk);
}

typedef struct audio_play_ext {
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
	AudioPlayer * pAP;
} audio_ext_t;

static int apdater_load (audio_play* This, const char* filename)
{
	return ((audio_ext_t*)This)->pAP->load(filename);
}

static int apdater_load_buf (audio_play* This,const char* buf,int size)
{
	return ((audio_ext_t*)This)->pAP->loadBuf(buf, size);
}

static int apdater_load_fp (audio_play* This,int handle,char closed_by_me)
{
	return ((audio_ext_t*)This)->pAP->loadFP(handle, closed_by_me);
}

static int apdater_play (audio_play* This,int times,int free_when_stop)
{
	return ((audio_ext_t*)This)->pAP->play(times, free_when_stop);
}

static int apdater_playstop (audio_play* This)
{
	return ((audio_ext_t*)This)->pAP->play(1, 0);
}

static void apdater_pause (audio_play* This)
{
	return ((audio_ext_t*)This)->pAP->pause();
}

static int apdater_stop (audio_play* This)
{
	return ((audio_ext_t*)This)->pAP->stop();
}

static void apdater_resume(audio_play* This)
{
	return ((audio_ext_t*)This)->pAP->resume();
}

static int apdater_volume (audio_play* This,int volume)
{
	return ((audio_ext_t*)This)->pAP->setVolume(volume);
}

static void apdater_rewind(audio_play* This)
{
	return ((audio_ext_t*)This)->pAP->rewind();
}

static void apdater_seek (audio_play* This,int ms,int flag)
{
	return ((audio_ext_t*)This)->pAP->seek(ms, flag);
}

static int apdater_iseof (audio_play* This)
{
	return ((audio_ext_t*)This)->pAP->isEof();
}

static int apdater_ispaused(audio_play* This)
{
	return ((audio_ext_t*)This)->pAP->isPaused();
}

static int apdater_destroy(audio_play* This)
{
	delete ((audio_ext_t*)This)->pAP;
	free(This);
	return 0;
}

static audio_ext_t* audio_create()
{
	audio_ext_t * pa = (audio_ext_t *)malloc(sizeof(audio_ext_t));
	if (!pa)
	{
		fprintf(stderr, "MEM alloc error.");
		return NULL;
	}

#define SET_VALUE(name) pa->name = apdater_##name
	SET_VALUE(load);
	SET_VALUE(load_fp);
	SET_VALUE(load_buf);
	SET_VALUE(play);
	SET_VALUE(playstop);
	SET_VALUE(pause);
	SET_VALUE(stop);
	SET_VALUE(resume);
	SET_VALUE(volume);
	SET_VALUE(rewind);
	SET_VALUE(seek);
	SET_VALUE(iseof);
	SET_VALUE(ispaused);
	SET_VALUE(destroy);
#undef SET_VALUE

	return pa;
}

//MP3
audio_play_p CreateMp3Player()
{
	static audio_ext_t * p_audio = NULL;
	if (!p_audio) {
		p_audio = audio_create();
		p_audio->pAP = dynamic_cast<AudioPlayer*>(MUSPlayer::instance());
	}

	return (audio_play_p) p_audio;
}

//ogg
audio_play_p CreateOggPlayer()
{
	return CreateMp3Player();
}

//wav
audio_play_p CreateWavPlayer()
{
	static audio_ext_t * p_audio = NULL;
	if (!p_audio) {
		p_audio = audio_create();
		p_audio->pAP = dynamic_cast<AudioPlayer*>(new WAVPlayer());
	}

	return (audio_play_p) p_audio;
}

#endif // defined(WIN32) || defined(__linux__)
