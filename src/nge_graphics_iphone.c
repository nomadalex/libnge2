#include <SDL.h>
#include "SDL_opengles.h"
#include <math.h>
#include "nge_graphics.h"
#include "nge_image_load.h"
#include "nge_timer.h"
#include "nge_misc.h"
#include "nge_tex_cache.h"

//fps count
static uint32 m_frame = 0;
static uint32 m_t0 = 0;

static uint32 m_tex_in_ram = -1;
//static uint32 m_tex_id = 0;
static nge_timer* timer = NULL;

static float screen_r =0.0;
static float screen_g =0.0;
static float screen_b =0.0;
static float screen_a =1.0;

static screen_context_t nge_screen = {
	"NGE2",
	SCREEN_WIDTH_IPHONE,
	SCREEN_HEIGHT_IPHONE,
	SCREEN_BPP,
	0
};

static float m_sintable[360];
static float m_costable[360];
#define RAD2DEG		57.29577951f
#define DEG2RAD		0.017453293f
#define SINF(a)  (m_sintable[a%360])
#define COSF(a)  (m_costable[a%360])

typedef struct taguvCoord{
	float u;
	float v;
}uvCoord;
typedef struct vectice2D{
	float x;
	float y;
	uint8 color[4];
	float uv[2];
}Vectice2D;

char* VecticesArray;
int max_vectice = 65535;
#define MAX_TEX_CACHE_SIZE 32

unsigned int m_texcache[MAX_TEX_CACHE_SIZE];



static void* NgeGetMemoryVectice(int size)
{
	if(size >= max_vectice){
		max_vectice	*= 2;
		if(max_vectice < size){
			max_vectice = size*2;
		}
		SAFE_FREE(VecticesArray);
		VecticesArray = (char*)malloc(max_vectice);
	}
	return VecticesArray;						   
	
}


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

char* GetVersion()
{
	static char version[] = {"nge2 iphone opengles driver v1.0"}; 
	return version;
}

screen_context_p GetScreenContext()
{
	return &nge_screen;
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
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrthof(0,nge_screen.width,nge_screen.height-20,-20,-1,1);
	//glOrthof(0,SCREEN_HEIGHT,SCREEN_WIDTH,0,-1,1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	VecticesArray = (char*)malloc(max_vectice);
}
void FiniGrahics()
{
	timer_free(timer);
	tex_cache_fini();
	glDeleteTextures(MAX_TEX_CACHE_SIZE,m_texcache);
	SAFE_FREE(VecticesArray);
}

void BeginScene(uint8 clear)
{
	if(clear == 1){
		glDisable(GL_SCISSOR_TEST);
		glClearColor( screen_r, screen_g, screen_b, screen_a );
		glClear( GL_COLOR_BUFFER_BIT );
		glEnable(GL_SCISSOR_TEST);
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


void EndScene()
{
	SDL_RenderPresent();
}

void FpsInit()
{
	m_frame = 0;
	m_t0 = 0;
}

void SetClip(int x,int y,int w,int h)
{
	glScissor(x,nge_screen.height-y-h,w,h);
}

void ResetClip()
{
	SetClip(0, 0, nge_screen.width,nge_screen.height);
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
	timer->start(timer);
	if( timer->get_ticks(timer) < 1000 /limit )
    {
            nge_sleep( ( 1000 / limit) - timer->get_ticks(timer) );
    }
}

void DrawLine(float x1, float y1, float x2, float y2, int color,int dtype)
{
	static uint8 r,g,b,a;	
	Vectice2D* vectices = (Vectice2D*)NgeGetMemoryVectice(2*sizeof(Vectice2D));
	GetRGBA(color,dtype,&r,&g,&b,&a);
	vectices[0].x = x1;
	vectices[0].y = y1;
	vectices[0].color[0] = r;
	vectices[0].color[1] = g;
	vectices[0].color[2] = b;
	vectices[0].color[3] = a;	
	vectices[1].x = x2;
	vectices[1].y = y2;
	vectices[1].color[0] = r;
	vectices[1].color[1] = g;
	vectices[1].color[2] = b;
	vectices[1].color[3] = a;	
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vectice2D),
                   vectices[0].color);	
	glVertexPointer(2,GL_FLOAT,sizeof(Vectice2D),vectices);
	glEnableClientState(GL_VERTEX_ARRAY);
	glDrawArrays(GL_LINES, 0, 2);	
}

