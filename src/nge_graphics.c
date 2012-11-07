/***************************************************************************
 *            nge_graphics.c
 *
 *  2011/03/27 09:29:04
 *  Copyright  2011  Kun Wang <ifreedom.cn@gmail.com>
 ****************************************************************************/
/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include "nge_platform.h"
#include "nge_debug_log.h"
#include "nge_timer.h"
#include "nge_misc.h"
#include "nge_tex_cache.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nge_graphics.h"

#if defined NGE_IPHONE || defined NGE_ANDROID
#define NGE_GLES
#ifdef NGE_IPHONE
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#include <OpenGLES/ES2/glext.h>
#include <OpenGLES/ES2/glext.h>

//this 2 functions implement in ***ViewController.m
extern void NGE_SetFramebuffer();
extern void NGE_PresentFramebuffer();
#else
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif

#define glOrtho glOrthof
#elif defined NGE_WIN || defined NGE_LINUX
#include <GL/glew.h>
#include <GL/gl.h>

#if defined NGE_LINUX
#include <X11/Xlib.h>
#include <GL/glx.h>

#else
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#endif

static float m_sintable[360];
static float m_costable[360];
#define RAD2DEG		57.29577951f
#define DEG2RAD		0.017453293f
#define SINF(a)  (m_sintable[a%360])
#define COSF(a)  (m_costable[a%360])

//fps count
static uint32_t m_frame = 0;
static uint32_t m_t0 = 0;
static int fps_last_ticks = 0;
static char inited = 0;
static int cacheid = 0;
static uint8_t tex_ret = 0;

#define MAX_TEX_CACHE_SIZE 256
GLuint m_texcache[MAX_TEX_CACHE_SIZE];
GLuint fbo = 0;
// nge_screen *************************
static screen_context_t nge_screen = {
	NULL,
	SCREEN_WIDTH,
	SCREEN_HEIGHT,
	SCREEN_BPP,
	0,

	SCREEN_WIDTH,
	SCREEN_HEIGHT,
	1.0f,
	1.0f
};

typedef struct
{
	GLfloat x,y;
}Vectice2D_t;
#define VECT_2D_X(v) (v.x)
#define VECT_2D_Y(v) (v.y)
#define VECT_2D_PRINT(v)						\
	printf(#v "x %f, y %f\n", v.x, v.y)
#define VECT_2D_SET(vec, X, Y) \
	vec.x = X;				   \
	vec.y = Y
#define VECT_2D_COPY(v1, v2)   \
	v1.x = v2.x;			   \
	v1.y = v2.y

typedef struct
{
	GLubyte r,g,b,a;
}Color_t;
#define COLOR_T_R(c) (c.r)
#define COLOR_T_G(c) (c.g)
#define COLOR_T_B(c) (c.b)
#define COLOR_T_A(c) (c.a)
#define COLOR_T_SET(c, R, G, B, A) \
	c.r = R;					   \
	c.g = G;					   \
	c.b = B;					   \
	c.a = A
#define COLOR_T_COPY(c1, c2) \
	c1.r = c2.r;			 \
	c1.g = c2.g;			 \
	c1.b = c2.b;			 \
	c1.a = c2.a

typedef struct
{
	GLfloat u,v;
}TexCoord_t;
#define TEX_C_T_U(t) (t.u)
#define TEX_C_T_V(t) (t.v)
#define TEX_C_T_PRINT(t)						\
	printf(#t "u %f, v %f\n", t.u, t.v)
#define TEX_C_T_SET(t, U, V) \
	t.u = U;				 \
	t.v = V
#define TEX_C_T_COPY(t1, t2) \
	t1.u = t2.u;			 \
	t1.v = t2.v

static Vectice2D_t *gl_vectices;
static Color_t *gl_colors;
static TexCoord_t *gl_tex_uvs;

static uint32_t max_vectices=0,max_colors=0,max_tex_uvs=0;
#define GL_ARRAY_CHECK_V(size)											\
	if(((uint32_t)size) > max_vectices){									\
		max_vectices = size;											\
		SAFE_FREE(gl_vectices);											\
		gl_vectices = (Vectice2D_t*)malloc(size*sizeof(Vectice2D_t));	\
		glVertexPointer(2,GL_FLOAT,sizeof(Vectice2D_t),gl_vectices);	\
	}

#define GL_ARRAY_CHECK_C(size)											\
	if(((uint32_t)size) > max_colors)										\
	{																	\
		max_colors = size;												\
		SAFE_FREE(gl_colors);											\
		gl_colors = (Color_t*)malloc(size*sizeof(Color_t));				\
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Color_t), gl_colors); \
	}

#define GL_ARRAY_CHECK_T(size)											\
	if(((uint32_t)size) > max_tex_uvs)									\
	{																	\
		max_tex_uvs = size;												\
		SAFE_FREE(gl_tex_uvs);											\
		gl_tex_uvs = (TexCoord_t*)malloc(size*sizeof(TexCoord_t));		\
		glTexCoordPointer(2,GL_FLOAT,sizeof(TexCoord_t),gl_tex_uvs);	\
	}

