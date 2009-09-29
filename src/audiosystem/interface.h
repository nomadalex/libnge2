#ifndef _INTERFACE_H_____
#define _INTERFACE_H_____


typedef int (*fd_load) (const char *);

typedef int (*fd_play) (void);

typedef int (*fd_playstop) (void);

typedef void (*fd_pause) (void);

typedef int (*fd_stop) (void);

typedef void (*fd_resume)(void);

typedef void (*fd_seek) (int);

typedef int (*fd_time) (void);

typedef int (*fd_eos) (void);

typedef int (*fd_volume) (int);


typedef struct {
    fd_load     load;
    fd_play     play;
    fd_playstop playstop;
    fd_pause    pause;
    fd_stop     stop;
    fd_resume   resume;
    fd_seek     seek;
    fd_time     time;
    fd_eos      eos;
	  fd_volume   volume;
    char        extension[256];        
} music_ops;

//the interface
#ifdef _cplusplus
extern "C"
#endif

//libcoolaudio default init
void CoolAudioDefaultInit();
//libcoolaudio default finish
void CoolAudioDefaultFini();

//ogg support
void OGGPlayInit(music_ops* ops);
void OGGPlayFini(void);

//mp3 support
void MP3PlayInit(music_ops* ops);
void MP3PlayFini(void);

//pcm support
void PCMPlayInit(music_ops* ops);
void PCMPlayFini(void);

//any others will come soon
//......

#ifdef _cplusplus
}
#endif


#endif