void DrawLineEx(pointf p1,pointf p2 ,int color,int dtype)
{
	DrawLine(p1.x, p1.y, p2.x, p2.y,color,dtype);
}

void DrawCircle(float x, float y, float radius, int color,int dtype)
{
	static uint8 r,g,b,a;
	int i,angle = 359;
	Vectice2D* vectices = (Vectice2D*)NgeGetMemoryVectice(181*sizeof(Vectice2D));
	GetRGBA(color,dtype,&r,&g,&b,&a);
	for(i=0; i<180; i++)
	{
		vectices[i].color[0] = r;
		vectices[i].color[1] = g;
		vectices[i].color[2] = b;
		vectices[i].color[3] = a;	
		vectices[i].x = x+radius*COSF(angle);
		vectices[i].y = y+radius*SINF(angle);
		angle -= 2;
		if (angle < 0)
			angle = 0;
	}
	
	vectices[180].color[0] = r;
	vectices[180].color[1] = g;
	vectices[180].color[2] = b;
	vectices[180].color[3] = a;	
	vectices[180].x = x+radius*COSF(0);
	vectices[180].y = y+radius*SINF(0);
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vectice2D),
                   vectices[0].color);	
	glVertexPointer(2,GL_FLOAT,sizeof(Vectice2D),vectices);
	glEnableClientState(GL_VERTEX_ARRAY);
	glDrawArrays(GL_LINE_STRIP, 0, 181);	
}

void FillCircle(float x, float y, float radius, int color,int dtype)
{
	static uint8 r,g,b,a;
	int i,angle = 359;
	Vectice2D* vectices = (Vectice2D*)NgeGetMemoryVectice(182*sizeof(Vectice2D));
	GetRGBA(color,dtype,&r,&g,&b,&a);
	vectices[0].color[0] = r;
	vectices[0].color[1] = g;
	vectices[0].color[2] = b;
	vectices[0].color[3] = a;	
	vectices[0].x = x;
	vectices[0].y = y;

	
	for(i=0; i<180; i++)
	{
		vectices[i+1].color[0] = r;
		vectices[i+1].color[1] = g;
		vectices[i+1].color[2] = b;
		vectices[i+1].color[3] = a;	
		vectices[i+1].x = x+radius*COSF(angle);
		vectices[i+1].y = y+radius*SINF(angle);

		angle -= 2;
		if (angle < 0)
			angle = 0;
	}
	
	vectices[181].color[0] = r;
	vectices[181].color[1] = g;
	vectices[181].color[2] = b;
	vectices[181].color[3] = a;	
	vectices[181].x = x+radius*COSF(359);
	vectices[181].y = y+radius*SINF(359);
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vectice2D),
                   vectices[0].color);	
	glVertexPointer(2,GL_FLOAT,sizeof(Vectice2D),vectices);
	glEnableClientState(GL_VERTEX_ARRAY);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 182);	
}

void DrawEllipse(float x,float y ,float xradius,float yradius,int color,int dtype)
{
	static uint8 r,g,b,a;
	int i,angle = 359;
	Vectice2D* vectices = (Vectice2D*)NgeGetMemoryVectice(181*sizeof(Vectice2D));
	GetRGBA(color,dtype,&r,&g,&b,&a);
	for(i=0; i<180; i++)
	{
		vectices[i].color[0] = r;
		vectices[i].color[1] = g;
		vectices[i].color[2] = b;
		vectices[i].color[3] = a;	
		vectices[i].x = x+COSF(angle)*xradius;
		vectices[i].y = y+SINF(angle)*yradius;
		angle -= 2;
		if (angle < 0)
			angle = 0;
	}
	
	vectices[180].color[0] = r;
	vectices[180].color[1] = g;
	vectices[180].color[2] = b;
	vectices[180].color[3] = a;	
	vectices[180].x = x+xradius*COSF(0);
	vectices[180].y = y+yradius*SINF(0);
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vectice2D),
                   vectices[0].color);	
	glVertexPointer(2,GL_FLOAT,sizeof(Vectice2D),vectices);
	glEnableClientState(GL_VERTEX_ARRAY);
	glDrawArrays(GL_LINE_STRIP, 0, 181);	
}

