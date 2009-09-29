#ifndef NGE_MOVIE_PLAY_H
#define NGE_MOVIE_PLAY_H
//use libpmp to play movie


// load file and do file init
typedef int (*md_load) (const char *);
// play movie
typedef int (*md_play) (void);
// return 1 if movie is end 
typedef int (*md_eos) (void);
//stop movie and free CURRENT FILE resource
typedef int (*md_stop) (void);

typedef struct {
	md_load     load;
    md_play     play;
    md_stop     stop;
    md_eos      eos;
    char        extension[256];       
}movie_ops;

//the interface
#ifdef _cplusplus
extern "C"
#endif

void MoviePlayInit(movie_ops *ops);
void MoviePlayFini(void);

#ifdef _cplusplus
}
#endif



#endif

