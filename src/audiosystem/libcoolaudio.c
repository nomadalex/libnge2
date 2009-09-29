#include "interface.h"

#ifdef WIN32
#include <stdio.h>
#include <stdlib.h>
#include "SDL.h"
//for Windows
//default init for windows ,load SDL mixer
void CoolAudioDefaultInit()
{
	/* Initialize the SDL library */
	if ( SDL_Init(SDL_INIT_AUDIO) < 0 ) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n",SDL_GetError());
		exit(0);
	}
}

void CoolAudioDefaultFini()
{
	
}

#else
#include <pspsdk.h>
#include <pspkernel.h>
#include <stdio.h>
#include <stdlib.h>
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
