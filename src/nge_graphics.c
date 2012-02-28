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
#include "nge_graphics.h"

#if defined NGE_IPHONE || defined NGE_ANDROID
#define NGE_GLES
#ifdef NGE_IPHONE
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#else
#include <GLES/gl.h>
#endif

#define glOrtho glOrthof
#elif defined NGE_WIN || defined NGE_LINUX

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
static uint32 m_frame = 0;
static uint32 m_t0 = 0;
static int fps_last_ticks = 0;

#define MAX_TEX_CACHE_SIZE 32
GLuint m_texcache[MAX_TEX_CACHE_SIZE];

// nge_screen *************************
static screen_context_t nge_screen = {
	"NGE2",
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

static uint32 max_vectices=0,max_colors=0,max_tex_uvs=0;
#define GL_ARRAY_CHECK_V(size)											\
	if(((uint32)size) > max_vectices){									\
		max_vectices = size;											\
		SAFE_FREE(gl_vectices);											\
		gl_vectices = (Vectice2D_t*)malloc(size*sizeof(Vectice2D_t));	\
		glVertexPointer(2,GL_FLOAT,sizeof(Vectice2D_t),gl_vectices);	\
	}

#define GL_ARRAY_CHECK_C(size)											\
	if(((uint32)size) > max_colors)										\
	{																	\
		max_colors = size;												\
		SAFE_FREE(gl_colors);											\
		gl_colors = (Color_t*)malloc(size*sizeof(Color_t));				\
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Color_t), gl_colors); \
	}

#define GL_ARRAY_CHECK_T(size)											\
	if(((uint32)size) > max_tex_uvs)									\
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

static inline void GetRGBA(int color,int dtype,uint8* r,uint8* g,uint8* b,uint8* a)
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