void FillEllipse(float x,float y ,float xradius,float yradius,int color,int dtype)
{
	static uint8 r,g,b,a;
	int i,angle = 359;
	Vectice2D* vectices = (Vectice2D*)NgeGetMemoryVectice(182*sizeof(Vectice2D));
	GetRGBA(color,dtype,&r,&g,&b,&a);
	vectices[0].color[0] = r;
	vectices[0].color[1] = g;
	vectices[0].color[2] = b;
	vectices[0].color[3] = a;	
	vectices[0].x = x;
	vectices[0].y = y;
	
	
	for(i=0; i<180; i++)
	{
		vectices[i+1].color[0] = r;
		vectices[i+1].color[1] = g;
		vectices[i+1].color[2] = b;
		vectices[i+1].color[3] = a;	
		vectices[i+1].x = x+COSF(angle)*xradius;
		vectices[i+1].y = y+SINF(angle)*yradius;
		
		angle -= 2;
		if (angle < 0)
			angle = 0;
	}
	
	vectices[181].color[0] = r;
	vectices[181].color[1] = g;
	vectices[181].color[2] = b;
	vectices[181].color[3] = a;	
	vectices[181].x = x+xradius*COSF(359);
	vectices[181].y = y+yradius*SINF(359);
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vectice2D),
                   vectices[0].color);	
	glVertexPointer(2,GL_FLOAT,sizeof(Vectice2D),vectices);
	glEnableClientState(GL_VERTEX_ARRAY);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 182);	
}

void PutPix(float x,float y ,int color,int dtype)
{
	static uint8 r,g,b,a;	
	Vectice2D* vectices = (Vectice2D*)NgeGetMemoryVectice(1*sizeof(Vectice2D));
	GetRGBA(color,dtype,&r,&g,&b,&a);
	vectices[0].x = x;
	vectices[0].y = y;
	vectices[0].color[0] = r;
	vectices[0].color[1] = g;
	vectices[0].color[2] = b;
	vectices[0].color[3] = a;	
	
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vectice2D),
                   vectices[0].color);	
	glVertexPointer(2,GL_FLOAT,sizeof(Vectice2D),vectices);
	glEnableClientState(GL_VERTEX_ARRAY);
	glDrawArrays(GL_POINTS, 0, 1);
}

void DrawPolygon(float* x, float* y, int count, int color,int dtype)
{
	static uint8 r,g,b,a,i;	
	Vectice2D* vectices = (Vectice2D*)NgeGetMemoryVectice(count*sizeof(Vectice2D));
	GetRGBA(color,dtype,&r,&g,&b,&a);
	for(i = 0;i<count;i++){
		vectices[i].x = x[i];
		vectices[i].y = y[i];
		vectices[i].color[0] = r;
		vectices[i].color[1] = g;
		vectices[i].color[2] = b;
		vectices[i].color[3] = a;
	}

	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vectice2D),
                   vectices[0].color);	
	glVertexPointer(2,GL_FLOAT,sizeof(Vectice2D),vectices);
	glEnableClientState(GL_VERTEX_ARRAY);
	glDrawArrays(GL_LINE_STRIP, 0, count);
}

void FillPolygon(float* x, float* y, int count, int color,int dtype)
{
	static uint8 r,g,b,a,i;	
	Vectice2D* vectices = (Vectice2D*)NgeGetMemoryVectice(count*sizeof(Vectice2D));
	GetRGBA(color,dtype,&r,&g,&b,&a);
	for(i = 0;i<count;i++){
		vectices[i].x = x[i];
		vectices[i].y = y[i];
		vectices[i].color[0] = r;
		vectices[i].color[1] = g;
		vectices[i].color[2] = b;
		vectices[i].color[3] = a;
	}

	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vectice2D),
                   vectices[0].color);	
	glVertexPointer(2,GL_FLOAT,sizeof(Vectice2D),vectices);
	glEnableClientState(GL_VERTEX_ARRAY);
	glDrawArrays(GL_TRIANGLE_FAN, 0, count);
}

