/* @(#)nge_graphics.c
 */

#if defined WIN32 || defined __linux__
#include <GL/glut.h>
#include <GL/gl.h>
#include <SDL.h>
#endif

#if defined WIN32 || defined __linux__ || defined IPHONEOS
#include <math.h>
#include "nge_graphics.h"
#include "nge_image_load.h"
#include "nge_timer.h"
#include "nge_misc.h"
#include "nge_tex_cache.h"
#endif

#if defined IPHONEOS
#include "nge_input_proc.h"
#endif

#if defined WIN32 || defined __linux__ || defined IPHONEOS
//fps count
static uint32 m_frame = 0;
static uint32 m_t0 = 0;

static uint32 m_tex_in_ram = -1;
#define MAX_TEX_CACHE_SIZE 32
int     m_texcache[MAX_TEX_CACHE_SIZE];

static float m_sintable[360];
static float m_costable[360];
#define RAD2DEG		57.29577951f
#define DEG2RAD		0.017453293f
#define SINF(a)  (m_sintable[a%360])
#define COSF(a)  (m_costable[a%360])

// nge_screen *************************
static screen_context_t nge_screen = {
	"NGE2",
	SCREEN_WIDTH,
	SCREEN_HEIGHT,
	SCREEN_BPP,
	0
};

static float screen_r =0.0, screen_g =0.0, screen_b =0.0, screen_a =1.0;

static void GetRGBA(int color,int dtype,uint8* r,uint8* g,uint8* b,uint8* a)
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

uint32 SetScreenColor(uint8 r,uint8 g,uint8 b,uint8 a)
{
	uint32 u_lastcolor;
	u_lastcolor = MAKE_RGBA_8888(((int)(screen_r*255)),((int)(screen_g*255)),((int)(screen_b*255)),((int)(screen_a*255)));
	screen_r = r/255.0;
	screen_g = g/255.0;
	screen_b = b/255.0;
	screen_a = a/255.0;
	return u_lastcolor;
}

screen_context_p GetScreenContext()
{
	return &nge_screen;
}


void SetScreenType(int type)
{
}
#endif

#if defined WIN32 || defined __linux__
static uint32 m_tex_id = 0;
static nge_timer* timer = NULL;


>>>>>>> Stashed changes
// ******************************

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

void InitGrahics()
{
	int i = 0;
	int screen_flag = 0;
#if defined(WIN32) || defined(__linux__)
	if ( SDL_Init(SDL_INIT_VIDEO) < 0 )
		exit(1);
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 1);
	SDL_WM_SetCaption(nge_screen->name,NULL);

	screen_flag = SDL_OPENGL|SDL_HWSURFACE|SDL_DOUBLEBUF;
	if(nge_screen->fullscreen != 0)
		screen_flag |= SDL_FULLSCREEN;
	SDL_SetVideoMode( nge_screen->width, nge_screen->height, nge_screen->bpp,screen_flag);
