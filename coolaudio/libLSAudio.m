/*
 *  libLSAudio.c
 *  LSAudio
 *
 *  Created by jock li on 8/19/09.
 *  Copyright 2009 topoc. All rights reserved.
 *
 */

#include "LSAudioDeleCall.h"

#pragma mark libLSAudio
#pragma mark -

#pragma mark Private units

#define _LIBLSAUDIO_TRUE 1
#define _LIBLSAUDIO_FALSE 0

static NSString* GetFileResoucePath(const char* cfilename)
{
	char filename[256] = {0};
	sprintf(filename,"@%s",cfilename);
	NSString* namestr = [[NSString alloc] initWithBytes:filename length:strlen(filename) encoding:NSASCIIStringEncoding];	
	NSRange r = [namestr rangeOfString:@"." options:NSBackwardsSearch]; // find ext
	NSString *path = @"/", *ext = @"";
	
	if(r.length!=0) // found ext
	{
		[ext release];
		r.location++;
		r.length = namestr.length - r.location;
		ext = [namestr substringWithRange:r];
	}
	
	NSRange r2 = [namestr rangeOfString:@"/" options:NSBackwardsSearch]; // find name
	if(r2.length!=0) // found path
	{
		[path release];
		r2.length = r2.location;
		r2.location = 0;
		path = [namestr substringWithRange:r2];	
	}
	
	NSRange r3 = {r2.length+1, namestr.length - r2.length - 2 - r.length};
	namestr = [namestr substringWithRange:r3];
	
	return [[NSBundle mainBundle] pathForResource:namestr ofType:ext inDirectory:path]; 
}

static int core_audio_load(struct audio_play* This,const char* filename)
{
	core_audio_play_p PThis = (core_audio_play_p)This;
	NSError *relErr = nil;
	
	NSString *soundFilePath = GetFileResoucePath(filename);
	NSURL *fileURL = [[NSURL alloc] initFileURLWithPath: soundFilePath];
	
	PThis->player = [[AVAudioPlayer alloc] initWithContentsOfURL: fileURL error: &relErr];
	
	PThis->audiocallback = (id)[LSAudioDeleCall new];
	[(LSAudioDeleCall *)PThis->audiocallback initWithCoreAudioPlay:PThis];
	PThis->player.delegate = (LSAudioDeleCall *)PThis->audiocallback;
	[fileURL release];
	
	if(relErr == nil)
	{
		[PThis->player prepareToPlay];
		return _LIBLSAUDIO_TRUE;
	}
	else
		return _LIBLSAUDIO_FALSE;
	
}

static int core_audio_play(struct audio_play* This,int times,int free_when_stop)
{	
	core_audio_play_p PThis = (core_audio_play_p)This;
	if(PThis->player == nil)
		return _LIBLSAUDIO_FALSE;
	//PThis->playcount = times;
	PThis->player.numberOfLoops = times - 1;
	PThis->autostop = free_when_stop;
	return [PThis->player play] ? _LIBLSAUDIO_TRUE : _LIBLSAUDIO_FALSE;
}

static int core_audio_playstop(struct audio_play* This)
{
	return core_audio_play(This, 1, 1);
}

static void core_audio_pause (struct audio_play* This)
{
	core_audio_play_p p = (core_audio_play_p)This;	
	[p->player pause];
}

static int core_audio_stop (struct audio_play* This)
{
	core_audio_play_p p = (core_audio_play_p)This;
	if(p->player == NULL)
		return _LIBLSAUDIO_FALSE;
	
	if (p->player.playing) {
		[p->player stop];
	}
	if(p->audiocallback != NULL){
		[p->audiocallback release];
	}
	[p->player release];
	p->player = NULL;
	return _LIBLSAUDIO_TRUE;
}

static void core_audio_resume (struct audio_play* This)
{
	core_audio_play_p p = (core_audio_play_p)This;
	[p->player play];
}

static int core_audio_volume (struct audio_play* This,int volume)
{
	core_audio_play_p p = (core_audio_play_p)This;	
	if(p->player==NULL)
		return _LIBLSAUDIO_FALSE;
	p->player.volume = (float)volume / 255.0f;
	return _LIBLSAUDIO_TRUE;
}

