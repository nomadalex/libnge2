#include <stdio.h>
#include <stdlib.h>
#include "audio_interface.h"

#if defined(WIN32) || defined(__linux__)
#include "SDL/SDL.h"
#include "SDL/SDL_mixer.h"

//for Windows and linux
//default init for windows and linux,load SDL mixer
void CoolAudioDefaultInit()
{
  int rate = MIX_DEFAULT_FREQUENCY;
  Uint16 format = MIX_DEFAULT_FORMAT;
  int channels = MIX_DEFAULT_CHANNELS;

  /* Initialize the SDL and SDL_mixer library */
  if ( SDL_Init(SDL_INIT_AUDIO) < 0 && Mix_Init(MIX_INIT_MP3|MIX_INIT_OGG) != (MIX_INIT_MP3|MIX_INIT_OGG) ) {
	fprintf(stderr, "Couldn't initialize SDL: %s\n",SDL_GetError());
	exit(1);
  }

  /* Open the audio device */
  if (Mix_OpenAudio(rate, format, channels, 4096) < 0) {
	fprintf(stderr, "Couldn't open audio: %s\n", Mix_GetError());
	exit(1);
  } else {
	Mix_QuerySpec(&rate, &format, &channels);
	// printf("Opened audio at %d Hz %d bit %s (%s), %d bytes audio buffer\n", rate,
	//   (format&0xFF),
	//   (channels > 2) ? "surround" : (channels > 1) ? "stereo" : "mono",
	//   (format&0x1000) ? "BE" : "LE",
	//   bufsize );
  }
  /* Set the music volume */
  Mix_VolumeMusic(MIX_MAX_VOLUME);

  /* Set the external music player, if any */
  // Mix_SetMusicCMD(getenv("MUSIC_CMD"));
}

void CoolAudioDefaultFini()
{
  Mix_CloseAudio();
  Mix_Quit();
}

#elif defined IPHONEOS
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
