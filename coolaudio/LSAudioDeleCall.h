//
//  LSAudioDeleCall.h
//  LSAudio
//
//  Created by jock li on 8/21/09.
//  Copyright 2009 topoc. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>

#include "audio_interface.h"

typedef struct core_audio_play_tag{
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
	// 专用内部参数
	AVAudioPlayer *player;
	int		      playcount;
	int           autostop;
	id			  audiocallback;
}core_audio_play_t,*core_audio_play_p;

@interface LSAudioDeleCall : NSObject<AVAudioPlayerDelegate> {
	core_audio_play_p mcoreaudio;
}

@property (readwrite) core_audio_play_p mcoreaudio;


- (void)initWithCoreAudioPlay:(core_audio_play_p) pcoreaudio;

- (void)audioPlayerDidFinishPlaying:(AVAudioPlayer *)player successfully:(BOOL)flag;

@end

