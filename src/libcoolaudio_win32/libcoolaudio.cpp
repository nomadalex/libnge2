#include "audio_interface.h"
#include "win32_audio_builder.h"

CTopAudioBuilder* pBuilder = NULL;

void CoolAudioDefaultInit()
{
	if(pBuilder == NULL){
		pBuilder = new CTopAudioBuilder();
	}
}

void CoolAudioDefaultFini()
{
	if(pBuilder != NULL){
		delete pBuilder;
		pBuilder = NULL;
	}
}