void FillPolygonGrad(float* x, float* y, int count, int* colors,int dtype)
{
	static uint8 r,g,b,a,i;	
	Vectice2D* vectices = (Vectice2D*)NgeGetMemoryVectice(count*sizeof(Vectice2D));
	
	for(i = 0;i<count;i++){
		vectices[i].x = x[i];
		vectices[i].y = y[i];
		GetRGBA(colors[i],dtype,&r,&g,&b,&a);
		vectices[i].color[0] = r;
		vectices[i].color[1] = g;
		vectices[i].color[2] = b;
		vectices[i].color[3] = a;
	}
	
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vectice2D),
                   vectices[0].color);	
	glVertexPointer(2,GL_FLOAT,sizeof(Vectice2D),vectices);
	glEnableClientState(GL_VERTEX_ARRAY);
	glDrawArrays(GL_TRIANGLE_FAN, 0, count);
}

void DrawRect(float x, float y, float width, float height,int color,int dtype)
{
	static uint8 r,g,b,a;	
	Vectice2D* vectices = (Vectice2D*)NgeGetMemoryVectice(5*sizeof(Vectice2D));
	GetRGBA(color,dtype,&r,&g,&b,&a);
	if(y == 0.0f)
		y = 0.1f;
	vectices[0].x = x;
	vectices[0].y = y;
	vectices[0].color[0] = r;
	vectices[0].color[1] = g;
	vectices[0].color[2] = b;
	vectices[0].color[3] = a;	
	vectices[1].x = x ;
	vectices[1].y = y +height;
	vectices[1].color[0] = r;
	vectices[1].color[1] = g;
	vectices[1].color[2] = b;
	vectices[1].color[3] = a;
	vectices[2].x = x + width;
	vectices[2].y = y + height;
	vectices[2].color[0] = r;
	vectices[2].color[1] = g;
	vectices[2].color[2] = b;
	vectices[2].color[3] = a;	
	vectices[3].x = x + width;
	vectices[3].y = y ;
	vectices[3].color[0] = r;
	vectices[3].color[1] = g;
	vectices[3].color[2] = b;
	vectices[3].color[3] = a;
	vectices[4].x = x ;
	vectices[4].y = y ;
	vectices[4].color[0] = r;
	vectices[4].color[1] = g;
	vectices[4].color[2] = b;
	vectices[4].color[3] = a;	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vectice2D),
                   vectices[0].color);	
	glVertexPointer(2,GL_FLOAT,sizeof(Vectice2D),vectices);
	glEnableClientState(GL_VERTEX_ARRAY);
	glDrawArrays(GL_LINE_STRIP, 0, 5);		
}

void DrawRectEx(rectf rect,int color,int dtype)
{
	DrawRect(rect.top, rect.left, rect.right-rect.left, rect.bottom-rect.top,color,dtype);
}

void FillRect(float x, float y, float width, float height,int color,int dtype)
{
	static uint8 r,g,b,a;	
	Vectice2D* vectices = (Vectice2D*)NgeGetMemoryVectice(4*sizeof(Vectice2D));
	GetRGBA(color,dtype,&r,&g,&b,&a);
	if(y == 0.0f)
		y = 0.1f;
	vectices[0].x = x;
	vectices[0].y = y;
	vectices[0].color[0] = r;
	vectices[0].color[1] = g;
	vectices[0].color[2] = b;
	vectices[0].color[3] = a;	
	vectices[1].x = x ;
	vectices[1].y = y +height;
	vectices[1].color[0] = r;
	vectices[1].color[1] = g;
	vectices[1].color[2] = b;
	vectices[1].color[3] = a;
	vectices[3].x = x + width;
	vectices[3].y = y + height;
	vectices[3].color[0] = r;
	vectices[3].color[1] = g;
	vectices[3].color[2] = b;
	vectices[3].color[3] = a;	
	vectices[2].x = x + width;
	vectices[2].y = y ;
	vectices[2].color[0] = r;
	vectices[2].color[1] = g;
	vectices[2].color[2] = b;
	vectices[2].color[3] = a;
		
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vectice2D),
                   vectices[0].color);	
	glVertexPointer(2,GL_FLOAT,sizeof(Vectice2D),vectices);
	glEnableClientState(GL_VERTEX_ARRAY);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);	
}