#define GL_ARRAY_EN(what)						\
	glEnableClientState(GL_##what##_ARRAY);

#define GL_ARRAY_DIS(what)						\
	glDisableClientState(GL_##what##_ARRAY);

static inline void GetRGBA(int color,int dtype,uint8_t* r,uint8_t* g,uint8_t* b,uint8_t* a)
{
	switch(dtype)
	{
	case DISPLAY_PIXEL_FORMAT_5551:
		GET_RGBA_5551(color,(*r),(*g),(*b),(*a));
		break;
	case DISPLAY_PIXEL_FORMAT_4444:
		GET_RGBA_4444(color,(*r),(*g),(*b),(*a));
		break;
	case DISPLAY_PIXEL_FORMAT_565:
		GET_RGBA_565(color,(*r),(*g),(*b),(*a));
		break;
	case DISPLAY_PIXEL_FORMAT_8888:
		GET_RGBA_8888(color,(*r),(*g),(*b),(*a));
		break;
	}
}

char* NGE_GetVersion()
{
	static char version[] = {
#ifdef NGE_IPHONE
		"nge2 iphone opengles driver v1.0"
#elif defined NGE_ANDROID
		"nge2 android opengles driver v1.0"
#else
		"nge2 driver v2.0"
#endif
	};
	return version;
}

static Color_t screen_c = {0, 0, 0, 0};

uint32_t SetScreenColor(uint8_t r,uint8_t g,uint8_t b,uint8_t a)
{
	uint32_t u_lastcolor;
	u_lastcolor = MAKE_RGBA_8888(((int)(COLOR_T_R(screen_c)*255)),((int)(COLOR_T_G(screen_c)*255)),((int)(COLOR_T_B(screen_c)*255)),((int)(COLOR_T_A(screen_c)*255)));
	COLOR_T_R(screen_c) = r/255.0;
	COLOR_T_G(screen_c) = g/255.0;
	COLOR_T_B(screen_c) = b/255.0;
	COLOR_T_A(screen_c) = a/255.0;
	return u_lastcolor;
}

screen_context_p GetScreenContext()
{
	return &nge_screen;
}

void SetTexBlend(int src_blend, int des_blend)
{
	glBlendFunc(src_blend,des_blend);
}

void ResetTexBlend()
{
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
}

void SetTexBlendEquation(int color_equation, int alpha_equation)
{
	glBlendEquationSeparate(color_equation, alpha_equation);
}

void ResetTexBlendEquation()
{
	int bindingFbo;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING_EXT, &bindingFbo);
	if(bindingFbo == 0)
		glBlendEquation(GL_FUNC_ADD);
	else
		glBlendEquationSeparate(GL_FUNC_ADD, 0x8008/*GL_MAX*/);
}

void SetClip(int x,int y,int w,int h)
{
	float rate_w_ori = 1.0f*nge_screen.width/nge_screen.ori_width; //1/nge_screen.rate_w;
	float rate_h_ori = 1.0f*nge_screen.height/nge_screen.ori_height; //1/nge_screen.rate_h;
	glScissor(floor(x*rate_w_ori),floor(nge_screen.height-rate_h_ori*y-rate_h_ori*h),
		ceil(w*rate_w_ori),ceil(h*rate_h_ori));
}

void ResetClip()
{
	SetClip(0, 0, nge_screen.ori_width,nge_screen.ori_height);
}



#if defined NGE_LINUX
Display *g_dpy;
Window g_win;
GLXContext g_ctx;
static void
makeWindow(const char *name, int x, int y, int width, int height)
{
	int attrib[] = { GLX_RGBA,
					 GLX_RED_SIZE, 1,
					 GLX_GREEN_SIZE, 1,
					 GLX_BLUE_SIZE, 1,
					 GLX_DOUBLEBUFFER,
					 None };
	int scrnum;
	XSetWindowAttributes attr;
	unsigned long mask;
	Window root;
	XVisualInfo *visinfo;

	scrnum = DefaultScreen( g_dpy );
	root = RootWindow( g_dpy, scrnum );

	visinfo = glXChooseVisual( g_dpy, scrnum, attrib );
	if (!visinfo) {
		printf("Error: couldn't get an RGB, Double-buffered visual\n");
		exit(1);
	}

	/* window attributes */
	attr.background_pixel = 0;
	attr.border_pixel = 0;
	attr.colormap = XCreateColormap( g_dpy, root, visinfo->visual, AllocNone);
	attr.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask;
	mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

	g_win = XCreateWindow( g_dpy, root, 0, 0, width, height,
						 0, visinfo->depth, InputOutput,
						 visinfo->visual, mask, &attr );

	/* set hints and properties */
	{
		XSizeHints sizehints;
		sizehints.x = x;
		sizehints.y = y;
		sizehints.width  = width;
		sizehints.height = height;
		sizehints.flags = USSize | USPosition;
		XSetNormalHints(g_dpy, g_win, &sizehints);
		XSetStandardProperties(g_dpy, g_win, name, name,
							   None, (char **)NULL, 0, &sizehints);
	}

	g_ctx = glXCreateContext( g_dpy, visinfo, NULL, True );
	if (!g_ctx) {
		printf("Error: glXCreateContext failed\n");
		exit(1);
	}

	XFree(visinfo);
}
#endif

void ResetGraphicsCache(void)
{
	int i;	
	tex_cache_fini();
	tex_cache_init(MAX_TEX_CACHE_SIZE);
	glDeleteTextures(MAX_TEX_CACHE_SIZE,m_texcache);
	glGenTextures( MAX_TEX_CACHE_SIZE, &m_texcache[0] );
	for(i=0;i<MAX_TEX_CACHE_SIZE;i++){
		tex_cache_add(i,m_texcache[i]);
		glBindTexture(GL_TEXTURE_2D, m_texcache[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
}

void nge_graphics_reset(void)
{
	int i;
	// reset for fps------------------
	m_frame = 0;
	m_t0 = 0;
	fps_last_ticks = nge_get_tick();
	if(inited == 0){
		inited = 1;
		tex_cache_init(MAX_TEX_CACHE_SIZE);
		glGenTextures( MAX_TEX_CACHE_SIZE, &m_texcache[0] );
		for(i=0;i<MAX_TEX_CACHE_SIZE;i++){
				tex_cache_add(i,m_texcache[i]);
				glBindTexture(GL_TEXTURE_2D, m_texcache[i]);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}
        #if defined NGE_IPHONE || defined NGE_ANDROID
            glGenFramebuffers(1, &fbo);
        #endif

	}
	else{
		tex_cache_fini();
		tex_cache_init(MAX_TEX_CACHE_SIZE);
		glDeleteTextures(MAX_TEX_CACHE_SIZE,m_texcache);
		glGenTextures( MAX_TEX_CACHE_SIZE, &m_texcache[0] );
		for(i=0;i<MAX_TEX_CACHE_SIZE;i++){
				tex_cache_add(i,m_texcache[i]);
				glBindTexture(GL_TEXTURE_2D, m_texcache[i]);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}
		nge_print("cleared.\n");
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,nge_screen.ori_width,nge_screen.ori_height,0, -1, 1);
	glMatrixMode(GL_MODELVIEW);

	glEnable(GL_SCISSOR_TEST);
	ResetClip();

	glEnable( GL_TEXTURE_2D );
	glDisable( GL_DEPTH_TEST );
	glShadeModel( GL_SMOOTH );
	glEnable(GL_BLEND);
	ResetTexBlend();

	GL_ARRAY_EN(VERTEX);
	glVertexPointer(2,GL_FLOAT,sizeof(Vectice2D_t),gl_vectices);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Color_t), gl_colors);
	glTexCoordPointer(2,GL_FLOAT,sizeof(TexCoord_t),gl_tex_uvs);
}

#ifdef NGE_WIN
// decl from nge_input.c
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void EnableOpenGL(HWND hWnd, HDC * hDC, HGLRC * hRC)
{
	PIXELFORMATDESCRIPTOR pfd;
	int format;

	/* get the device context (DC) */
	*hDC = GetDC( hWnd );

	/* set the pixel format for the DC */
	ZeroMemory( &pfd, sizeof( pfd ) );
	pfd.nSize = sizeof( pfd );
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;
	format = ChoosePixelFormat( *hDC, &pfd );
	SetPixelFormat( *hDC, format, &pfd );

	/* create and enable the render context (RC) */
	*hRC = wglCreateContext( *hDC );
	wglMakeCurrent( *hDC, *hRC );
#if defined NGE_WIN || defined NGE_LINUX
    	glewInit();
	glGenFramebuffersEXT(1, &fbo);
#endif
}

void DisableOpenGL(HWND hWnd, HDC hDC, HGLRC hRC)
{
	wglMakeCurrent( NULL, NULL );
	wglDeleteContext( hRC );
	ReleaseDC( hWnd, hDC );
#if defined NGE_WIN || defined NGE_LINUX
	glDeleteFramebuffersEXT(1, &fbo);
#endif
}

static HWND hWnd;
static HDC hDC;
static HGLRC hRC;

void makeWindow(const char *name, int x, int y, int width, int height)
{
	HINSTANCE hInstance;
	WNDCLASS wc;
	RECT rect;

	hInstance = GetModuleHandle(NULL);
	// register window class
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon( NULL, IDI_APPLICATION );
	wc.hCursor = LoadCursor( NULL, IDC_ARROW );
	wc.hbrBackground = (HBRUSH)GetStockObject( BLACK_BRUSH );
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "Nge2App";
	RegisterClass( &wc );

	// create main window
	hWnd = CreateWindow(
		"Nge2App", name,
		WS_CAPTION | WS_POPUPWINDOW | WS_VISIBLE,
		x, y, width, height,
		NULL, NULL, hInstance, NULL );

	GetClientRect(hWnd, &rect);
	width = width + width - rect.right;
	height = height + height - rect.bottom;
	SetWindowPos(hWnd, HWND_TOP, 0, 0, width, height, SWP_NOMOVE);
}
#endif

void InitGraphics()
{
	int i = 0;

	if (nge_screen.name == NULL) {
		nge_screen.name = malloc(8);
		strncpy(nge_screen.name, "NGE2", 8);
	}

#if defined NGE_WIN
	makeWindow(nge_screen.name, 0, 0, nge_screen.width, nge_screen.height);
	// enable OpenGL for the window
	EnableOpenGL( hWnd, &hDC, &hRC );
#elif defined NGE_LINUX
	g_dpy = XOpenDisplay(NULL);
	if (!g_dpy) {
		nge_error("Couldn't open default display.\n");
		return;
	}
	makeWindow(nge_screen.name, 0, 0, nge_screen.width, nge_screen.height);
	XMapWindow(g_dpy, g_win);
	glXMakeCurrent(g_dpy, g_win, g_ctx);
#endif

	//-------------------------------

	for (i=0;i<360;i++)
	{
		m_sintable[i] = sin(i*DEG2RAD);
		m_costable[i] = cos(i*DEG2RAD);
	}

	
	nge_graphics_reset();

	nge_log("Init Graphics Ok\n");
}

void FiniGraphics()
{
	tex_cache_fini();
#ifndef NGE_ANDROID
	/* android will destory gl context by itself. */
	glDeleteTextures(MAX_TEX_CACHE_SIZE,m_texcache);
#endif
	SAFE_FREE(gl_vectices);
	gl_vectices = NULL;
	max_vectices = 0;
	SAFE_FREE(gl_colors);
	gl_colors = NULL;
	max_colors = 0;
	SAFE_FREE(gl_tex_uvs);
	gl_tex_uvs = NULL;
	max_tex_uvs = 0;

#if defined NGE_WIN
	// shutdown OpenGL
	DisableOpenGL( hWnd, hDC, hRC );

	// destroy the window explicitly
	DestroyWindow( hWnd );
#endif

#if defined NGE_LINUX
	glXDestroyContext(g_dpy, g_ctx);
	XDestroyWindow(g_dpy, g_win);
	XCloseDisplay(g_dpy);
#endif
    
#if defined NGE_IPHONE
    glDeleteFramebuffers(1, &fbo);
#endif
}

void ShowFps()
{
#ifndef NDEBUG
	int t;
	float seconds;
	m_frame++;
	t = nge_get_tick();
	if ( (t - m_t0) >= 1000) {
		seconds = (t - m_t0) / 1000.0;
		nge_log("%d frames in %g seconds = %g FPS\n", m_frame, seconds, m_frame / seconds);
		m_t0 = t;
		m_frame = 0;
	}
#endif
}

void LimitFps(uint32_t limit)
{
	int ticks = 0, sleep_ticks = 0;
	if(limit == 0)
		limit = 60;
	ticks = nge_get_tick();
	sleep_ticks = 1000/limit - (ticks - fps_last_ticks);
	fps_last_ticks = ticks;

	if( sleep_ticks > 0 )
		nge_sleep( sleep_ticks );
}

#define ROTATE_2D(angle, xcent, ycent)          \
	glTranslatef(xcent,ycent,0);                \
	glRotatef(angle,0,0,1);                     \
	glTranslatef(-(xcent),-(ycent),0)
void BeginScene(uint8_t clear)
{
#if defined NGE_IPHONE
    NGE_SetFramebuffer();
#endif
    if(clear == 1){
		glDisable(GL_SCISSOR_TEST);
		glClearColor( COLOR_T_R(screen_c), COLOR_T_G(screen_c), COLOR_T_B(screen_c), COLOR_T_A(screen_c) );
		glClear( GL_COLOR_BUFFER_BIT);
		glEnable(GL_SCISSOR_TEST);
	}
	glLoadIdentity();
}

void EndScene()
{
#if defined NGE_IPHONE
    NGE_PresentFramebuffer();
#elif defined NGE_LINUX
	glXSwapBuffers(g_dpy, g_win);
#elif defined NGE_WIN
	SwapBuffers( hDC );
#elif defined NGE_ANDROID
	glFlush();
#endif
}


static uint8_t r,g,b,a;
#define SET_COLOR(color, dtype)                 \
	GetRGBA(color,dtype,&r,&g,&b,&a);           \
	glColor4ub(r, g, b, a)

#if defined NGE_GLES
#define BEFORE_DRAW()
#define AFTER_DRAW()
#else
#define BEFORE_DRAW()                           \
	glPushAttrib(GL_CURRENT_BIT)

#define AFTER_DRAW()                            \
	glPopAttrib()
#endif

void DrawLine(float x1, float y1, float x2, float y2, int color,int dtype)
{
	glDisable(GL_TEXTURE_2D);
	BEFORE_DRAW();
	GL_ARRAY_CHECK_V(2);
	VECT_2D_SET(gl_vectices[0], x1, y1);
	VECT_2D_SET(gl_vectices[1], x2, y2);
	SET_COLOR(color, dtype);
	glDrawArrays(GL_LINES, 0, 2);
	AFTER_DRAW();
	glEnable(GL_TEXTURE_2D);
}

void DrawLineEx(pointf p1,pointf p2 ,int color,int dtype)
{
	DrawLine(p1.x, p1.y, p2.x, p2.y,color,dtype);
}

void DrawCircle(float x, float y, float radius, int color,int dtype)
{
	int i;
	glDisable(GL_TEXTURE_2D);
	BEFORE_DRAW();
	GL_ARRAY_CHECK_V(360);
	for(i=0; i<360;i++)
	{
		VECT_2D_SET(gl_vectices[i], x+radius*COSF(i), y+radius*SINF(i));
	}
	SET_COLOR(color, dtype);
	glDrawArrays(GL_LINE_LOOP, 0, 360);
	AFTER_DRAW();
	glEnable(GL_TEXTURE_2D);
}

void FillCircle(float x, float y, float radius, int color,int dtype)
{
	int i;
	glDisable(GL_TEXTURE_2D);
	BEFORE_DRAW();
	GL_ARRAY_CHECK_V(360);
	for(i=0; i<360;i++)
	{
		VECT_2D_SET(gl_vectices[i], x+radius*COSF(i), y+radius*SINF(i));
	}
	SET_COLOR(color, dtype);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 360);
	AFTER_DRAW();
	glEnable(GL_TEXTURE_2D);
}

void DrawEllipse(float x,float y ,float xradius,float yradius,int color,int dtype)
{
	int i;
	glDisable(GL_TEXTURE_2D);
	BEFORE_DRAW();
	GL_ARRAY_CHECK_V(360);
	for(i=0; i<360;i++)
	{
		VECT_2D_SET(gl_vectices[i], x+xradius*COSF(i), y+yradius*SINF(i));
	}
	SET_COLOR(color, dtype);
	glDrawArrays(GL_LINE_LOOP, 0, 360);
	AFTER_DRAW();
	glEnable(GL_TEXTURE_2D);
}

void FillEllipse(float x,float y ,float xradius,float yradius,int color,int dtype)
{
	int i;
	glDisable(GL_TEXTURE_2D);
	BEFORE_DRAW();
	GL_ARRAY_CHECK_V(360);
	for(i=0; i<360;i++)
	{
		VECT_2D_SET(gl_vectices[i], x+xradius*COSF(i), y+yradius*SINF(i));
	}
	SET_COLOR(color, dtype);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 360);
	AFTER_DRAW();
	glEnable(GL_TEXTURE_2D);
}

