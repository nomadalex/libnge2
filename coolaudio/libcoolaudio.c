#include <stdio.h>
#include <stdlib.h>
#include "audio_interface.h"

#if defined IPHONEOS
void CoolAudioDefaultInit() {}
void CoolAudioDefaultFini() {}

#elif defined _PSP
#include <pspsdk.h>
#include <pspkernel.h>
#include <pspaudiocodec.h>
#include <pspmpeg.h>
//for PSP
//default init for psp ,load prxs
void CoolAudioDefaultInit()
{
#ifdef PSP_FW_VERSION_1XX
	int result = pspSdkLoadStartModule("flash0:/kd/me_for_vsh.prx", PSP_MEMORY_PARTITION_KERNEL);
		result = pspSdkLoadStartModule("flash0:/kd/videocodec.prx", PSP_MEMORY_PARTITION_KERNEL);
		result = pspSdkLoadStartModule("flash0:/kd/audiocodec.prx", PSP_MEMORY_PARTITION_KERNEL);
		result = pspSdkLoadStartModule("flash0:/kd/mpegbase.prx", PSP_MEMORY_PARTITION_KERNEL);
		result = pspSdkLoadStartModule("flash0:/kd/mpeg_vsh.prx", PSP_MEMORY_PARTITION_USER);
		pspSdkFixupImports(result);
#else
	int result = sceUtilityLoadAvModule(0);
	result = sceUtilityLoadAvModule(3);
#endif
		sceMpegInit();
}

void CoolAudioDefaultFini()
{

}
#endif