uint32 SetScreenColor(uint8 r,uint8 g,uint8 b,uint8 a)
{
	uint32 u_lastcolor;
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

void SetScreenType(int type)
{
#ifdef NGE_IPHONE
	if(type == 2){
		nge_screen.fullscreen = 2;
		SetSwapXY(1);
	}
	else{
		nge_screen.fullscreen = 0;
		SetSwapXY(0);
	}
#endif
}

void SetTexBlend(int src_blend, int des_blend)
{
	if(src_blend==0&&des_blend==0)
		glBlendFunc(GL_SRC_ALPHA,GL_ONE);
	else
		glBlendFunc(src_blend,des_blend);
}

void ResetTexBlend()
{
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
}

void SetClip(int x,int y,int w,int h)
{
	/* float rate_w_ori = 1/nge_screen.rate_w; */
	/* float rate_h_ori = 1/nge_screen.rate_h; */
	/* glScissor(floor(x*rate_w_ori),floor(nge_screen.height-rate_h_ori* (y-h)), */
	/* 		  ceil(w*rate_w_ori),ceil(h*rate_h_ori)); */
	glScissor(x,nge_screen.height-y-h,w,h);
}

void ResetClip()
{
	SetClip(0,0,nge_screen.width, nge_screen.height);
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

void nge_graphics_reset(void)
{
	int i;
	// reset for fps------------------
	m_frame = 0;
	m_t0 = 0;
	fps_last_ticks = nge_get_tick();

	// reset cache
	tex_cache_clear();
	glGenTextures( MAX_TEX_CACHE_SIZE, &m_texcache[0] );
	for(i=0;i<MAX_TEX_CACHE_SIZE;i++){
		tex_cache_add(i,m_texcache[i]);
		glBindTexture(GL_TEXTURE_2D, m_texcache[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,nge_screen.width,nge_screen.height,0, -1, 1);
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
}

void DisableOpenGL(HWND hWnd, HDC hDC, HGLRC hRC)
{
	wglMakeCurrent( NULL, NULL );
	wglDeleteContext( hRC );
	ReleaseDC( hWnd, hDC );
}

static HWND hWnd;
static HDC hDC;
static HGLRC hRC;

void makeWindow(const char *name, int x, int y, int width, int height)
{
	HINSTANCE hInstance;
	WNDCLASS wc;

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
}
#endif

void InitGrahics()
{
	int i = 0;
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

	tex_cache_init(MAX_TEX_CACHE_SIZE);
	nge_graphics_reset();

	nge_log("Init Graphics Ok\n");
}

void FiniGrahics()
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
}

void ShowFps()
{
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
}

void LimitFps(uint32 limit)
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

void BeginScene(uint8 clear)
{
	if(clear == 1){
		glDisable(GL_SCISSOR_TEST);
		glClearColor( COLOR_T_R(screen_c), COLOR_T_G(screen_c), COLOR_T_B(screen_c), COLOR_T_A(screen_c) );
		glClear( GL_COLOR_BUFFER_BIT);
		glEnable(GL_SCISSOR_TEST);
	}
	glLoadIdentity();
	if(nge_screen.fullscreen == 2){
		ROTATE_2D(90, nge_screen.width/2,nge_screen.height/2);
	}
}

void EndScene()
{
#if defined NGE_LINUX
	glXSwapBuffers(g_dpy, g_win);
#elif defined NGE_WIN
	SwapBuffers( hDC );
#elif defined NGE_ANDROID
	glFlush();
#endif
}

static uint8 r,g,b,a;
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
	BEFORE_DRAW();
	if(y1 == 0.0)
		y1 = 0.1;
	if(y2 == 0.0)
		y2 = 0.1;
	GL_ARRAY_CHECK_V(2);
	VECT_2D_SET(gl_vectices[0], x1, y1);
	VECT_2D_SET(gl_vectices[1], x2, y2);
	SET_COLOR(color, dtype);
	glDrawArrays(GL_LINES, 0, 2);
	AFTER_DRAW();
}

void DrawLineEx(pointf p1,pointf p2 ,int color,int dtype)
{
	DrawLine(p1.x, p1.y, p2.x, p2.y,color,dtype);
}

void DrawCircle(float x, float y, float radius, int color,int dtype)
{
	int i;
	BEFORE_DRAW();
	GL_ARRAY_CHECK_V(360);
	for(i=0; i<360;i++)
	{
		VECT_2D_SET(gl_vectices[i], x+radius*COSF(i), y+radius*SINF(i));
	}
	SET_COLOR(color, dtype);
	glDrawArrays(GL_LINE_LOOP, 0, 360);
	AFTER_DRAW();
}

void FillCircle(float x, float y, float radius, int color,int dtype)
{
	int i;
	BEFORE_DRAW();
	GL_ARRAY_CHECK_V(360);
	for(i=0; i<360;i++)
	{
		VECT_2D_SET(gl_vectices[i], x+radius*COSF(i), y+radius*SINF(i));
	}
	SET_COLOR(color, dtype);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 360);
	AFTER_DRAW();
}

void DrawEllipse(float x,float y ,float xradius,float yradius,int color,int dtype)
{
	int i;
	BEFORE_DRAW();
	GL_ARRAY_CHECK_V(360);
	for(i=0; i<360;i++)
	{
		VECT_2D_SET(gl_vectices[i], x+xradius*COSF(i), y+yradius*SINF(i));
	}
	SET_COLOR(color, dtype);
	glDrawArrays(GL_LINE_LOOP, 0, 360);
	AFTER_DRAW();
}

void FillEllipse(float x,float y ,float xradius,float yradius,int color,int dtype)
{
	int i;
	BEFORE_DRAW();
	GL_ARRAY_CHECK_V(360);
	for(i=0; i<360;i++)
	{
		VECT_2D_SET(gl_vectices[i], x+xradius*COSF(i), y+yradius*SINF(i));
	}
	SET_COLOR(color, dtype);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 360);
	AFTER_DRAW();
}

void PutPix(float x,float y ,int color,int dtype)
{
	BEFORE_DRAW();
	GL_ARRAY_CHECK_V(1);
	VECT_2D_SET(gl_vectices[0], x, y);
	SET_COLOR(color,dtype);
	glDrawArrays(GL_POINTS, 0, 1);
	AFTER_DRAW();
}

#define DRAW_POLYGON_IMP(mode, count)					\
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
	AFTER_DRAW()

#define DRAW_POLYGON_IMP_COLOR(mode, count)				\
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
	AFTER_DRAW()

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
	if(dy == 0.0)
		dy = 0.1;
	DRAW_POLYGON_IMP(GL_LINE_LOOP, 4);
}

void DrawRectEx(rectf rect,int color,int dtype)
{
	DrawRect(rect.top, rect.left, rect.right-rect.left, rect.bottom-rect.top,color,dtype);
}

void FillRect(float dx, float dy, float width, float height,int color,int dtype)
{
	SET_RECT_ARRAY();
	if(dy == 0.0)
		dy = 0.1;
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
	if(dy == 0.0)
		dy = 0.1;
	DRAW_POLYGON_IMP_COLOR(GL_TRIANGLE_FAN, 4);
}