void PutPix(float x,float y ,int color,int dtype)
{
	glDisable(GL_TEXTURE_2D);
	BEFORE_DRAW();
	GL_ARRAY_CHECK_V(1);
	VECT_2D_SET(gl_vectices[0], x, y);
	SET_COLOR(color,dtype);
	glDrawArrays(GL_POINTS, 0, 1);
	AFTER_DRAW();
	glEnable(GL_TEXTURE_2D);
}

#define DRAW_POLYGON_IMP(mode, count)					\
	glDisable(GL_TEXTURE_2D);							\
	BEFORE_DRAW();                                      \
	GL_ARRAY_CHECK_V(count);							\
	{                                                   \
		int i;                                          \
		for(i=0; i<count;i++)                           \
		{                                               \
			VECT_2D_SET( gl_vectices[i], x[i], y[i] );	\
		}                                               \
		SET_COLOR(color,dtype);							\
		glDrawArrays(mode, 0, count);                   \
	}                                                   \
	AFTER_DRAW();										\
	glEnable(GL_TEXTURE_2D)

#define DRAW_POLYGON_IMP_COLOR(mode, count)				\
	glDisable(GL_TEXTURE_2D);							\
	BEFORE_DRAW();                                      \
	GL_ARRAY_EN(COLOR);									\
	GL_ARRAY_CHECK_V(count);							\
	GL_ARRAY_CHECK_C(count);							\
	{                                                   \
		int i;                                          \
		for(i=0; i<count;i++)                           \
		{                                               \
			VECT_2D_SET( gl_vectices[i], x[i], y[i] );	\
			GetRGBA(colors[i],dtype,&r,&g,&b,&a);		\
			COLOR_T_SET( gl_colors[i], r, g, b, a );	\
		}                                               \
		glDrawArrays(mode, 0, count);                   \
	}                                                   \
	GL_ARRAY_DIS(COLOR);								\
	AFTER_DRAW();										\
	glEnable(GL_TEXTURE_2D)