static void core_audio_seek (struct audio_play* This,int ms,int flag)
{
	core_audio_play_p p = (core_audio_play_p)This;
	if(p->player==NULL)
		return;
	
	int s = ms / 1000;
	if(flag == AUDIO_SEEK_END){
		s = p->player.duration - s;
	}
	else if(flag == AUDIO_SEEK_CUR)
	{
		s = p->player.currentTime + s;
	}
	if(s<0){
		p->player.currentTime = 0;
		return;
	}
	if(s>p->player.duration){
		p->player.currentTime = p->player.duration;
		return;
	}
	p->player.currentTime = s;
}

static void core_audio_rewind (struct audio_play* This)
{
	core_audio_seek(This, 0, AUDIO_SEEK_SET);
}

static int core_audio_iseos (struct audio_play* This)
{
	core_audio_play_p p = (core_audio_play_p)This;
	if (p->player == NULL)
		return _LIBLSAUDIO_TRUE;
	return p->player.playing ? _LIBLSAUDIO_FALSE : _LIBLSAUDIO_TRUE;
}

static int core_audio_ispaused (struct audio_play* This)
{
	core_audio_play_p p = (core_audio_play_p)This;
	if (p->player == NULL)
		return _LIBLSAUDIO_FALSE;
	
	return p->player.playing ? _LIBLSAUDIO_FALSE : _LIBLSAUDIO_TRUE;
}

static int core_audio_destroy (struct audio_play* This)
{
	core_audio_play_p p = (core_audio_play_p)This;
	if (p == NULL) return _LIBLSAUDIO_FALSE;
	core_audio_stop(This);
	free(p);
	p = NULL;
	return _LIBLSAUDIO_TRUE;
}
	

static audio_play_p core_audio_createplayer()
{
	core_audio_play_p p = (core_audio_play_p)malloc(sizeof(core_audio_play_t));	
	memset(p, 0, sizeof(core_audio_play_t));
	
	p->load = core_audio_load;
	p->load_fp = NULL;
	p->load_buf = NULL;
	p->play = core_audio_play;
	p->playstop = core_audio_playstop;
	p->pause = core_audio_pause;
	p->stop = core_audio_stop;
	p->resume = core_audio_resume;
	p->volume = core_audio_volume;
	p->rewind = core_audio_rewind;
	p->seek = core_audio_seek;
	p->iseof = core_audio_iseos;
	p->destroy = core_audio_destroy;
	
	return (audio_play_p)p;
}

#pragma mark Public Interface Handle
audio_play_p CreateMp3Player()
{
	return core_audio_createplayer();
}

audio_play_p CreateWavPlayer()
{
	return core_audio_createplayer();
}

audio_play_p CreateOggPlayer()
{
	return core_audio_createplayer();
}

#pragma mark -
#pragma mark Authoer Test (no using)

/*
 
void LoadSound(const char* filename)
{	
	
	NSString *namestr = [[NSString alloc] initWithBytes:filename length:strlen(filename) encoding:NSASCIIStringEncoding];
	NSLog(@"LoadSound filename=%@\n", namestr);
	
	NSRange r = [namestr rangeOfString:@"." options:NSBackwardsSearch]; // find ext
	NSString *path = @"/", *ext = @"";
	
	if(r.length!=0) // found ext
	{
		[ext release];
		r.location++;
		r.length = namestr.length - r.location;
		ext = [namestr substringWithRange:r];
		NSLog(@"LoadSound found ext is %@\n", ext);
	}
	
	NSRange r2 = [namestr rangeOfString:@"/" options:NSBackwardsSearch]; // find name
	if(r2.length!=0) // found path
	{
		[path release];
		r2.length = r2.location;
		r2.location = 0;
		path = [namestr substringWithRange:r2];
		NSLog(@"LoadSound found path is %@\n", path);		
	}
	
	NSRange r3 = {r2.length+1, namestr.length - r2.length - 2 - r.length};
	namestr = [namestr substringWithRange:r3];
	NSLog(@"LoadSound found name is %@\n", namestr);	
	
	SystemSoundID pmph; 
	id sndpath = [[NSBundle mainBundle] pathForResource:namestr ofType:ext inDirectory:path]; 
	CFURLRef baseURL = (CFURLRef) [[NSURL alloc] initFileURLWithPath:sndpath]; 
	AudioServicesCreateSystemSoundID (baseURL, &pmph); 
	AudioServicesPlaySystemSound(pmph); 
}
 */

