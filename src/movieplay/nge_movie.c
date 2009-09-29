#include "nge_movie.h"


static int  PMP_Load (const char *);
static int  PMP_Play (void);
static int  PMP_Stop (void);
static int  PMP_Eos  (void);



// Call once on startup to initialize pmp playback (loads the codecs)
char* pmp_init();

// play file s and decode in pixelformat 'format' (see pspmpeg.h for modes, possible are 4444,5650,5551 and 8888)
// if show is set to 1 the display of the video will be handled by the output thread
// else the caller has to display the video itself by querying the frames with pmp_get_frame and somehow getting them
// displayed on screen
char *pmp_play(char *s, int show, int format);

// return pointer to current frame, also set frame format, width, height and buffer width
void* pmp_get_frame(int* format, int* width, int* height, int* vbw );

// Stop the playback and free all resources
void pmp_stop();

// Check if playback is still running
int pmp_isplaying();

static void CleanUp()
{

}

void MoviePlayInit(movie_ops* ops)
{
	
    ops->load  =  PMP_Load;
    ops->play  =  PMP_Play;
    ops->stop  =  PMP_Stop;
    ops->eos   =  PMP_Eos;
}

void MoviePlayFini(void)
{
	CleanUp();

}

#ifdef WIN32
#include <stdio.h>
#include <SDL.h>
int  PMP_Load (const char *name)
{
	return 1;
}
int  PMP_Play (void)
{
	printf("NOW Play The Movie X:D\n");
	SDL_Delay(1000);
	return 1;
}
int  PMP_Stop (void)
{
	return 1;
}

int  PMP_Eos  (void)
{
	return 1;
}

#else
/*
#define SCE_MPEG_AVC_FORMAT_DEFAULT -1
#define SCE_MPEG_AVC_FORMAT_5650 0
#define SCE_MPEG_AVC_FORMAT_5551 1
#define SCE_MPEG_AVC_FORMAT_4444 2
#define SCE_MPEG_AVC_FORMAT_8888 3
*/
#include "pmp.h"
#include "nge_io_file.h"

int flags = 1;
static char filename[256]; 

int  PMP_Load (const char *name)
{
	//check file;
	pmp_init();
	int handle  = io_fopen(name,IO_RDONLY);
	if(handle <= 0){
		flags = 0;
		return 0;
	}
	else{
		io_fclose(handle);
		strncpy(filename,name,256);
		return 1;
	}
   	
}
int  PMP_Play (void)
{
	if(flags)
		pmp_play(filename, 1,3);
	return 1;
}
int  PMP_Stop (void)
{
	pmp_stop();
	return 1;
}

int  PMP_Eos  (void)
{
	return pmp_isplaying()==1?0:1;
}


#endif