void DrawPolygon(float* x, float* y, int count, int color,int dtype)
{
	DRAW_POLYGON_IMP(GL_LINE_LOOP, count);
}

void FillPolygon(float* x, float* y, int count, int color,int dtype)
{
	DRAW_POLYGON_IMP(GL_TRIANGLE_FAN, count);
}

void FillPolygonGrad(float* x, float* y, int count, int* colors,int dtype)
{
	DRAW_POLYGON_IMP_COLOR(GL_TRIANGLE_FAN, count);
}

#define SET_RECT_ARRAY()						\
	float x[4], y[4];							\
	x[0] = dx;									\
	x[1] = dx;									\
	x[2] = dx+width;							\
	x[3] = dx+width;							\
	y[0] = dy;									\
	y[1] = dy+height;							\
	y[2] = dy+height;							\
	y[3] = dy

void DrawRect(float dx, float dy, float width, float height,int color,int dtype)
{
	SET_RECT_ARRAY();
	DRAW_POLYGON_IMP(GL_LINE_LOOP, 4);
}

void DrawRectEx(rectf rect,int color,int dtype)
{
	DrawRect(rect.top, rect.left, rect.right-rect.left, rect.bottom-rect.top,color,dtype);
}

void FillRect(float dx, float dy, float width, float height,int color,int dtype)
{
	SET_RECT_ARRAY();
	DRAW_POLYGON_IMP(GL_TRIANGLE_FAN, 4);
}