inline void FillRectGradEx(rectf rect,int* colors,int dtype)
{
	FillRectGrad(rect.top, rect.left, rect.right-rect.left, rect.bottom-rect.top,colors,dtype);
}

/** 填充三角型(单色)
 *@param pointf v0,顶点坐标v1
 *@param pointf v2,顶点坐标v2
 *@param pointf v2,顶点坐标v3
 *@param int color,填充色
 *@param int dtype,显示模式,填充色要与之相对应
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
	uint32 format = GL_RGBA;
	if(pimg->dtype == DISPLAY_PIXEL_FORMAT_565){
		format = GL_RGB;
	}
	glTexImage2D(GL_TEXTURE_2D, 0, format, pimg->texw, pimg->texh, 0, format, pimg->dtype, pimg->data);
}

static int cacheid = 0;
static uint8 tex_ret = 0;
#define BIND_AND_TEST_CACHE(tex)						\
	do {												\
		tex_ret = tex_cache_getid(tex->texid,&cacheid);	\
		glBindTexture(GL_TEXTURE_2D, cacheid);			\
		if(tex_ret == 0 ||tex->modified==1){			\
			TexImage2D(tex);							\
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

void ImageToScreen(image_p tex,float dx,float dy)
{
	BEFORE_DRAW_IMAGE();
	SET_TEX_COORD(tex, 0, 0, 0, 0, 0, 1, 2, 3);
	SET_IMAGE_RECT_BY_TEX(tex, dx, dy);
	AFTER_DRAW_IMAGE();
}

void DrawImage(image_p tex,float sx,float sy,float sw,float sh,float dx,float dy,float dw,float dh)
{
	BEFORE_DRAW_IMAGE();
	SET_TEX_COORD(tex, sx, sy, sw, sh, 0, 1, 2, 3);

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
	if(dy == 0.0f)
		dy = 0.1f;
	BEFORE_DRAW_IMAGE();
	SET_TEX_COORD(tex, sx, sy, sw, sh, 0, 1, 2, 3);

	if (sw == 0) sw = tex->w;
	if (sh == 0) sh = tex->h;
	SET_IMAGE_RECT(dx, dy, sw*xscale, sh*yscale);
	ROTATE_2D(angle, dx+sw*xscale/2, dy+sh*yscale/2);
	SET_COLOR(mask,tex->dtype);
	AFTER_DRAW_IMAGE();
}

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

void ImageToScreenTrans(image_p tex,float dx,float dy,int trans)
{
	BEFORE_DRAW_IMAGE();
	SET_IMAGE_TRANS(trans, tex);
	SET_IMAGE_RECT_BY_TEX(tex, dx, dy);
	AFTER_DRAW_IMAGE();
}

void DrawImageTrans(image_p tex,float sx,float sy,float sw,float sh,float dx,float dy,float dw,float dh,int trans)
{
	BEFORE_DRAW_IMAGE();
	SET_IMAGE_TRANS(trans, tex);

	if(dw==0&&dh==0){
		SET_IMAGE_RECT_BY_TEX(tex, dx, dy);
	}else{
		SET_IMAGE_RECT(dx, dy, dw, dh);
	}
	AFTER_DRAW_IMAGE();
}

void DrawImageMaskTrans(image_p tex,float sx , float sy, float sw, float sh, float dx, float dy, float dw, float dh,int mask,int trans)
{
	BEFORE_DRAW_IMAGE();
	SET_IMAGE_TRANS(trans, tex);

	if(dw==0&&dh==0){
		SET_IMAGE_RECT_BY_TEX(tex, dx, dy);
	}else{
		SET_IMAGE_RECT(dx, dy, dw, dh);
	}
	SET_COLOR(mask,tex->dtype);
	AFTER_DRAW_IMAGE();
}

void RenderQuadTrans(image_p tex,float sx ,float sy ,float sw ,float sh ,float dx ,float dy ,float xscale  ,float yscale ,float angle ,int mask,int trans)
{
	if(dy == 0.0f)
		dy = 0.1f;
	BEFORE_DRAW_IMAGE();
	SET_IMAGE_TRANS(trans, tex);

	if (sw == 0) sw = tex->w;
	if (sh == 0) sh = tex->h;
	SET_IMAGE_RECT(dx, dy, sw*xscale, sh*yscale);
	ROTATE_2D(angle, dx+sw*xscale/2, dy+sh*yscale/2);
	SET_COLOR(mask,tex->dtype);
	AFTER_DRAW_IMAGE();
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
