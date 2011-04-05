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
#include "nge_graphics.h"
#include "nge_timer.h"
#include "nge_misc.h"
#include "nge_tex_cache.h"
#include <math.h>

#if defined WIN32 || defined __linux__
#include <GL/glut.h>
#include <GL/gl.h>
#include <SDL.h>
#elif defined IPHONEOS
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
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
    0
};

typedef struct
{
    GLfloat x,y;
}Vectice2D_t;

typedef struct
{
    GLubyte r,g,b,a;
}Color_t;

typedef struct
{
    GLfloat u,v;
}TexCoord_t;

static Vectice2D_t *vectices;
static Color_t *colors;
static TexCoord_t *tex_uvs;

static uint32 max_vectices=0,max_colors=0,max_tex_uvs=0;

static void check_GL_arrays(int size, uint8 use_color, uint8 use_tex_coord)
{
    if(size >= max_vectices){
        max_vectices = size;
        SAFE_FREE(vectices);
        vectices = (Vectice2D_t*)malloc(size*sizeof(Vectice2D_t));
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(2,GL_FLOAT,sizeof(Vectice2D_t),vectices);
    }
    if(use_color)
    {
        glEnableClientState(GL_COLOR_ARRAY);
        if(size>max_colors)
        {
            max_colors = size;
            SAFE_FREE(colors);
            colors = (Color_t*)malloc(size*sizeof(Color_t));
            glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Color_t), colors);
        }
    }
    else
        glDisableClientState(GL_COLOR_ARRAY);

    if(use_tex_coord)
    {
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        if(size>max_tex_uvs)
        {
            max_tex_uvs = size;
            SAFE_FREE(tex_uvs);
            tex_uvs = (Color_t*)malloc(size*sizeof(TexCoord_t));
            glTexCoordPointer(2,GL_FLOAT,sizeof(TexCoord_t),tex_uvs);
        }
    }
    else
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

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

char* GetVersion()
{
    static char version[] = {
#ifdef IPHONEOS
        "nge2 iphone opengles driver v1.0"
#else
        "nge2 driver v2.0"
#endif
    };
    return version;
}

static float screen_r =0.0, screen_g =0.0, screen_b =0.0, screen_a =1.0;

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
#ifdef IPHONEOS
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