void FillRectEx(rectf rect,int color,int dtype)
{
	FillRect(rect.top, rect.left, rect.right-rect.left, rect.bottom-rect.top,color,dtype);
}

//顶点color顺序为逆时针方向0->3设置
// 0---------------------3
//  |                   |
//  |                   |
//	|                   |
// 1---------------------2
//
void FillRectGrad(float dx, float dy, float width, float height,int* colors,int dtype)
{
	SET_RECT_ARRAY();
	DRAW_POLYGON_IMP_COLOR(GL_TRIANGLE_FAN, 4);
}

inline void FillRectGradEx(rectf rect,int* colors,int dtype)
{
	FillRectGrad(rect.top, rect.left, rect.right-rect.left, rect.bottom-rect.top,colors,dtype);
}

/** 填充三角型(单色)
 *@param[in] v1 顶点坐标v1
 *@param[in] v2 顶点坐标v2
 *@param[in] v3 顶点坐标v3
 *@param[in] color 填充色
 *@param[in] dtype 显示模式,填充色要与之相对应
 *@return void ,无
 */
inline void FillTri(pointf v1,pointf v2,pointf v3 ,int color,int dtype)
{
	float x[3], y[3];
	x[0] = v1.x;
	x[1] = v2.x;
	x[2] = v3.x;
	y[0] = v1.y;
	y[1] = v2.y;
	y[2] = v3.y;
	FillPolygon(x, y, 3, color, dtype);
}

int PreLoadImage(image_p pimg)
{
	return 1;
}

static inline void TexImage2D(image_p pimg)
{
	uint32_t format = GL_RGBA;
	if(pimg->dtype == DISPLAY_PIXEL_FORMAT_565){
		format = GL_RGB;
	}
	glTexImage2D(GL_TEXTURE_2D, 0, format, pimg->texw, pimg->texh, 0, format, pimg->dtype, pimg->data);
}

#define BIND_AND_TEST_CACHE(tex)						\
	do {												\
		tex_ret = tex_cache_getid(tex->texid,&cacheid);	\
		glBindTexture(GL_TEXTURE_2D, cacheid);			\
		if(tex_ret == 0 ||tex->modified==1){			\
			TexImage2D(tex);							\
			if(tex->filter == FILTER_NEAREST){          \
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); \
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); \
			}                                                                      \
			else{                                                                  \
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  \
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);  \
			}                                                                      \
			tex->modified = 0;							\
		}												\
	}while(0)

#define SET_TEX_COORD(tex, sx, sy, sw, sh, topleft, bottomleft, bottomright, topright) \
	if(sw==0&&sh==0){													\
		if(sx==0&&sy==0){												\
			if(tex->w==tex->texw&&tex->h==tex->texh)					\
			{															\
				TEX_C_T_SET( gl_tex_uvs[topleft], 0, 0 );				\
				TEX_C_T_SET( gl_tex_uvs[bottomleft], 0, 1 );			\
				TEX_C_T_SET( gl_tex_uvs[bottomright], 1, 1 );			\
				TEX_C_T_SET( gl_tex_uvs[topright], 1, 0 );				\
			}else{														\
				TEX_C_T_SET( gl_tex_uvs[topleft], 0, 0 );				\
				TEX_C_T_SET( gl_tex_uvs[bottomleft], 0, (float)tex->h/tex->texh ); \
				TEX_C_T_SET( gl_tex_uvs[bottomright], (float)tex->w/tex->texw, (float)tex->h/tex->texh ); \
				TEX_C_T_SET( gl_tex_uvs[topright], (float)tex->w/tex->texw, 0 ); \
			}															\
		}else{															\
			TEX_C_T_SET( gl_tex_uvs[topleft], (float)sx/tex->texw, (float)sy/tex->texh ); \
			TEX_C_T_SET( gl_tex_uvs[bottomleft], (float)sx/tex->texw, (float)tex->h/tex->texh ); \
			TEX_C_T_SET( gl_tex_uvs[bottomright], (float)tex->w/tex->texw, (float)tex->h/tex->texh ); \
			TEX_C_T_SET( gl_tex_uvs[topright], (float)tex->w/tex->texw, (float)sy/tex->texh ); \
		}																\
	}else{																\
		if(sx==0&&sy==0){												\
			TEX_C_T_SET( gl_tex_uvs[topleft], 0, 0 );					\
			TEX_C_T_SET( gl_tex_uvs[bottomleft], 0, (float)sh/tex->texh ); \
			TEX_C_T_SET( gl_tex_uvs[bottomright], (float)sw/tex->texw, (float)sh/tex->texh ); \
			TEX_C_T_SET( gl_tex_uvs[topright], (float)sw/tex->texw, 0 ); \
		}else{															\
			TEX_C_T_SET( gl_tex_uvs[topleft], (float)sx/tex->texw, (float)sy/tex->texh ); \
			TEX_C_T_SET( gl_tex_uvs[bottomleft], (float)sx/tex->texw, (float)(sy+sh)/tex->texh ); \
			TEX_C_T_SET( gl_tex_uvs[bottomright], (float)(sx+sw)/tex->texw, (float)(sy+sh)/tex->texh ); \
			TEX_C_T_SET( gl_tex_uvs[topright], (float)(sx+sw)/tex->texw, (float)sy/tex->texh ); \
		}																\
	}

#define BEFORE_DRAW_IMAGE()						\
	BEFORE_DRAW();								\
	glPushMatrix();								\
	BIND_AND_TEST_CACHE(tex);					\
	GL_ARRAY_CHECK_V(4);						\
	GL_ARRAY_CHECK_T(4)

