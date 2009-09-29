#include "play_ogg.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * this file is a template for the lib,the psp ogg play will
 * implement soon!
 */


static int  OGG_Load (const char *);
static int  OGG_Play (void);
static void OGG_Pause(void);
static int  OGG_Stop (void);
static void OGG_Resume(void);
static void OGG_Seek (int);
static int  OGG_Time (void);
static int  OGG_Eos  (void);
static int  OGG_Volume(int);


static void CleanUp()
{
	
}


void OGGPlayInit(music_ops* ops)
{
	
    ops->load  =  OGG_Load;
    ops->play  =  OGG_Play;
    ops->pause =  OGG_Pause;
    ops->stop  =  OGG_Stop;
    ops->resume=  OGG_Resume;
    ops->time  =  OGG_Time;
    ops->seek  =  OGG_Seek;
    ops->eos   =  OGG_Eos;
    ops->volume = OGG_Volume;
    strncpy(ops->extension,"ogg",4);

}

void OGGPlayFini()
{
	CleanUp();
}

int  OGG_Volume(int volume)
{
	return volume;
}



int  OGG_Load (const char *name)
{
	return 1;
}


int  OGG_Play (void)
{
	return 1;
}
void OGG_Pause(void)
{
	
}
int  OGG_Stop (void)
{
	return 1;
}
void OGG_Resume (void)
{
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
	return 1;
}