void FillRectEx(rectf rect,int color,int dtype)
{
	FillRect(rect.top, rect.left, rect.right-rect.left, rect.bottom-rect.top,color,dtype);
}

void FillRectGrad(float x, float y, float width, float height,int* colors,int dtype)
{
	static uint8 r,g,b,a;	
	Vectice2D* vectices = (Vectice2D*)NgeGetMemoryVectice(4*sizeof(Vectice2D));
	if(y == 0.0f)
		y = 0.1f;
	vectices[0].x = x;
	vectices[0].y = y;
	GetRGBA(colors[0],dtype,&r,&g,&b,&a);	
	vectices[0].color[0] = r;
	vectices[0].color[1] = g;
	vectices[0].color[2] = b;
	vectices[0].color[3] = a;	
	vectices[1].x = x ;
	vectices[1].y = y +height;
	GetRGBA(colors[1],dtype,&r,&g,&b,&a);	
	vectices[1].color[0] = r;
	vectices[1].color[1] = g;
	vectices[1].color[2] = b;
	vectices[1].color[3] = a;
	vectices[3].x = x + width;
	vectices[3].y = y + height;
	GetRGBA(colors[2],dtype,&r,&g,&b,&a);
	vectices[3].color[0] = r;
	vectices[3].color[1] = g;
	vectices[3].color[2] = b;
	vectices[3].color[3] = a;	
	vectices[2].x = x + width;
	vectices[2].y = y ;
	GetRGBA(colors[3],dtype,&r,&g,&b,&a);
	vectices[2].color[0] = r;
	vectices[2].color[1] = g;
	vectices[2].color[2] = b;
	vectices[2].color[3] = a;
	
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vectice2D),
                   vectices[0].color);	
	glVertexPointer(2,GL_FLOAT,sizeof(Vectice2D),vectices);
	glEnableClientState(GL_VERTEX_ARRAY);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);		
}

void FillRectGradEx(rectf rect,int* colors,int dtype)
{
	FillRectGrad(rect.top, rect.left, rect.right-rect.left, rect.bottom-rect.top,colors,dtype);
}

void FillTri(pointf v0,pointf v2,pointf v3 ,int color,int dtype)
{
}

int PreLoadImage(image_p pimg)
{
	return 1;
}
/*
int TexImage2D(image_p pimg)
{
	uint32 format = GL_RGBA;
	m_tex_in_ram = pimg->texid;
	if(pimg->dtype == DISPLAY_PIXEL_FORMAT_565){
		format = GL_RGB;
	}
	glEnable(GL_TEXTURE_2D);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pimg->texw, pimg->texh, 0,GL_RGBA, GL_UNSIGNED_BYTE, pimg->data);
	return 1;
}*/

int TexImage2D(image_p pimg)
{
	uint32 format = GL_RGBA;
	m_tex_in_ram = pimg->texid;
	if(pimg->dtype == DISPLAY_PIXEL_FORMAT_565){
			  format = GL_RGB;
	}
	glEnable(GL_TEXTURE_2D);
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, pimg->texw, pimg->texh, 0,format,pimg->dtype, pimg->data);
	return 1;
}