/* #ifndef NDEBUG */
#if 0
#define DEBUG_PRINT_DRAW_IMAGE					\
	TEX_C_T_PRINT(gl_tex_uvs[0]);				\
	TEX_C_T_PRINT(gl_tex_uvs[1]);				\
	TEX_C_T_PRINT(gl_tex_uvs[2]);				\
	TEX_C_T_PRINT(gl_tex_uvs[3]);				\
	VECT_2D_PRINT(gl_vectices[0]);				\
	VECT_2D_PRINT(gl_vectices[1]);				\
	VECT_2D_PRINT(gl_vectices[2]);				\
	VECT_2D_PRINT(gl_vectices[3]);
#else
#define DEBUG_PRINT_DRAW_IMAGE
#endif

#define AFTER_DRAW_IMAGE()						\
	GL_ARRAY_EN(TEXTURE_COORD);					\
	DEBUG_PRINT_DRAW_IMAGE						\
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);		\
	GL_ARRAY_DIS(TEXTURE_COORD);				\
	glPopMatrix();								\
	AFTER_DRAW()

#define SET_IMAGE_RECT_BY_TEX(tex, dx, dy)						\
 	VECT_2D_SET( gl_vectices[0], dx, dy );						\
	VECT_2D_SET( gl_vectices[1], dx, dy+tex->h );				\
	VECT_2D_SET( gl_vectices[2], dx+tex->w, dy+tex->h );	\
	VECT_2D_SET( gl_vectices[3], dx+tex->w, dy )

#define SET_IMAGE_RECT(dx, dy, dw, dh)				\
	VECT_2D_SET( gl_vectices[0], dx, dy );			\
	VECT_2D_SET( gl_vectices[1], dx, dy+dh );		\
	VECT_2D_SET( gl_vectices[2], dx+dw, dy+dh );	\
	VECT_2D_SET( gl_vectices[3], dx+dw, dy )

#define TRANSLATE_QUAD()								\
	TEX_C_T_SET( gl_tex_uvs[0], quad.v[0].u, quad.v[0].v );	\
	TEX_C_T_SET( gl_tex_uvs[1], quad.v[1].u, quad.v[1].v );	\
	TEX_C_T_SET( gl_tex_uvs[2], quad.v[2].u, quad.v[2].v );	\
	TEX_C_T_SET( gl_tex_uvs[3], quad.v[3].u, quad.v[3].v );	\
	VECT_2D_SET( gl_vectices[0], quad.v[0].x, quad.v[0].y );\
	VECT_2D_SET( gl_vectices[1], quad.v[1].x, quad.v[1].y );\
	VECT_2D_SET( gl_vectices[2], quad.v[2].x, quad.v[2].y );\
	VECT_2D_SET( gl_vectices[3], quad.v[3].x, quad.v[3].y );\
	GetRGBA(quad.v[0].color,tex->dtype,&r,&g,&b,&a);		\
	COLOR_T_SET( gl_colors[0], r, g, b, a );			\
	GetRGBA(quad.v[1].color,tex->dtype,&r,&g,&b,&a);		\
	COLOR_T_SET( gl_colors[1], r, g, b, a );			\
	GetRGBA(quad.v[2].color,tex->dtype,&r,&g,&b,&a);		\
	COLOR_T_SET( gl_colors[2], r, g, b, a );			\
	GetRGBA(quad.v[3].color,tex->dtype,&r,&g,&b,&a);		\
	COLOR_T_SET( gl_colors[3], r, g, b, a )			

#define BEFORE_DRAW_QUAD()								\
	image_p tex = quad.tex;								\
	BEFORE_DRAW_IMAGE();								\
	GL_ARRAY_CHECK_C(4)

#define AFTER_DRAW_QUAD()								\
	SET_COLOR(0xFFFFFFFF,tex->dtype);					\
	GL_ARRAY_EN(COLOR);									\
	GL_ARRAY_EN(TEXTURE_COORD);							\
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);				\
	GL_ARRAY_DIS(TEXTURE_COORD);						\
	GL_ARRAY_DIS(COLOR);								\
	glPopMatrix();										\
	AFTER_DRAW()

void ImageToScreen(image_p tex,float dx,float dy)
{
	BEFORE_DRAW_IMAGE();
	SET_TEX_COORD(tex, 0, 0, 0, 0, 0, 1, 2, 3);
    //SET_COLOR(tex->mask,tex->dtype);
	SET_IMAGE_RECT_BY_TEX(tex, dx, dy);
	AFTER_DRAW_IMAGE();
}

void DrawImage(image_p tex,float sx,float sy,float sw,float sh,float dx,float dy,float dw,float dh)
{
	BEFORE_DRAW_IMAGE();
	SET_TEX_COORD(tex, sx, sy, sw, sh, 0, 1, 2, 3);
    //SET_COLOR(tex->mask,tex->dtype);
	if(dw==0&&dh==0){
		SET_IMAGE_RECT_BY_TEX(tex, dx, dy);
	}else{
		SET_IMAGE_RECT(dx, dy, dw, dh);
	}
	AFTER_DRAW_IMAGE();
}

void DrawImageMask(image_p tex,float sx,float sy,float sw,float sh,float dx,float dy,float dw,float dh,int mask)
{
	BEFORE_DRAW_IMAGE();
	SET_TEX_COORD(tex, sx, sy, sw, sh, 0, 1, 2, 3);

	if(dw==0&&dh==0){
		SET_IMAGE_RECT_BY_TEX(tex, dx, dy);
	}else{
		SET_IMAGE_RECT(dx, dy, dw, dh);
	}
	SET_COLOR(mask,tex->dtype);
	AFTER_DRAW_IMAGE();
}

void RenderQuad(image_p tex,float sx,float sy,float sw,float sh,float dx,float dy,float xscale ,float yscale,float angle,int mask)
{
	BEFORE_DRAW_IMAGE();
	SET_TEX_COORD(tex, sx, sy, sw, sh, 0, 1, 2, 3);

	if (sw == 0) sw = tex->w;
	if (sh == 0) sh = tex->h;
	SET_IMAGE_RECT(dx, dy, sw*xscale, sh*yscale);
	ROTATE_2D(angle, dx+tex->rcentrex*xscale, dy+tex->rcentrey*yscale);
	SET_COLOR(mask,tex->dtype);
	AFTER_DRAW_IMAGE();
}


//static function for DrawRegion
/* NGE_TRANS_x for private use */
enum{
	NGE_TRANS_NONE = 0,
	NGE_TRANS_V,
	NGE_TRANS_H,
	NGE_TRANS_HV
};

