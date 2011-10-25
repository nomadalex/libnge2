#include <stdio.h>
#include "audio_codec.h"
#include "codecs/mp3_codec.h"

AudioCodec* AudioCodec::create_audio_codec(CodecId id)
	{
		AudioCodec* codec = NULL;
		switch(id){
			case MP3:
				codec =  new Mp3Codec;
				break;
			case ATRAC3:
				break;
			case AAC:
				break;
			case TTA:
				break;
			case WMA:
				break;
			default:
				break;
		}
		return 	codec;
}