static void DrawImageRot(image_p texture,float sx,float sy,float sw,float sh,float dx,float dy,float dw,float dh,float angle,int mask)
{
	static uint8 r,g,b,a;
	static int cacheid = 0;
	static int ret = 0;
	Vectice2D* vectices = (Vectice2D*)NgeGetMemoryVectice(4*sizeof(Vectice2D));
	glEnable(GL_TEXTURE_2D);
	glTranslatef(dx+texture->rcentrex,dy+texture->rcentrey,0);
	glRotatef(angle,0,0,1);
	glTranslatef(-(dx+texture->rcentrex),-(dy+texture->rcentrey),0);	
 	/*if((texture->modified==1)||texture->texid != m_tex_in_ram){
		TexImage2D(texture);
		texture->modified = 0;
	}*/
	ret = tex_cache_getid(texture->texid,&cacheid);
	glBindTexture(GL_TEXTURE_2D, cacheid);
	if(ret == 0 ||texture->modified==1){
		TexImage2D(texture);
		texture->modified = 0;
	}
	if(sw==0 && sh==0){
		sw = texture->w;
		sh = texture->h;
	}
	if(dw==0 && dh==0){
		dw = sw;
		dh = sh;
	}
	GetRGBA(mask,texture->dtype,&r,&g,&b,&a);
	vectices[0].x = dx;
	vectices[0].y = dy;
	vectices[0].color[0] = r;
	vectices[0].color[1] = g;
	vectices[0].color[2] = b;
	vectices[0].color[3] = a;
	vectices[0].uv[0] = sx/texture->texw;
	vectices[0].uv[1] = sy/texture->texh;
	
	vectices[1].x = dx ;
	vectices[1].y = dy+dh;
	vectices[1].color[0] = r;
	vectices[1].color[1] = g;
	vectices[1].color[2] = b;
	vectices[1].color[3] = a;
	vectices[1].uv[0] = sx/texture->texw;
	vectices[1].uv[1] = (sy+sh)/texture->texh;
	
	vectices[3].x = dx+dw;
	vectices[3].y = dy+dh;
	vectices[3].color[0] = r;
	vectices[3].color[1] = g;
	vectices[3].color[2] = b;
	vectices[3].color[3] = a;
	vectices[3].uv[0] = (sx+sw)/texture->texw;
	vectices[3].uv[1] = (sy+sh)/texture->texh;
	
	vectices[2].x = dx+dw;
	vectices[2].y = dy ;
	vectices[2].color[0] = r;
	vectices[2].color[1] = g;
	vectices[2].color[2] = b;
	vectices[2].color[3] = a;
	vectices[2].uv[0] = (sx+sw)/texture->texw;
	vectices[2].uv[1] = sy/texture->texh;
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vectice2D),
                   vectices[0].color);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2,GL_FLOAT,sizeof(Vectice2D),vectices[0].uv);
	glVertexPointer(2,GL_FLOAT,sizeof(Vectice2D),vectices);
	glEnableClientState(GL_VERTEX_ARRAY);
	
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDisable(GL_TEXTURE_2D);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void DrawImageMask(image_p tex,float sx,float sy,float sw,float sh,float dx,float dy,float dw,float dh,int mask)
{
	DrawImageRot(tex,sx,sy,sw,sh,dx,dy,dw,dh,0,mask);
}


void DrawImage(image_p tex,float sx,float sy,float sw,float sh,float dx,float dy,float dw,float dh)
{
	DrawImageRot(tex,sx,sy,sw,sh,dx,dy,dw,dh,0,tex->mask);
}