#define SET_IMAGE_TRANS(trans, tex)					\
	switch(trans){									\
	case NGE_TRANS_V:								\
		SET_TEX_COORD(tex, 0, 0, 0, 0, 1, 0, 3, 2); \
		break;										\
	case NGE_TRANS_H:								\
		SET_TEX_COORD(tex, 0, 0, 0, 0, 3, 2, 1, 0); \
		break;										\
	case NGE_TRANS_HV:								\
		SET_TEX_COORD(tex, 0, 0, 0, 0, 2, 1, 0, 3); \
		break;										\
	default:										\
		SET_TEX_COORD(tex, 0, 0, 0, 0, 0, 1, 2, 3); \
	}

static void RenderQuadTrans(image_p tex,float sx ,float sy ,float sw ,float sh ,float dx ,float dy ,float xscale  ,float yscale ,float angle ,int mask,int trans)
{
	BEFORE_DRAW_IMAGE();
	SET_IMAGE_TRANS(trans, tex);

	if (sw == 0) sw = tex->w;
	if (sh == 0) sh = tex->h;
	SET_IMAGE_RECT(dx, dy, sw*xscale, sh*yscale);
	ROTATE_2D(angle, dx+tex->rcentrex*xscale, dy+tex->rcentrey*yscale);
	SET_COLOR(mask,tex->dtype);
	AFTER_DRAW_IMAGE();
}

