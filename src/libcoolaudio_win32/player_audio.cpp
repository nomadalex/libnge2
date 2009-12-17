#include "audio_interface.h"
#include "win32_audio_builder.h"


typedef struct audio_play_private{
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
	//private
	IDSoundObject* private_player;
	int            type;
}audio_play_private_t,*audio_play_private_p;

static int  AUDIO_load (struct audio_play* This,const char* filename);
static int  AUDIO_play (struct audio_play* This,int times,int free_when_stop);
static void AUDIO_pause(struct audio_play* This);
static int  AUDIO_stop (struct audio_play* This);
static void AUDIO_resume(struct audio_play* This);
static int  AUDIO_volume (struct audio_play* This,int volume);
static void AUDIO_rewind(struct audio_play* This);
static void AUDIO_seek (struct audio_play* This,int ms,int flag);
static int  AUDIO_iseof (struct audio_play* This);
static int  AUDIO_ispaused(struct audio_play* This);
static int  AUDIO_destroy(struct audio_play* This);



audio_play_p CreateWavPlayer()
{
	audio_play_private_p player = (audio_play_private_p)malloc(sizeof(audio_play_private_t));
	memset(player,0,sizeof(audio_play_private_t));

	if(pBuilder == NULL)
		return NULL;
	player->type = ST_WAVE;
	player->load = AUDIO_load;
	player->play = AUDIO_play;

	player->iseof   = AUDIO_iseof;
	player->destroy = AUDIO_destroy;
	player->ispaused = AUDIO_ispaused;
	player->resume  = AUDIO_resume;
	player->pause   = AUDIO_pause;
	player->rewind  = AUDIO_rewind;
	player->stop    = AUDIO_stop;
	player->seek    = AUDIO_seek;
	player->volume  = AUDIO_volume;
	return (audio_play_p)player;
}

audio_play_p CreateMp3Player()
{
	audio_play_private_p player = (audio_play_private_p)malloc(sizeof(audio_play_private_t));
	memset(player,0,sizeof(audio_play_private_t));
	
	if(pBuilder == NULL)
		return NULL;
	player->type = ST_MP3;
	player->load = AUDIO_load;
	player->play = AUDIO_play;
	return (audio_play_p)player;
}

audio_play_p CreateOggPlayer()
{
	audio_play_private_p player = (audio_play_private_p)malloc(sizeof(audio_play_private_t));
	memset(player,0,sizeof(audio_play_private_t));
	
	if(pBuilder == NULL)
		return NULL;
	player->type = ST_OGG;
	player->load = AUDIO_load;
	player->play = AUDIO_play;
	return (audio_play_p)player;
}


int AUDIO_load (struct audio_play* This,const char* filename)
{
	if(This == NULL)
		return 0;

	if(pBuilder == NULL)
		return 0;
	if( ((audio_play_private_p)This)->private_player !=NULL ){
		delete ((audio_play_private_p)This)->private_player;
	}
	IDSoundObject* player = pBuilder->BuildSoundObject(((audio_play_private_p)This)->type);
    ((audio_play_private_p)This)->private_player = player;
	
	return player->Load(filename);
}

int AUDIO_play (struct audio_play* This,int times,int free_when_stop)
{
	if(This == NULL)
		return 0;
	IDSoundObject* player = ((audio_play_private_p)This)->private_player;
	if(player == NULL)
		return 0;
	// only play(*,1,0) now!
	player->Play();

	return 1;
}


void AUDIO_pause(struct audio_play* This)
{
	if(This == NULL)
		return ;
	IDSoundObject* player = ((audio_play_private_p)This)->private_player;
	if(player == NULL)
		return ;
	player->Pause();
}

int  AUDIO_stop (struct audio_play* This)
{
	if(This == NULL)
		return 0;
	IDSoundObject* player = ((audio_play_private_p)This)->private_player;
	if(player == NULL)
		return 0;
	player->Stop();
	delete player;
	((audio_play_private_p)This)->private_player = NULL;
	return 1;
}

void AUDIO_resume(struct audio_play* This)
{
	if(This == NULL)
		return ;
	IDSoundObject* player = ((audio_play_private_p)This)->private_player;
	if(player == NULL)
		return ;
	player->Resume();
}

int  AUDIO_volume (struct audio_play* This,int volume)
{
	if(This == NULL)
		return 0;
	IDSoundObject* player = ((audio_play_private_p)This)->private_player;
	if(player == NULL)
		return 0;
	int old = player->GetVolume();
	player->SetVolume(volume);
	return old;
}

void AUDIO_rewind(struct audio_play* This)
{
	if(This == NULL)
		return ;
	IDSoundObject* player = ((audio_play_private_p)This)->private_player;
	if(player == NULL)
		return ;
	player->ReWind();
}

void AUDIO_seek (struct audio_play* This,int ms,int flag)
{
	if(This == NULL)
		return ;
	IDSoundObject* player = ((audio_play_private_p)This)->private_player;
	if(player == NULL)
		return ;
	printf("NOT SUPPORT YET !\n");
}

int  AUDIO_iseof (struct audio_play* This)
{
	if(This == NULL)
		return -1;
	IDSoundObject* player = ((audio_play_private_p)This)->private_player;
	if(player == NULL)
		return 0;
	return player->IsEos();
}

int  AUDIO_ispaused(struct audio_play* This)
{
	if(This == NULL)
		return 0;
	IDSoundObject* player = ((audio_play_private_p)This)->private_player;
	if(player == NULL)
		return 0;
	return 1;
}

int  AUDIO_destroy(struct audio_play* This)
{
	if(This == NULL)
		return 0;
	IDSoundObject* player = ((audio_play_private_p)This)->private_player;
	if(player == NULL)
		return 0;
	AUDIO_stop(This);
	if(This)
		free(This);
	return 1;
}