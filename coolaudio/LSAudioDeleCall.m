//
//  LSAudioDeleCall.m
//  LSAudio
//
//  Created by jock li on 8/21/09.
//  Copyright 2009 topoc. All rights reserved.
//

#import "LSAudioDeleCall.h"


@implementation LSAudioDeleCall

@synthesize mcoreaudio;

- (void)audioPlayerDidFinishPlaying:(AVAudioPlayer *)player successfully:(BOOL)flag
{
	if(mcoreaudio == NULL || mcoreaudio->player == NULL)
		return;
	if (flag) {
		// REPLAY IF NEED
		if(mcoreaudio->playcount>1)
		{
			mcoreaudio->play((audio_play_p)mcoreaudio, mcoreaudio->playcount-1, mcoreaudio->autostop);
		}
		else if(mcoreaudio->autostop)
		{
			mcoreaudio->stop((audio_play_p)mcoreaudio);
		}
	}
	else {
		// CALL STOP
		mcoreaudio->stop((audio_play_p)mcoreaudio);
	}

}

- (void)initWithCoreAudioPlay:(core_audio_play_p) pcoreaudio
{
	self.mcoreaudio = pcoreaudio;
}
@end