void RenderQuad(image_p texture,float sx,float sy,float sw,float sh,float dx,float dy,float xscale ,float yscale,float angle,int mask )
{	
	static uint8 r,g,b,a;
	static int cacheid = 0;
	static int ret = 0;
	Vectice2D* vectices = (Vectice2D*)NgeGetMemoryVectice(4*sizeof(Vectice2D));
	if(dy == 0.0f)
		dy = 0.1f;
	glEnable(GL_TEXTURE_2D);
	glTranslatef(dx+texture->rcentrex*xscale,dy+texture->rcentrey*yscale,0);
	glRotatef(angle,0.0f,0.0f,1.0f);
	glTranslatef(-(dx+texture->rcentrex*xscale),-(dy+texture->rcentrey*yscale),0);	
 	/*if((texture->modified==1)||texture->texid != m_tex_in_ram){
		TexImage2D(texture);
		texture->modified = 0;
	}*/
	ret = tex_cache_getid(texture->texid,&cacheid);
	glBindTexture(GL_TEXTURE_2D, cacheid);
	if(ret == 0 ||texture->modified==1){
		TexImage2D(texture);
		texture->modified = 0;
	}
	
	if(sw == 0 && sh == 0){
		GetRGBA(mask,texture->dtype,&r,&g,&b,&a);
		vectices[0].x = dx;
		vectices[0].y = dy;
		vectices[0].color[0] = r;
		vectices[0].color[1] = g;
		vectices[0].color[2] = b;
		vectices[0].color[3] = a;
		vectices[0].uv[0] = 0.0f;
		vectices[0].uv[1] = 0.0f;
		
		vectices[1].x = dx ;
		vectices[1].y = dy+texture->texh*yscale;
		vectices[1].color[0] = r;
		vectices[1].color[1] = g;
		vectices[1].color[2] = b;
		vectices[1].color[3] = a;
		vectices[1].uv[0] = 0.0f;
		vectices[1].uv[1] = 1.0f;//texture->texh;
		
		vectices[3].x = dx+texture->texw*xscale;
		vectices[3].y = dy+texture->texh*yscale;
		vectices[3].color[0] = r;
		vectices[3].color[1] = g;
		vectices[3].color[2] = b;
		vectices[3].color[3] = a;
		vectices[3].uv[0] = 1.0f;//texture->texw;
		vectices[3].uv[1] = 1.0f;//texture->texh;
		
		vectices[2].x = dx+texture->texw*xscale;
		vectices[2].y = dy ;
		vectices[2].color[0] = r;
		vectices[2].color[1] = g;
		vectices[2].color[2] = b;
		vectices[2].color[3] = a;
		vectices[2].uv[0] = 1.0f;//texture->texw;
		vectices[2].uv[1] = 0.0f;		
	}
	else{
		GetRGBA(mask,texture->dtype,&r,&g,&b,&a);
		vectices[0].x = dx;
		vectices[0].y = dy;
		vectices[0].color[0] = r;
		vectices[0].color[1] = g;
		vectices[0].color[2] = b;
		vectices[0].color[3] = a;
		vectices[0].uv[0] = sx/texture->texw;
		vectices[0].uv[1] = sy/texture->texh;
		
		vectices[1].x = dx ;
		vectices[1].y = dy+sh*yscale;
		vectices[1].color[0] = r;
		vectices[1].color[1] = g;
		vectices[1].color[2] = b;
		vectices[1].color[3] = a;
		vectices[1].uv[0] = sx/texture->texw;
		vectices[1].uv[1] = (sy+sh)/texture->texh;
		
		vectices[3].x = dx+sw*xscale;
		vectices[3].y = dy+sh*yscale;
		vectices[3].color[0] = r;
		vectices[3].color[1] = g;
		vectices[3].color[2] = b;
		vectices[3].color[3] = a;
		vectices[3].uv[0] = (sx+sw)/texture->texw;
		vectices[3].uv[1] = (sy+sh)/texture->texh;
		
		vectices[2].x = dx+sw*xscale;
		vectices[2].y = dy ;
		vectices[2].color[0] = r;
		vectices[2].color[1] = g;
		vectices[2].color[2] = b;
		vectices[2].color[3] = a;
		vectices[2].uv[0] = (sx+sw)/texture->texw;
		vectices[2].uv[1] = sy/texture->texh;
	}
	
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vectice2D),
                   vectices[0].color);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2,GL_FLOAT,sizeof(Vectice2D),vectices[0].uv);
	glVertexPointer(2,GL_FLOAT,sizeof(Vectice2D),vectices);
	glEnableClientState(GL_VERTEX_ARRAY);
	
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDisable(GL_TEXTURE_2D);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void ImageToScreen(image_p texture,float dx,float dy)
{
	DrawImage(texture,0,0,texture->w,texture->h,dx,dy,0,0);
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

