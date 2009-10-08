#include "libnge2.h"


static int inited = 0;

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
	if(inited==0){	
		#ifdef WIN32
		screen = GetScreenContext();
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
		#elif defined IPHONEOS
			// iphone
		// iphone
		screen = GetScreenContext();
		if (SDL_Init(SDL_INIT_VIDEO) < 0) {
			printf("Could not initialize SDL\n");
		}
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
		SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 1);
		int windowID =
		SDL_CreateWindow(NULL, 0, 0, screen->width, screen->height,
                         SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
		
		if (windowID == 0) {
			printf("Could not initialize Window\n");
		}
		if (SDL_CreateRenderer(windowID, -1, 0) != 0) {
			printf("Could not create renderer\n");
		    }		
		#endif
		#if defined WIN32 || defined IPHONEOS
			// empty
		#else
			if(flags&INIT_AUDIO)
				CoolAudioDefaultInit();
		#endif		
		InitGrahics();
		inited = 1;
	}
}
void NGE_Quit()
{
	if(inited){
		FiniGrahics();
		FiniInput();
		
		#if defined WIN32 || defined IPHONEOS
			// empty
		#else
			CoolAudioDefaultFini();
		#endif
			
		#ifdef WIN32
			SDL_Quit();
			#ifdef MMGR
				m_dumpMemoryReport();
			#endif
		#elif defined IPHONEOS
			SDL_Quit();
		#endif
	}
}