void InitGrahics()
{
    int i = 0;
#if defined(WIN32) || defined(__linux__)
    int screen_flag = 0;
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

    glEnable( GL_TEXTURE_2D );
    glDisable( GL_DEPTH_TEST );
    glShadeModel( GL_SMOOTH );
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    tex_cache_init(MAX_TEX_CACHE_SIZE);
    glGenTextures( MAX_TEX_CACHE_SIZE, &m_texcache[0] );
    for(i=0;i<MAX_TEX_CACHE_SIZE;i++){
        tex_cache_add(i,m_texcache[i]);
        glBindTexture(GL_TEXTURE_2D, m_texcache[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }

    glEnable(GL_SCISSOR_TEST);
    ResetClip();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrthof(0,nge_screen.width,nge_screen.height,0,-1,1);

    glMatrixMode(GL_MODELVIEW);

    // init for fps------------------
    m_frame = 0;
    m_t0 = 0;
    fps_last_ticks = nge_get_ticks();
    //-------------------------------

    for (i=0;i<360;i++)
    {
        m_sintable[i] = sin(i*DEG2RAD);
        m_costable[i] = cos(i*DEG2RAD);
    }
#ifndef NDEBUG
    printf("Init Graphics Ok\n");
#endif
}

void FiniGrahics()
{
    tex_cache_fini();
    glDeleteTextures(MAX_TEX_CACHE_SIZE,m_texcache);
    SAFE_FREE(vectices);
    SAFE_FREE(colors);
    SAFE_FREE(tex_uvs);
}

void SetClip(int x,int y,int w,int h)
{
    glScissor(x,nge_screen.height-y-h,w,h);
}

void ResetClip()
{
    SetClip(0, 0, nge_screen.width, nge_screen.height);
}

void ShowFps()
{
    int t;
    float seconds;

    m_frame++;
    t = nge_get_ticks();
    if ( (t - m_t0) >= 1000) {
        seconds = (t - m_t0) / 1000.0;
        printf("%d frames in %g seconds = %g FPS\n", m_frame, seconds, m_frame / seconds);
        m_t0 = t;
        m_frame = 0;
    }
}

void LimitFps(uint32 limit)
{
    int ticks = 0, sleep_ticks = 0;
    if(limit == 0)
        limit = 60;
    ticks = nge_get_ticks();
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
        glClearColor( screen_r, screen_g, screen_b, screen_a );
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
#ifndef IPHONEOS
    SDL_GL_SwapBuffers();
#endif
}

#define SET_COLOR(color, dtype)                 \
    GetRGBA(color,dtype,&r,&g,&b,&a);           \
    glColor4ub(r, g, b, a)

#define AROUND_DRAW(codeblock)                  \
    static uint8 r,g,b,a;                       \
    glPushAttrib(GL_CURRENT_BIT);               \
    codeblock                                   \
    glPopAttrib()

void DrawLine(float x1, float y1, float x2, float y2, int color,int dtype)
{
    AROUND_DRAW(
        {
            if(y1 == 0.0)
                y1 = 0.1;
            if(y2 == 0.0)
                y2 = 0.1;
            SET_COLOR(color, dtype);
            glBegin(GL_LINES);
            glVertex2f(x1, y1);
            glVertex2f(x2, y2);
            glEnd();
        });
}

void DrawLineEx(pointf p1,pointf p2 ,int color,int dtype)
{
    DrawLine(p1.x, p1.y, p2.x, p2.y,color,dtype);
}

void DrawCircle(float x, float y, float radius, int color,int dtype)
{
    AROUND_DRAW({
            int i;
            check_GL_arrays(360, 0, 0);
            for(i=0; i<360;i++)
            {
                vectices[i].x = x+radius*COSF(i);
                vectices[i].y = y+radius*SINF(i);
            }
            SET_COLOR(color, dtype);
            glDrawArrays(GL_LINE_LOOP, 0, 360);});
}

void FillCircle(float x, float y, float radius, int color,int dtype)
{
    AROUND_DRAW({
            int i;
            check_GL_arrays(360, 0, 0);
            for(i=0; i<360;i++)
            {
                vectices[i].x = x+radius*COSF(i);
                vectices[i].y = y+radius*SINF(i);
            }
            SET_COLOR(color, dtype);
            glDrawArrays(GL_POLYGON, 0, 360);});
}

void DrawEllipse(float x,float y ,float xradius,float yradius,int color,int dtype)
{
    AROUND_DRAW({
            int i;
            check_GL_arrays(360, 0, 0);
            for(i=0; i<360;i++)
            {
                vectices[i].x = x+COSF(i)*xradius;
                vectices[i].y = y+SINF(i)*yradius;
            }
            SET_COLOR(color, dtype);
            glDrawArrays(GL_LINE_LOOP, 0, 360);});
}

void FillEllipse(float x,float y ,float xradius,float yradius,int color,int dtype)
{
    AROUND_DRAW({
            int i;
            check_GL_arrays(360, 0, 0);
            for(i=0; i<360;i++)
            {
                vectices[i].x = x+COSF(i)*xradius;
                vectices[i].y = y+SINF(i)*yradius;
            }
            SET_COLOR(color, dtype);
            glDrawArrays(GL_POLYGON, 0, 360);});
}

void PutPix(float x,float y ,int color,int dtype)
{
    AROUND_DRAW({
            SET_COLOR(color,dtype);
            glBegin(GL_POINTS);
            glVertex2f(x,y);
            glEnd();});
}

#define DRAW_POLYGON_IMP(mode, extracode)       \
    AROUND_DRAW({                               \
            int i;                              \
            check_GL_arrays(count, 0, 0);       \
            for(i=0; i<count;i++)               \
            {                                   \
                vectices[i].x = x[i];           \
                vectices[i].y = y[i];           \
                extracode                       \
            }                                   \
            SET_COLOR(color,dtype);             \
            glDrawArrays(mode, 0, count);})

void DrawPolygon(float* x, float* y, int count, int color,int dtype)
{
    DRAW_POLYGON_IMP(GL_LINE_LOOP, {});
}

void FillPolygon(float* x, float* y, int count, int color,int dtype)
{
    DRAW_POLYGON_IMP(GL_POLYGON, {});
}

void FillPolygonGrad(float* x, float* y, int count, int* colors,int dtype)
{
    DRAW_POLYGON_IMP(GL_POLYGON, {
            GetRGBA(colors[i],dtype,&r,&g,&b,&a);
            colors[i] = { r, g, b, a };
        });
}

void DrawRect(float dx, float dy, float width, float height,int color,int dtype)
{
    if(dy == 0.0)
        dy = 0.1;
    AROUND_DRAW({
            SET_COLOR(color,dtype);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glRectf(dx,dy,dx+width,dy+height);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);});
}

inline void DrawRectEx(rectf rect,int color,int dtype)
{
    DrawRect(rect.top, rect.left, rect.right-rect.left, rect.bottom-rect.top,color,dtype);
}

void FillRect(float dx, float dy, float width, float height,int color,int dtype)
{
    if(dy == 0.0)
        dy = 0.1;
    AROUND_DRAW({
            glRectf(dx,dy,dx+width, dy+height);})
}

inline void FillRectEx(rectf rect,int color,int dtype)
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
inline void FillRectGrad(float dx, float dy, float width, float height,int* colors,int dtype)
{
    float x[4] = { dx, dx, dx+width, dx+width}, y[4] = { dy+height, dy, dy, dy+height};
    FillPolygonGrad(x, y, 4, colors, dtype);
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
    float x[4] = { v1.x, v2.x, v3.x }, y[4] = { v1.y, v2.y, v3.y };
    FillPolygon(x, y, 3, color, dtype);
}

int PreLoadImage(image_p pimg)
{
    return 1;
}

void TexImage2D(image_p pimg)
{
    uint32 format = GL_RGBA;
    if(pimg->dtype == DISPLAY_PIXEL_FORMAT_565){
        format = GL_RGB;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, format, pimg->texw, pimg->texh, 0, format, pimg->dtype, pimg->data);
}

#define BIND_AND_CACHE(tex, id)                     \
    do {                                            \
        int ret = tex_cache_getid(tex->texid,&id);  \
        glBindTexture(GL_TEXTURE_2D, id);           \
        if(ret == 0 ||tex->modified==1){            \
            TexImage2D(tex);                        \
            tex->modified = 0;                      \
        }                                           \
    }while(0)

static void DrawImageRotTrans(image_p texture,float sx,float sy,float sw,float sh,float dx,float dy,float dw,float dh,float angle,int mask,int trans)
{
    static uint8 r,g,b,a;
    static int cacheid = 0;
    glPushMatrix();
    ROTATE_2D(angle, dx+texture->rcentrex,dy+texture->rcentrey);

    BIND_AND_CACHE(texture, cacheid);
    if(sw==0&&sh==0){
        sw = texture->w;
        sh = texture->h;
    }
    if(dw==0&&dh==0){
        dw = sw;//texture->w;
        dh = sh;//texture->h;
    }

    check_GL_arrays(4, 0, 1);
    switch(trans)
    {
    case NGE_TRANS_V:
        tex_uvs[0] = {sx/texture->texw, sy/texture->texh}; vectice[0] = {dx, dy+dh};
        tex_uvs[1] = {(sx+sw)/texture->texw, sy/texture->texh}; vectice[1] = { dx+dw,dy+d};
        tex_uvs[2] = {(sx+sw)/texture->texw, (sy+sh)/texture->texh}; vectice[2] = { dx+dw,d};
        tex_uvs[3] = {sx/texture->texw, (sy+sh)/texture->texh}; vectice[3] = {dx,d};
        break;
    case NGE_TRANS_H:
        tex_uvs[0] = {(sx)/texture->w, sy/texture->texh}; vectice[0] = {dx-(texture->texw-texture->w), dy};
        tex_uvs[1] = {(-1)*(sx+sw)/texture->w,sy/texture->texh}; vectice[1] = { dx+dw,d};
        tex_uvs[2] = {(-1)*(sx+sw)/texture->w, (sy+sh)/texture->texh}; vectice[2] = { dx+dw,dy+d};
        tex_uvs[3] = {(sx)/texture->w, (sy+sh)/texture->texh}; vectice[3] = {dx-(texture->texw-texture->w),dy+d};
        break;
    case NGE_TRANS_HV:
        tex_uvs[0] = {(sx)/texture->w, sy/texture->texh}; vectice[0] = {dx-(texture->texw-texture->w), dy+dh};
        tex_uvs[1] = {(-1)*(sx+sw)/texture->w,sy/texture->texh}; vectice[1] = { dx+dw,dy+d};
        tex_uvs[2] = {(-1)*(sx+sw)/texture->w, (sy+sh)/texture->texh}; vectice[2] = { dx+dw,d};
        tex_uvs[3] = {(sx)/texture->w, (sy+sh)/texture->texh}; vectice[3] = {dx-(texture->texw-texture->w),d};
        break;
        //case NGE_TRANS_NONE:
    default:
        tex_uvs[0] = { sx/texture->texw, sy/texture->texh}; vectices[0] = { dx,dy};
        tex_uvs[1] = { (sx+sw)/texture->texw, sy/texture->texh}; vectices[1] = { dx+dw,dy};
        tex_uvs[2] = { (sx+sw)/texture->texw, (sy+sh)/texture->texh}; vectices[2] = { dx+dw,dy+dh};
        tex_uvs[3] = { sx/texture->texw, (sy+sh)/texture->texh}; vectices[3] = { dx,dy+dh};
        break;
    }
    SET_COLOR(mask,texture->dtype);
    glDrawArrays(GL_POLYGON, 0, 4);
    glPopMatrix();
}

static inline void DrawImageRot(image_p texture,float sx,float sy,float sw,float sh,float dx,float dy,float dw,float dh,float angle,int mask)
{
    DrawImageRotTrans(texture,sx,sy,sw,sh, dx, dy, dw, dh, angle, mask, NGE_TRANS_NONE);
}

inline void DrawImageMask(image_p tex,float sx,float sy,float sw,float sh,float dx,float dy,float dw,float dh,int mask)
{
    DrawImageRot(tex,sx,sy,sw,sh,dx,dy,dw,dh,0,mask);
}


inline void DrawImage(image_p tex,float sx,float sy,float sw,float sh,float dx,float dy,float dw,float dh)
{
    DrawImageRot(tex,sx,sy,sw,sh,dx,dy,dw,dh,0,tex->mask);
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

void RenderQuad(image_p texture,float sx,float sy,float sw,float sh,float dx,float dy,float xscale ,float yscale,float angle,int mask)
{
    static uint8 r,g,b,a;
    static cacheid= 0;
    if(dy == 0.0f)
        dy = 0.1f;
    glPushMatrix();
    ROTATE_2D(angle, dx+texture->rcentrex*xscale,dy+texture->rcentrey*yscale);

    BIND_AND_CACHE(texture, cacheid);
    check_GL_arrays(4, 0, 1);
    if(sw==0&&sh==0){
        tex_uvs[0] = {0.0f, 0.0f}; vectice[0] = {dx, dy};
        tex_uvs[1] = {1.0f, 0.0f}; vectice[1] = { dx+texture->texw*xscale,dy};
        tex_uvs[2] = {1.0f, 1.0f}; vectice[2] = { dx+texture->texw*xscale,dy+texture->texh*yscale};
        tex_uvs[3] = {0.0f, 1.0f}; vectice[3] = {dx,dy+texture->texh*yscale};
    }
    else{
        tex_uvs[0] = {sx/texture->texw, sy/texture->texh}; vectice[0] = {dx, dy};
        tex_uvs[1] = {(sx+sw)/texture->texw, sy/texture->texh}; vectice[1] = { dx+sw*xscale,dy};
        tex_uvs[2] = {(sx+sw)/texture->texw, (sy+sh)/texture->texh}; vectice[2] = { dx+sw*xscale,dy+sh*yscale};
        tex_uvs[3] = {sx/texture->texw, (sy+sh)/texture->texh}; vectice[3] = {dx,dy+sh*yscale};
    }
    SET_COLOR(mask, texture->dtype);
    glDrawArrays(GL_POLYGON, 0, 4);
    glPopMatrix();
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
    static cacheid= 0;
    if(dy == 0.0f)
        dy = 0.1f;
    glPushMatrix();
    ROTATE_2D(angle, dx+texture->rcentrex*xscale,dy+texture->rcentrey*yscale);

    BIND_AND_CACHE(texture, cacheid);
    check_GL_arrays(4, 0, 1);
    if(sw==0&&sh==0){
        sw = texture->w;
        sh = texture->h;
    }

    switch(trans)
    {
    case NGE_TRANS_V:
        tex_uvs[0] = {sx/texture->texw, sy/texture->texh}; vectice[0] = {dx, dy+sh*yscale};
        tex_uvs[1] = {(sx+sw)/texture->texw, sy/texture->texh}; vectice[1] = { dx+sw*xscale,dy+sh*yscale};
        tex_uvs[2] = {(sx+sw)/texture->texw, (sy+sh)/texture->texh}; vectice[2] = { dx+sw*xscale,dy};
        tex_uvs[3] = {sx/texture->texw, (sy+sh)/texture->texh}; vectice[3] = {dx,dy};
        break;
    case NGE_TRANS_H:
        tex_uvs[0] = {(sx)/texture->w, sy/texture->texh}; vectice[0] = {dx-(texture->texw-texture->w), dy};
        tex_uvs[1] = {(-1)*(sx+sw)/texture->w,sy/texture->texh}; vectice[1] = { dx+sw*xscale,dy};
        tex_uvs[2] = {(-1)*(sx+sw)/texture->w, (sy+sh)/texture->texh}; vectice[2] = { dx+sw*xscale,dy+sh*yscale};
        tex_uvs[3] = {(sx)/texture->w, (sy+sh)/texture->texh}; vectice[3] = {dx-(texture->texw-texture->w),dy+sh*yscale};
        break;
    case NGE_TRANS_HV:
        tex_uvs[0] = {(sx)/texture->w, sy/texture->texh}; vectice[0] = {dx-(texture->texw-texture->w), dy+sh*yscale};
        tex_uvs[1] = {(-1)*(sx+sw)/texture->w,sy/texture->texh}; vectice[1] = { dx+sw*xscale,dy+sh*yscale};
        tex_uvs[2] = {(-1)*(sx+sw)/texture->w, (sy+sh)/texture->texh}; vectice[2] = { dx+sw*xscale,dy};
        tex_uvs[3] = {(sx)/texture->w, (sy+sh)/texture->texh}; vectice[3] = {dx-(texture->texw-texture->w),dy};
        break;
//    case NGE_TRANS_NONE:
    default:
        tex_uvs[0] = {sx/texture->texw, sy/texture->texh}; vectice[0] = {dx, dy};
        tex_uvs[1] = {(sx+sw)/texture->texw, sy/texture->texh}; vectice[1] = { dx+sw*xscale,dy};
        tex_uvs[2] = {(sx+sw)/texture->texw, (sy+sh)/texture->texh}; vectice[2] = { dx+sw*xscale,dy+sh*yscale};
        tex_uvs[3] = {sx/texture->texw, (sy+sh)/texture->texh}; vectice[3] = {dx,dy+sh*yscale};
        break;
    }
    SET_COLOR(mask, texture->dtype);
    glDrawArrays(GL_POLYGON, 0, 4);
    glPopMatrix();
}

void ImageToScreenTrans(image_p texture,float dx,float dy,int trans)
{
    DrawImageTrans(texture,0,0,texture->w,texture->h,dx,dy,0,0,trans);
}

image_p ScreenToImage()
{
    image_p pimage = image_create(nge_screen.width,nge_screen.height,DISPLAY_PIXEL_FORMAT_8888);
    if(pimage == NULL)
        return NULL;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glReadPixels( 0, 0, TEXTRUE_MAX_WIDTH, TEXTRUE_MAX_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, pimage->data );
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
