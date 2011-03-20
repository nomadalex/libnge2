#include "libnge2.h"

static int initFlags = 0;

void NGE_SetScreenContext(const char* winname,int screen_width,int screen_height,int screen_bpp,int screen_full)
{
	screen_context_p screen = GetScreenContext();
	if(winname!=NULL)
		strncpy(screen->name,winname,256);
	screen->width = screen_width;
	screen->height = screen_height;
	screen->bpp = screen_bpp;
	screen->fullscreen = screen_full;
}

void NGE_Init(int flags)
{
	screen_context_p screen = NULL;
	int screen_flag = 0;
	if(initFlags==0){
		screen = GetScreenContext();
#if defined(WIN32) || defined(__linux__)
	    /* screen_flag = SDL_OPENGL|SDL_HWSURFACE|SDL_DOUBLEBUF; */
		screen_flag = SDL_OPENGL;
		flags |= SDL_INIT_JOYSTICK;
			if ( SDL_Init(SDL_INIT_VIDEO) < 0 )
   				exit(1);
			SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8);
			SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
			SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
			SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1);
			SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 1);
			SDL_WM_SetCaption(screen->name,NULL);
			if(screen->fullscreen != 0)
				screen_flag |= SDL_FULLSCREEN;
			SDL_SetVideoMode( screen->width, screen->height, screen->bpp,screen_flag);
#endif

//#ifndef IPHONEOS // psp and linux and win32 and iphone(on iphone, it is a void function.)
		if(flags&INIT_AUDIO)
		  CoolAudioDefaultInit();
//#endif
#ifndef IPHONEOS
		if(flags&INIT_VIDEO)
			InitGrahics();
#endif
		initFlags = flags;
	}
}

void NGE_Quit()
{
	if(initFlags){
#ifndef IPHONEOS
		FiniInput();
		if(initFlags&INIT_VIDEO)
			FiniGrahics();
#endif
		if(initFlags&INIT_AUDIO)
			CoolAudioDefaultFini();

#if defined WIN32 || defined __linux__// linux and win32
		SDL_Quit();
#endif

#ifdef MMGR
		m_dumpMemoryReport();
#endif
	}
}