//////////////////////////////////////////////////////////////////////////
void DrawRegion(image_p tex,int x_src, int y_src, int width, int height, int transform, int x_dest, int y_dest, int anchor)
{
	if(tex == NULL){
		return ;
	}

	//////////////////////////////////////////////////////////////////////////
	//锚点的正确性
	switch (anchor) {
	case 0:
	case ANCHOR_TOP | ANCHOR_LEFT:
	case ANCHOR_TOP | ANCHOR_RIGHT:
	case ANCHOR_BOTTOM | ANCHOR_LEFT:
	case ANCHOR_BOTTOM | ANCHOR_RIGHT:
	case ANCHOR_TOP | ANCHOR_HCENTER:
	case ANCHOR_BOTTOM | ANCHOR_HCENTER:
	case ANCHOR_LEFT | ANCHOR_VCENTER:
	case ANCHOR_RIGHT | ANCHOR_VCENTER:
	case ANCHOR_HCENTER | ANCHOR_VCENTER:
		break;
	default:
		return;
	}

	if (transform < TRANS_NONE || transform > TRANS_MIRROR_ROT90) {
		return;
	}

	if(width < 0 || height < 0 || x_src < 0 || y_src < 0 || x_src + width > tex->w || y_src + height > tex->h) {
		return;
	}

	//////////////////////////////////////////////////////////////////////////
	//移动位置
	if ((INVERTED_AXES & transform) != 0) {
		//////////////////////////////////////////////////////////////////////////
		//高宽度坐标,位置偏移 目标是新的坐标要算到老的位置去
		switch(transform){
		case TRANS_ROT90:
			{
				//调试通过
				//偏移(x_dest - (int)(m_pImage->rcentrex - m_pImage->rcentrey)*10/10 - y_src)，最后在剪掉剩下的值(m_pImage->h - y_src - height)
				x_dest	= x_dest - (int)(tex->rcentrex - tex->rcentrey)*10/10 - y_src - (tex->h - y_src - height);
				y_dest	= y_dest + (int)(tex->rcentrex - tex->rcentrey)*10/10;
			}
			break;
		case TRANS_ROT270:
			{
				//调试通过
				x_dest	= x_dest - (int)(tex->rcentrex - tex->rcentrey)*10/10;
				if(width == 0){
					y_dest	= y_dest - (int)(tex->rcentrex + tex->rcentrey)*10/10 + (int)tex->w;
				}else{
					y_dest	= y_dest - (int)(tex->rcentrex + tex->rcentrey)*10/10 + width;
				}
			}
			break;
		case TRANS_MIRROR_ROT90:
			{
				//调试通过
				x_dest	= x_dest - (int)(tex->rcentrex - tex->rcentrey)*10/10;
				y_dest	= y_dest + (int)(tex->rcentrex - tex->rcentrey)*10/10 - (tex->w - width);
			}
			break;
		case TRANS_MIRROR_ROT270:
			{
				//调试通过
				x_dest	= x_dest - (int)(tex->rcentrex - tex->rcentrey)*10/10 - y_src - (tex->h - y_src - height);
				y_dest	= y_dest + (int)(tex->rcentrex - tex->rcentrey)*10/10;
			}
			break;
		}
	}else{
		//////////////////////////////////////////////////////////////////////////
		//高宽度坐标,位置偏移
		switch(transform){
		case TRANS_ROT180:
			{
				//调试通过
				//if(width != 0){
				x_dest	= x_dest - ((int)tex->w - (/*x_src + */width));
				y_dest	= y_dest - (tex->h - height);
				//}
			}
			break;
		case TRANS_MIRROR_ROT180:
			{
				//调试通过
				// 				if(width != 0){
				// 					x_dest	= x_dest - ((int)m_pImage->w - (/*x_src + */width));
				// 				}
			}
			break;
		case TRANS_MIRROR:
			{
				//调试通过
				x_dest	= x_dest - ((int)tex->w - (/*x_src + */width));
				y_dest	= y_dest - (tex->h - height);
			}
			break;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//锚点确定截取屏幕的位置
	if((INVERTED_AXES & transform) == 0) {
		if(anchor != ANCHOR_SOLID) {
			if((anchor & ANCHOR_BOTTOM) != 0) {
				y_dest -= height;
			}
			if((anchor & ANCHOR_RIGHT) != 0) {
				x_dest -= width;
			}
			if((anchor & ANCHOR_HCENTER) != 0) {
				x_dest -= width / 2;
			}
			if((anchor & ANCHOR_VCENTER) != 0) {
				y_dest -= height / 2;
			}
		}
	}else{
		if (anchor != ANCHOR_SOLID) {
			if((anchor & ANCHOR_BOTTOM) != 0) {
				y_dest -= width;
			}
			if((anchor & ANCHOR_RIGHT) != 0) {
				x_dest -= height;
			}
			if((anchor & ANCHOR_HCENTER) != 0) {
				x_dest -= height / 2;
			}
			if ((anchor & ANCHOR_VCENTER) != 0) {
				y_dest -= width / 2;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	switch (transform)
	{
	case TRANS_ROT90:
		RenderQuad(tex, x_src, y_src, width, height, x_dest, y_dest, 1.0f, 1.0f, 90.0f, tex->mask);
		break;
	case TRANS_ROT180:
		RenderQuad(tex, x_src, y_src, width, height, x_dest, y_dest, 1.0f, 1.0f, 180.0f, tex->mask);
		break;
	case TRANS_ROT270:
		RenderQuad(tex, x_src, y_src, width, height, x_dest, y_dest, 1.0f, 1.0f, 270.0f, tex->mask);
		break;
	case TRANS_MIRROR:
		//RenderQuadTrans(m_pImage, x_src, y_src, width, height, x_dest, y_dest, 1.0f, 1.0f, 0.0f, m_pImage->mask, NGE_TRANS_V);
		RenderQuadTrans(tex, x_src, y_src, width, height, x_dest, y_dest, 1.0f, 1.0f, 180.0f, tex->mask, NGE_TRANS_V);
		break;
	case TRANS_MIRROR_ROT90:
		//RenderQuadTrans(m_pImage, x_src, y_src, width, height, x_dest, y_dest, 1.0f, 1.0f, 90.0f, m_pImage->mask, NGE_TRANS_V);
		RenderQuadTrans(tex, x_src, y_src, width, height, x_dest, y_dest, 1.0f, 1.0f, 270.0f, tex->mask, NGE_TRANS_V);
		break;
	case TRANS_MIRROR_ROT180:
		//RenderQuadTrans(m_pImage, x_src, y_src, width, height, x_dest, y_dest, 1.0f, 1.0f, 180.0f, m_pImage->mask, NGE_TRANS_V);
		RenderQuadTrans(tex, x_src, y_src, width, height, x_dest, y_dest, 1.0f, 1.0f, 0.0f, tex->mask, NGE_TRANS_V);
		break;
	case TRANS_MIRROR_ROT270:
		//RenderQuadTrans(m_pImage, x_src, y_src, width, height, x_dest, y_dest, 1.0f, 1.0f, 270.0f, m_pImage->mask, NGE_TRANS_V);
		RenderQuadTrans(tex, x_src, y_src, width, height, x_dest, y_dest, 1.0f, 1.0f, 90.0f, tex->mask, NGE_TRANS_V);
		break;
	default:
		RenderQuad(tex, x_src, y_src, width, height, x_dest, y_dest, 1.0f, 1.0f, 0.0f, tex->mask);
		break;
	}
}



image_p ScreenToImage()
{
	image_p pimage = image_create(nge_screen.width,nge_screen.height,DISPLAY_PIXEL_FORMAT_8888);
	if(pimage == NULL)
		return NULL;
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glReadPixels( 0, 0, nge_screen.width, nge_screen.height, GL_RGBA, GL_UNSIGNED_BYTE, pimage->data );
	image_flipv(pimage);
	return pimage;
}

void ScreenShot(const char* filename)
{
	image_p pimage = ScreenToImage();
	if(filename == NULL||pimage == NULL)
		return;
	image_save(pimage,filename,1, 1);
	image_free(pimage);
}

BOOL BeginTarget(image_p _img,uint8_t clear){
    static int cacheid = 0;
	static int ret = 0;
	if(!_img)
		return FALSE;
	BIND_AND_TEST_CACHE(_img);
	//GL_MAX is not define in OPENGLES
    glBlendEquationSeparate(GL_FUNC_ADD, 0x8008/*GL_MAX*/);
#if defined NGE_WIN || defined NGE_LINUX	
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, cacheid, 0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,_img->w,0,_img->h, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushAttrib(GL_VIEWPORT_BIT);
	glViewport(0,0,_img->w, _img->h);
#elif defined NGE_IPHONE || defined NGE_ANDROID
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, cacheid, 0);
    glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,_img->w,0,_img->h, -1, 1);
    glMatrixMode(GL_MODELVIEW);
	glViewport(0,0,_img->w, _img->h);
#endif
    if(clear){
        glClearColor( COLOR_T_R(screen_c), COLOR_T_G(screen_c), COLOR_T_B(screen_c), COLOR_T_A(screen_c) );
        glClear( GL_COLOR_BUFFER_BIT);
    }
	return TRUE;
}

void EndTarget(){
#if defined NGE_WIN || defined NGE_LINUX
	glPopAttrib();
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,nge_screen.ori_width,nge_screen.ori_height,0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
#elif defined NGE_IPHONE || defined NGE_ANDROID
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,nge_screen.ori_width,nge_screen.ori_height,0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
#endif
    glBlendEquation(GL_FUNC_ADD);
}

image_p TargetToImage(int x,int y,int width,int height)
{
	image_p image = NULL;
	if(width < 0 && height < 0 && x < 0 && y < 0)
		return NULL;
	image = image_create(width,height,DISPLAY_PIXEL_FORMAT_8888);
	if(image){
		glReadPixels(x, y, image->texw, image->texh,GL_RGBA,image->dtype, image->data);
		image->modified = 1;
	}
	return image;
}


/*高级用户使用的模式,批处理模式*/
void Translate(float x,float y)
{
	glTranslatef(x,y,0);
}

void Scale(float x,float y)
{
	glScalef(x,y,1.0f);
}

void Rotate(float angle)
{
	glRotatef(angle,0,0,1); 
}

void Identity()
{
	glLoadIdentity();
}

void PushMatrix()
{
	glPushMatrix();
}

void PopMatrix()
{
	glPopMatrix();
}

void DrawImageBatch(image_p tex,rectf* uv_rect)
{
	BEFORE_DRAW_IMAGE();
	if(uv_rect == NULL){
		SET_TEX_COORD(tex, 0, 0, 0, 0, 0, 1, 2, 3);
	}
	else{
		SET_TEX_COORD(tex, uv_rect->left, uv_rect->top, uv_rect->left - uv_rect->right, 
		                uv_rect->bottom - uv_rect->top, 0, 1, 2, 3);
	}
	SET_IMAGE_RECT_BY_TEX(tex, 0, 0);
	AFTER_DRAW_IMAGE();
}
/*批处理模式END*/

void RealRenderQuad(quadf quad) {
	BEFORE_DRAW_QUAD();
	TRANSLATE_QUAD();
	AFTER_DRAW_QUAD();
}