#endif

	timer = timer_create();
	tex_cache_init(MAX_TEX_CACHE_SIZE);

	glEnable( GL_TEXTURE_2D );
	glDisable( GL_DEPTH_TEST );
	glShadeModel( GL_SMOOTH );
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glGenTextures( MAX_TEX_CACHE_SIZE, &m_texcache[0] );
	for(i=0;i<MAX_TEX_CACHE_SIZE;i++){
		tex_cache_add(i,m_texcache[i]);
		glBindTexture(GL_TEXTURE_2D, m_texcache[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	glEnable(GL_SCISSOR_TEST);
	ResetClip();
	for (i=0;i<360;i++)
	{
		m_sintable[i] = sin(i*DEG2RAD);
		m_costable[i] = cos(i*DEG2RAD);
	}
}

void FiniGrahics()
{
	timer_free(timer);
	tex_cache_fini();
	glDeleteTextures(MAX_TEX_CACHE_SIZE,m_texcache);
}

void BeginScene(uint8 clear)
{
	if(clear == 1){
		glDisable(GL_SCISSOR_TEST);
		glClearColor( screen_r, screen_g, screen_b, screen_a );
		glClear( GL_COLOR_BUFFER_BIT);
		glEnable(GL_SCISSOR_TEST);
	}
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,nge_screen.width,nge_screen.height,0,-1, 1);
}

void EndScene()
{
	SDL_GL_SwapBuffers();
}

void SetClip(int x,int y,int w,int h)
{
	glScissor(x,nge_screen.height-y-h,w,h);
}

void ResetClip()
{
	SetClip(0, 0, nge_screen.width, nge_screen.height);
}

void FpsInit()
{
	m_frame = 0;
	m_t0 = 0;
}

void ShowFps()
{
	static int init_fps = 0;
	int t;
	float seconds,fps;

	if(init_fps == 0){
		FpsInit();
		init_fps = 1;
	}
	m_frame++;
   	t = SDL_GetTicks();
	if (t - m_t0 >= 1000) {
		seconds = (t - m_t0) / 1000.0;
		fps = m_frame / seconds;
		printf("%d frames in %g seconds = %g FPS\n", m_frame, seconds, fps);
		m_t0 = t;
		m_frame = 0;
	}
}

void LimitFps(uint32 limit)
{
	if(limit == 0)
		limit = 60;
	if( timer->get_ticks(timer) < 1000 /limit )
	{
		nge_sleep( ( 1000 / limit) - timer->get_ticks(timer) );
	}
	timer->start(timer);
}

void DrawLine(float x1, float y1, float x2, float y2, int color,int dtype)
{
	static uint8 r,g,b,a;

	if(y1 == 0.0)
		y1 = 0.1;
	if(y2 == 0.0)
		y2 = 0.1;
	glDisable(GL_TEXTURE_2D);
	glPushAttrib(GL_CURRENT_BIT);
	GetRGBA(color,dtype,&r,&g,&b,&a);
	glColor4ub(r, g, b, a);
	glBegin(GL_LINES);
	glVertex2f(x1, y1);
	glVertex2f(x2, y2);
	glEnd();
	glPopAttrib();
	glEnable(GL_TEXTURE_2D);
}

void DrawLineEx(pointf p1,pointf p2 ,int color,int dtype)
{
	DrawLine(p1.x, p1.y, p2.x, p2.y,color,dtype);
}

void DrawCircle(float x, float y, float radius, int color,int dtype)
{
	static uint8 r,g,b,a;
	static int i;
	glDisable(GL_TEXTURE_2D);
	glPushAttrib(GL_CURRENT_BIT);
	GetRGBA(color,dtype,&r,&g,&b,&a);
	glColor4ub(r, g, b, a);
	glBegin(GL_LINE_LOOP);
	for(i=0; i<360;i++)
	{
		glVertex2f(x+radius*COSF(i), y+radius*SINF(i));
	}
	glEnd();
	glPopAttrib();
	glEnable(GL_TEXTURE_2D);
}

void FillCircle(float x, float y, float radius, int color,int dtype)
{
	static uint8 r,g,b,a;
	static int i;
	glDisable(GL_TEXTURE_2D);
	glPushAttrib(GL_CURRENT_BIT);
	GetRGBA(color,dtype,&r,&g,&b,&a);
	glColor4ub(r, g, b, a);
	glBegin(GL_TRIANGLE_FAN);
	for(i=0; i<360;i+=1)
	{
		glVertex2f(x+radius*COSF(i), y+radius*SINF(i));
	}
	glEnd();
	glPopAttrib();
	glEnable(GL_TEXTURE_2D);
}

void DrawEllipse(float x,float y ,float xradius,float yradius,int color,int dtype)
{
	static uint8 r,g,b,a;
	static int i;
	glDisable(GL_TEXTURE_2D);
	glPushAttrib(GL_CURRENT_BIT);
	GetRGBA(color,dtype,&r,&g,&b,&a);
	glColor4ub(r, g, b, a);
	glBegin(GL_LINE_LOOP);
	for   (i=0;   i<360;   i++)
	{
		glVertex2f(x+COSF(i)*xradius,y+SINF(i)*yradius);
	}
	glEnd();
	glPopAttrib();
	glEnable(GL_TEXTURE_2D);
}
void FillEllipse(float x,float y ,float xradius,float yradius,int color,int dtype)
{
	static uint8 r,g,b,a;
	static int i;
	glDisable(GL_TEXTURE_2D);
	glPushAttrib(GL_CURRENT_BIT);
	GetRGBA(color,dtype,&r,&g,&b,&a);
	glColor4ub(r, g, b, a);
	glBegin(GL_TRIANGLE_FAN);
	for   (i = 0;i < 360;i++)
	{
		glVertex2f(x+COSF(i)*xradius,y+SINF(i)*yradius);
	}
	glEnd();
	glPopAttrib();
	glEnable(GL_TEXTURE_2D);
}

void PutPix(float x,float y ,int color,int dtype)
{
	static uint8 r,g,b,a;
	static int i;
	glDisable(GL_TEXTURE_2D);
	glPushAttrib(GL_CURRENT_BIT);
	GetRGBA(color,dtype,&r,&g,&b,&a);
	glColor4ub(r, g, b, a);
	glBegin(GL_POINTS);
	glVertex2f(x,y);
	glEnd();
	glPopAttrib();
	glEnable(GL_TEXTURE_2D);
}

void DrawPolygon(float* x, float* y, int count, int color,int dtype)
{
	static uint8 r,g,b,a;
	static int i;
	glDisable(GL_TEXTURE_2D);
	glPushAttrib(GL_CURRENT_BIT);
	GetRGBA(color,dtype,&r,&g,&b,&a);
	glColor4ub(r, g, b, a);
	glBegin(GL_LINE_STRIP);

	for(i=0; i<count;i++)
	{
		//glVertex2f(x[i],SCREEN_HEIGHT-y[i]);
		glVertex2f(x[i],y[i]);
	}

	//glVertex2f(x[0],SCREEN_HEIGHT-y[0]);
	glVertex2f(x[0],y[0]);
	glEnd();

	glPopAttrib();
	glEnable(GL_TEXTURE_2D);
}

void FillPolygon(float* x, float* y, int count, int color,int dtype)
{
	static uint8 r,g,b,a;
	static int i;
	glDisable(GL_TEXTURE_2D);
	glPushAttrib(GL_CURRENT_BIT);
	GetRGBA(color,dtype,&r,&g,&b,&a);
	glColor4ub(r, g, b, a);
	glBegin(GL_TRIANGLE_FAN);

	for(i=0; i<count;i++)
	{
		glVertex2f(x[i],y[i]);
	}

	glEnd();
	glPopAttrib();
	glEnable(GL_TEXTURE_2D);
}

void FillPolygonGrad(float* x, float* y, int count, int* colors,int dtype)
{
	static uint8 r,g,b,a;
	static int i;
	glDisable(GL_TEXTURE_2D);
	glPushAttrib(GL_CURRENT_BIT);
	glBegin(GL_TRIANGLE_FAN);

	for(i=0; i<count;i++)
	{
		GetRGBA(colors[i],dtype,&r,&g,&b,&a);
		glColor4ub(r, g, b, a);
		glVertex2f(x[i],y[i]);
	}

	glEnd();
	glPopAttrib();
	glEnable(GL_TEXTURE_2D);
}

void DrawRect(float dx, float dy, float width, float height,int color,int dtype)
{
	static uint8 r,g,b,a;
	if(dy == 0.0)
		dy = 0.1;
	glDisable(GL_TEXTURE_2D);
	glPushAttrib(GL_CURRENT_BIT);

	GetRGBA(color,dtype,&r,&g,&b,&a);
	glColor4ub(r, g, b, a);
	glBegin(GL_LINE_LOOP);
	glVertex2f(dx, dy);
	glVertex2f(dx,dy+height);
	glVertex2f(dx+width,dy+height);
	glVertex2f(dx+width,dy);
	glEnd();
	glPopAttrib();
	glEnable(GL_TEXTURE_2D);
}

void DrawRectEx(rectf rect,int color,int dtype)
{
	DrawRect(rect.top, rect.left, rect.right-rect.left, rect.bottom-rect.top,color,dtype);
}

void FillRect(float dx, float dy, float width, float height,int color,int dtype)
{
	static uint8 r,g,b,a;
	if(dy == 0.0)
		dy = 0.1;
	glDisable(GL_TEXTURE_2D);
	glPushAttrib(GL_CURRENT_BIT);

	GetRGBA(color,dtype,&r,&g,&b,&a);
	glColor4ub(r, g, b, a);
	glBegin(GL_QUADS);
	glVertex2f(dx, dy);
	glVertex2f(dx,dy+height);
	glVertex2f(dx+width,dy+height);
	glVertex2f(dx+width,dy);
	glEnd();
	glPopAttrib();
	glEnable(GL_TEXTURE_2D);
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
	static color4ub vex0,vex1,vex2,vex3;
	if(dy == 0.0)
		dy = 0.1;
	glDisable(GL_TEXTURE_2D);
	glPushAttrib(GL_CURRENT_BIT);

	GetRGBA(colors[0],dtype,&vex0.r,&vex0.g,&vex0.b,&vex0.a);
	GetRGBA(colors[1],dtype,&vex1.r,&vex1.g,&vex1.b,&vex1.a);
	GetRGBA(colors[2],dtype,&vex2.r,&vex2.g,&vex2.b,&vex2.a);
	GetRGBA(colors[3],dtype,&vex3.r,&vex3.g,&vex3.b,&vex3.a);
	glBegin(GL_QUADS);
	glColor4ub(vex0.r, vex0.g,vex0.b,vex0.a);glVertex2f(dx, dy);
	glColor4ub(vex1.r, vex1.g,vex1.b,vex1.a);glVertex2f(dx,dy+height);
	glColor4ub(vex2.r, vex2.g,vex2.b,vex2.a);glVertex2f(dx+width,dy+height);
	glColor4ub(vex3.r, vex3.g,vex3.b,vex3.a);glVertex2f(dx+width,dy);
	glEnd();
	glPopAttrib();
	glEnable(GL_TEXTURE_2D);
}

void FillRectGradEx(rectf rect,int* colors,int dtype)
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
void FillTri(pointf v0,pointf v2,pointf v3 ,int color,int dtype)
{
}

int PreLoadImage(image_p pimg)
{
	return 1;
}

int TexImage2D(image_p pimg)
{
	uint32 format = GL_RGBA;
	m_tex_in_ram = pimg->texid;
	if(pimg->dtype == DISPLAY_PIXEL_FORMAT_565){
		format = GL_RGB;
	}
	glTexImage2D(GL_TEXTURE_2D, 0, 4, pimg->texw, pimg->texh, 0, format, pimg->dtype, pimg->data);
	return 1;
}

static void DrawImageRot(image_p texture,float sx,float sy,float sw,float sh,float dx,float dy,float dw,float dh,float angle,int mask)
{
	static uint8 r,g,b,a;
	static int cacheid = 0;
	static int ret = 0;
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(dx+texture->rcentrex,dy+texture->rcentrey,0);
	glRotatef(angle,0,0,1);
	glTranslatef(-(dx+texture->rcentrex),-(dy+texture->rcentrey),0);

	ret = tex_cache_getid(texture->texid,&cacheid);
	glBindTexture(GL_TEXTURE_2D, cacheid);
	if(ret == 0 ||texture->modified==1){
		TexImage2D(texture);
		texture->modified = 0;
	}
	if(sw==0&&sh==0){
		sw = texture->w;
		sh = texture->h;
	}
	if(dw==0&&dh==0){
		dw = sw;//texture->w;
		dh = sh;//texture->h;
	}
	glPushAttrib(GL_CURRENT_BIT);
	GetRGBA(mask,texture->dtype,&r,&g,&b,&a);
	glColor4ub(r, g, b, a);
	glBegin(GL_QUADS);
	glTexCoord2f(sx/texture->texw, sy/texture->texh); glVertex3f(dx, dy, 0);	// Bottom Left Of The Texture and Quad
	glTexCoord2f((sx+sw)/texture->texw, sy/texture->texh); glVertex3f( dx+dw,dy,0);	// Bottom Right Of The Texture and Quad
	glTexCoord2f((sx+sw)/texture->texw, (sy+sh)/texture->texh); glVertex3f( dx+dw,dy+dh,0);	// Top Right Of The Texture and Quad
	glTexCoord2f(sx/texture->texw, (sy+sh)/texture->texh); glVertex3f(dx,dy+dh,0);	// Top Left Of The Texture and Quad
	glEnd();
	glPopAttrib();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

static void DrawImageRotTrans(image_p texture,float sx,float sy,float sw,float sh,float dx,float dy,float dw,float dh,float angle,int mask,int trans)
{
	static uint8 r,g,b,a;
	static int cacheid = 0;
	static int ret = 0;
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(dx+texture->rcentrex,dy+texture->rcentrey,0);
	glRotatef(angle,0,0,1);
	glTranslatef(-(dx+texture->rcentrex),-(dy+texture->rcentrey),0);

	ret = tex_cache_getid(texture->texid,&cacheid);
	glBindTexture(GL_TEXTURE_2D, cacheid);
	if(ret == 0 ||texture->modified==1){
		TexImage2D(texture);
		texture->modified = 0;
	}
	if(sw==0&&sh==0){
		sw = texture->w;
		sh = texture->h;
	}
	if(dw==0&&dh==0){
		dw = sw;//texture->w;
		dh = sh;//texture->h;
	}
	glPushAttrib(GL_CURRENT_BIT);
	GetRGBA(mask,texture->dtype,&r,&g,&b,&a);
	glColor4ub(r, g, b, a);
	glBegin(GL_QUADS);
	switch(trans)
	{
	case NGE_TRANS_NONE:
		glTexCoord2f(sx/texture->texw, sy/texture->texh); glVertex3f(dx, dy, 0);	// Bottom Left Of The Texture and Quad
		glTexCoord2f((sx+sw)/texture->texw, sy/texture->texh); glVertex3f( dx+dw,dy,0);	// Bottom Right Of The Texture and Quad
		glTexCoord2f((sx+sw)/texture->texw, (sy+sh)/texture->texh); glVertex3f( dx+dw,dy+dh,0);	// Top Right Of The Texture and Quad
		glTexCoord2f(sx/texture->texw, (sy+sh)/texture->texh); glVertex3f(dx,dy+dh,0);	// Top Left Of The Texture and Quad
		break;
	case NGE_TRANS_V:
		glTexCoord2f(sx/texture->texw, sy/texture->texh); glVertex3f(dx, dy+dh, 0);	// Bottom Left Of The Texture and Quad
		glTexCoord2f((sx+sw)/texture->texw, sy/texture->texh); glVertex3f( dx+dw,dy+dh,0);	// Bottom Right Of The Texture and Quad
		glTexCoord2f((sx+sw)/texture->texw, (sy+sh)/texture->texh); glVertex3f( dx+dw,dy,0);	// Top Right Of The Texture and Quad
		glTexCoord2f(sx/texture->texw, (sy+sh)/texture->texh); glVertex3f(dx,dy,0);	// Top Left Of The Texture and Quad
		break;
	case NGE_TRANS_H:
		glTexCoord2f((sx)/texture->w, sy/texture->texh); glVertex3f(dx-(texture->texw-texture->w), dy, 0);	// Bottom Left Of The Texture and Quad
		glTexCoord2f((-1)*(sx+sw)/texture->w,sy/texture->texh); glVertex3f( dx+dw,dy,0);	// Bottom Right Of The Texture and Quad
		glTexCoord2f((-1)*(sx+sw)/texture->w, (sy+sh)/texture->texh); glVertex3f( dx+dw,dy+dh,0);	// Top Right Of The Texture and Quad
		glTexCoord2f((sx)/texture->w, (sy+sh)/texture->texh); glVertex3f(dx-(texture->texw-texture->w),dy+dh,0);	// Top Left Of The Texture and Quad
		break;
	case NGE_TRANS_HV:
		glTexCoord2f((sx)/texture->w, sy/texture->texh); glVertex3f(dx-(texture->texw-texture->w), dy+dh, 0);	// Bottom Left Of The Texture and Quad
		glTexCoord2f((-1)*(sx+sw)/texture->w,sy/texture->texh); glVertex3f( dx+dw,dy+dh,0);	// Bottom Right Of The Texture and Quad
		glTexCoord2f((-1)*(sx+sw)/texture->w, (sy+sh)/texture->texh); glVertex3f( dx+dw,dy,0);	// Top Right Of The Texture and Quad
		glTexCoord2f((sx)/texture->w, (sy+sh)/texture->texh); glVertex3f(dx-(texture->texw-texture->w),dy,0);	// Top Left Of The Texture and Quad
		break;
	default:
		glTexCoord2f(sx/texture->texw, sy/texture->texh); glVertex3f(dx, dy, 0);	// Bottom Left Of The Texture and Quad
		glTexCoord2f((sx+sw)/texture->texw, sy/texture->texh); glVertex3f( dx+dw,dy,0);	// Bottom Right Of The Texture and Quad
		glTexCoord2f((sx+sw)/texture->texw, (sy+sh)/texture->texh); glVertex3f( dx+dw,dy+dh,0);	// Top Right Of The Texture and Quad
		glTexCoord2f(sx/texture->texw, (sy+sh)/texture->texh); glVertex3f(dx,dy+dh,0);	// Top Left Of The Texture and Quad
		break;
	}
	glEnd();
	glPopAttrib();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


/*
  fix me
  void DrawLargeImageMask(image_p tex,float sx , float sy, float sw, float sh, float dx, float dy, float dw, float dh,int mask)
  {
  void DrawImageMask(tex,sx,sy,sw,sh,dx,dy,dw,dh,mask);
  }

  void DrawLargeImage(image_p tex,float sx , float sy, float sw, float sh, float dx, float dy, float dw, float dh,int mask)
  {
  DrawLargeImageMask(tex,sx,sy,sw,sh,dx,dy,dw,dh,tex->mask);
  }
*/

void DrawImageMask(image_p tex,float sx,float sy,float sw,float sh,float dx,float dy,float dw,float dh,int mask)
{
	DrawImageRot(tex,sx,sy,sw,sh,dx,dy,dw,dh,0,mask);
}


void DrawImage(image_p tex,float sx,float sy,float sw,float sh,float dx,float dy,float dw,float dh)
{
	DrawImageRot(tex,sx,sy,sw,sh,dx,dy,dw,dh,0,tex->mask);
}


void RenderQuad(image_p texture,float sx,float sy,float sw,float sh,float dx,float dy,float xscale ,float yscale,float angle,int mask)
{
	static uint8 r,g,b,a;
	static int ret = 0,cacheid= 0;
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(dx+texture->rcentrex*xscale,dy+texture->rcentrey*yscale,0);
	glRotatef(angle,0,0,1);
	glTranslatef(-(dx+texture->rcentrex*xscale),-(dy+texture->rcentrey*yscale),0);

	ret = tex_cache_getid(texture->texid,&cacheid);
	glBindTexture(GL_TEXTURE_2D, cacheid);
	if(ret == 0 ||texture->modified==1){
		TexImage2D(texture);
		texture->modified = 0;
	}

	if(sw==0&&sh==0){
		glPushAttrib(GL_CURRENT_BIT);
		GetRGBA(mask,texture->dtype,&r,&g,&b,&a);
		glColor4ub(r, g, b, a);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(dx, dy, 0);	// Bottom Left Of The Texture and Quad
		glTexCoord2f(1.0f, 0.0f); glVertex3f( dx+texture->texw*xscale,dy,0);	// Bottom Right Of The Texture and Quad
		glTexCoord2f(1.0f, 1.0f); glVertex3f( dx+texture->texw*xscale,dy+texture->texh*yscale,0);	// Top Right Of The Texture and Quad
		glTexCoord2f(0.0f, 1.0f); glVertex3f(dx,dy+texture->texh*yscale,0);	// Top Left Of The Texture and Quad
		glEnd();
		glPopAttrib();
	}
	else{
		glPushAttrib(GL_CURRENT_BIT);
		GetRGBA(mask,texture->dtype,&r,&g,&b,&a);
		glColor4ub(r, g, b, a);
		glBegin(GL_QUADS);
		glTexCoord2f(sx/texture->texw, sy/texture->texh); glVertex3f(dx, dy, 0);	// Bottom Left Of The Texture and Quad
		glTexCoord2f((sx+sw)/texture->texw, sy/texture->texh); glVertex3f( dx+sw*xscale,dy,0);	// Bottom Right Of The Texture and Quad
		glTexCoord2f((sx+sw)/texture->texw, (sy+sh)/texture->texh); glVertex3f( dx+sw*xscale,dy+sh*yscale,0);	// Top Right Of The Texture and Quad
		glTexCoord2f(sx/texture->texw, (sy+sh)/texture->texh); glVertex3f(dx,dy+sh*yscale,0);	// Top Left Of The Texture and Quad
		glEnd();
		glPopAttrib();
	}
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void ImageToScreen(image_p texture,float dx,float dy)
{
	DrawImage(texture,0,0,texture->w,texture->h,dx,dy,0,0);
}


void DrawImageMaskTrans(image_p tex,float sx,float sy,float sw,float sh,float dx,float dy,float dw,float dh,int mask,int trans)
{
	DrawImageRotTrans(tex,sx,sy,sw,sh,dx,dy,dw,dh,0,mask,trans);
}


void DrawImageTrans(image_p tex,float sx,float sy,float sw,float sh,float dx,float dy,float dw,float dh,int trans)
{
	DrawImageRotTrans(tex,sx,sy,sw,sh,dx,dy,dw,dh,0,tex->mask,trans);
}


void RenderQuadTrans(image_p texture,float sx,float sy,float sw,float sh,float dx,float dy,float xscale ,float yscale,float angle,int mask,int trans )
{
	static uint8 r,g,b,a;
	static int ret = 0,cacheid= 0;
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(dx+texture->rcentrex*xscale,dy+texture->rcentrey*yscale,0);
	glRotatef(angle,0,0,1);
	glTranslatef(-(dx+texture->rcentrex*xscale),-(dy+texture->rcentrey*yscale),0);

	ret = tex_cache_getid(texture->texid,&cacheid);
	glBindTexture(GL_TEXTURE_2D, cacheid);
	if(ret == 0 ||texture->modified==1){
		TexImage2D(texture);
		texture->modified = 0;
	}

	if(sw==0&&sh==0){
		sw = texture->w;
		sh = texture->h;
	}

	glPushAttrib(GL_CURRENT_BIT);
	GetRGBA(mask,texture->dtype,&r,&g,&b,&a);
	glColor4ub(r, g, b, a);
	glBegin(GL_QUADS);

	switch(trans)
	{
	case NGE_TRANS_NONE:
		glTexCoord2f(sx/texture->texw, sy/texture->texh); glVertex3f(dx, dy, 0);	// Bottom Left Of The Texture and Quad
		glTexCoord2f((sx+sw)/texture->texw, sy/texture->texh); glVertex3f( dx+sw*xscale,dy,0);	// Bottom Right Of The Texture and Quad
		glTexCoord2f((sx+sw)/texture->texw, (sy+sh)/texture->texh); glVertex3f( dx+sw*xscale,dy+sh*yscale,0);	// Top Right Of The Texture and Quad
		glTexCoord2f(sx/texture->texw, (sy+sh)/texture->texh); glVertex3f(dx,dy+sh*yscale,0);	// Top Left Of The Texture and Quad
		break;
	case NGE_TRANS_V:
		glTexCoord2f(sx/texture->texw, sy/texture->texh); glVertex3f(dx, dy+sh*yscale, 0);	// Bottom Left Of The Texture and Quad
		glTexCoord2f((sx+sw)/texture->texw, sy/texture->texh); glVertex3f( dx+sw*xscale,dy+sh*yscale,0);	// Bottom Right Of The Texture and Quad
		glTexCoord2f((sx+sw)/texture->texw, (sy+sh)/texture->texh); glVertex3f( dx+sw*xscale,dy,0);	// Top Right Of The Texture and Quad
		glTexCoord2f(sx/texture->texw, (sy+sh)/texture->texh); glVertex3f(dx,dy,0);	// Top Left Of The Texture and Quad
		break;
	case NGE_TRANS_H:
		glTexCoord2f((sx)/texture->w, sy/texture->texh); glVertex3f(dx-(texture->texw-texture->w), dy, 0);	// Bottom Left Of The Texture and Quad
		glTexCoord2f((-1)*(sx+sw)/texture->w,sy/texture->texh); glVertex3f( dx+sw*xscale,dy,0);	// Bottom Right Of The Texture and Quad
		glTexCoord2f((-1)*(sx+sw)/texture->w, (sy+sh)/texture->texh); glVertex3f( dx+sw*xscale,dy+sh*yscale,0);	// Top Right Of The Texture and Quad
		glTexCoord2f((sx)/texture->w, (sy+sh)/texture->texh); glVertex3f(dx-(texture->texw-texture->w),dy+sh*yscale,0);	// Top Left Of The Texture and Quad
		break;
	case NGE_TRANS_HV:
		glTexCoord2f((sx)/texture->w, sy/texture->texh); glVertex3f(dx-(texture->texw-texture->w), dy+sh*yscale, 0);	// Bottom Left Of The Texture and Quad
		glTexCoord2f((-1)*(sx+sw)/texture->w,sy/texture->texh); glVertex3f( dx+sw*xscale,dy+sh*yscale,0);	// Bottom Right Of The Texture and Quad
		glTexCoord2f((-1)*(sx+sw)/texture->w, (sy+sh)/texture->texh); glVertex3f( dx+sw*xscale,dy,0);	// Top Right Of The Texture and Quad
		glTexCoord2f((sx)/texture->w, (sy+sh)/texture->texh); glVertex3f(dx-(texture->texw-texture->w),dy,0);	// Top Left Of The Texture and Quad
		break;
	default:
		glTexCoord2f(sx/texture->texw, sy/texture->texh); glVertex3f(dx, dy, 0);	// Bottom Left Of The Texture and Quad
		glTexCoord2f((sx+sw)/texture->texw, sy/texture->texh); glVertex3f( dx+sw*xscale,dy,0);	// Bottom Right Of The Texture and Quad
		glTexCoord2f((sx+sw)/texture->texw, (sy+sh)/texture->texh); glVertex3f( dx+sw*xscale,dy+sh*yscale,0);	// Top Right Of The Texture and Quad
		glTexCoord2f(sx/texture->texw, (sy+sh)/texture->texh); glVertex3f(dx,dy+sh*yscale,0);	// Top Left Of The Texture and Quad
		break;
	}
	glEnd();
	glPopAttrib();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void ImageToScreenTrans(image_p texture,float dx,float dy,int trans)
{
	DrawImageTrans(texture,0,0,texture->w,texture->h,dx,dy,0,0,trans);
}

static int roundpower2(int width)
{
	int b = width;
	int n;
	for (n = 0; b != 0; n++) b >>= 1;
	b = 1 << n;
	if (b == 2 * width) b >>= 1;
	return b;
}

image_p ScreenToImage()
{
	image_p pimage = image_create(nge_screen.width,nge_screen.height,DISPLAY_PIXEL_FORMAT_8888);
	if(pimage == NULL)
		return NULL;
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glReadPixels( 0, 0, roundpower2(nge_screen.width), roundpower2(nge_screen.height), GL_RGBA, GL_UNSIGNED_BYTE, pimage->data );
	image_flipv(pimage);
	return pimage;
}

void ScreenShot(const char* filename)
{
	image_p pimage = ScreenToImage();
	if(filename == NULL||pimage == NULL)
		return;
	image_save_png(pimage,filename,1);
	image_free(pimage);
}
#endif
