#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <pspgu.h>
#include <pspgum.h>
#include <psprtc.h>
#include "nge_graphics.h"
#include "nge_vram.h"
#include "nge_timer.h"
#include "nge_misc.h"
#include <math.h>
static unsigned int __attribute__((aligned(16))) list[262144];

#define BUF_WIDTH (512)
#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)

#define PSP_SLICE_F 0.03125f
#define PSP_PI_DIV_180 0.0174532925
#define MIN(a,b) ((a)<(b)?(a):(b))

static float m_sintable[360];
static float m_costable[360];
#define RAD2DEG		57.29577951f
#define DEG2RAD		0.017453293f
#define SINF(a)  (m_sintable[a%360])
#define COSF(a)  (m_costable[a%360])


static uint32 screen_color = 0;

struct Vertex
{
	unsigned int color;
	float x,y,z;
};

struct VertexUV
{
	float u, v;
	unsigned int color;
	float x,y,z;
};

struct VertexUV_NO_CLOLR
{
	float u, v;
	float x,y,z;
};


static void* m_drawbuf;
static void* m_displaybuf;
static void* m_zbuf;
static ScePspFVector3 m_transmatrix;
static float ProjectionMatrix[2][16];
static int use_vblank = 0;;
//fps count
/**debug show fps*/
u64 m_lasttick;
u64 m_currtick;
uint32 m_tickres;
float m_currms;
uint32 m_fcount;
static uint8 show_fps = 0;
static uint32 m_tex_in_ram = -1;
static uint32 m_tex_id = 0;

static nge_timer* timer = NULL;

static screen_context_t nge_screen = {
		"NGE2",
		SCREEN_WIDTH_PSP,
		SCREEN_HEIGHT_PSP,
		SCREEN_BPP,
		0
};



static void FpsInit()
{
	m_currms = 1.0;
	m_fcount = 0;
	sceRtcGetCurrentTick(&m_lasttick);
	m_tickres = sceRtcGetTickResolution();
}

screen_context_p GetScreenContext()
{
	return &nge_screen;
}

void SetScreenType(int type)
{
}

void ShowFps()
{
	static int init_fps = 0;
	if(init_fps == 0){
		FpsInit();
		init_fps = 1;
	}
	show_fps = 1;
}

void LimitFps(uint32 limit)
{
	if(limit!=60){
		if(limit == 0)
			limit = 60;

		if( timer->get_ticks(timer) < 1000 / limit )
		{
				nge_sleep( ( 1000 / limit) - timer->get_ticks(timer) );
		}
		timer->start(timer);
		use_vblank = 0;
	}
	else{
		use_vblank = 1;
		//sceDisplayWaitVblankStart();
	}
}

#include <malloc.h>
int __freemem()
{
 void *ptrs[480];
 int mem, x, i;
 void *ptr;

 for (x = 0; x < 480; x++)
 {
	ptr = malloc(51200);
	if (!ptr) break;

	ptrs[x] = ptr;
 }
 mem = x * 51200;
 for (i = 0; i < x; i++)
  free(ptrs[i]);

 return mem;
}

static void myShowFps()
{
	static int tick = 10000;
	static int countm = 0;
	static int freemem = 0;
	double time_span;
	float curr_fps;

	++m_fcount;
	sceRtcGetCurrentTick(&m_currtick);
	{
		pspDebugScreenSetOffset((int)m_drawbuf);
		pspDebugScreenSetXY(0,0);
		curr_fps = 1.0f / m_currms;
		if(tick>curr_fps*60){
			tick = 0;
			freemem = __freemem();
			countm++;
		}
		tick++;
		pspDebugScreenPrintf("fps: %.2f \tfreemem: %d bytes(%.2fM) countm=%d",curr_fps,freemem,freemem*1.0/(1024*1024),countm );
		time_span = ((m_currtick-m_lasttick)) / (float)m_tickres;
		m_currms = time_span / m_fcount;
		m_fcount = 0;
		sceRtcGetCurrentTick(&m_lasttick);
	}
	m_drawbuf = sceGuSwapBuffers();
}


static void InitGu(void)
{
	// Setup GU
	//pspDebugScreenInit();
	// Setup GU
	m_drawbuf = getStaticVramBuffer(BUF_WIDTH,SCR_HEIGHT,GU_PSM_8888);
	m_displaybuf = getStaticVramBuffer(BUF_WIDTH,SCR_HEIGHT,GU_PSM_8888);
	m_zbuf = getStaticVramBuffer(BUF_WIDTH,SCR_HEIGHT,GU_PSM_4444);
	// setup GU
	sceGuInit();
	sceGuStart(GU_DIRECT,list);
	sceGuDrawBuffer(GU_PSM_8888,m_drawbuf,BUF_WIDTH);
	sceGuDispBuffer(SCR_WIDTH,SCR_HEIGHT,m_displaybuf,BUF_WIDTH);
	sceGuDepthBuffer(m_zbuf,BUF_WIDTH);
	sceGuOffset(2048 - (SCREEN_WIDTH/2), 2048 - (SCREEN_HEIGHT/2));
	sceGuViewport(2048, 2048, SCREEN_WIDTH, SCREEN_HEIGHT);
	// Scissoring
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuScissor(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	// Backface culling
	sceGuFrontFace(GU_CCW);
	sceGuDisable(GU_CULL_FACE);		// no culling in 2D
	// Depth test
	sceGuDisable(GU_DEPTH_TEST);
	sceGuDepthMask(GU_TRUE);		// disable z-writes
	// Color keying
	sceGuDisable(GU_COLOR_TEST);
	sceGuDisable(GU_ALPHA_TEST);
	sceGuDisable(GU_CLIP_PLANES);
	// Texturing
	sceGuEnable(GU_TEXTURE_2D);
	sceGuShadeModel(GU_SMOOTH);
	sceGuTexWrap(GU_CLAMP, GU_CLAMP);
	sceGuTexFilter(GU_LINEAR,GU_LINEAR);
	//sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
	sceGuTexEnvColor(0xFFFFFFFF);
	sceGuColor(0xFFFFFFFF);
	sceGuAmbientColor(0xFFFFFFFF);
	sceGuTexOffset(0.0f, 0.0f);
	sceGuTexScale(1.0f, 1.0f);
	// Blending
	sceGuEnable(GU_BLEND);
	sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
	sceGuTexFunc(GU_TFX_MODULATE,GU_TCC_RGBA);
	sceGuTexFilter(GU_LINEAR,GU_LINEAR);
	sceGuDisable(GU_DITHER);
	/*if (triBpp<4)
	{
		sceGuSetDither( (ScePspIMatrix4*)DitherMatrix[0] );
		sceGuEnable(GU_DITHER);
	}*/


	// Projection
	gumLoadIdentity( (ScePspFMatrix4*)ProjectionMatrix[0] );
	gumOrtho( (ScePspFMatrix4*)ProjectionMatrix[0], 0.0f, 480.0f, 272.0f, 0.0f, -1.0f, 1.0f );

	gumLoadIdentity( (ScePspFMatrix4*)ProjectionMatrix[1] );
	ScePspFVector3 displace = { -0.002f, 0.00367f, 0.0f };	// ~ 1/480, 1/272
	gumTranslate( (ScePspFMatrix4*)ProjectionMatrix[1], &displace );
	gumMultMatrix( (ScePspFMatrix4*)ProjectionMatrix[1], (ScePspFMatrix4*)ProjectionMatrix[1], (ScePspFMatrix4*)ProjectionMatrix[0] );

	sceGumMatrixMode(GU_PROJECTION);
	sceGumLoadMatrix( (ScePspFMatrix4*)ProjectionMatrix[0] );


	sceGumMatrixMode(GU_VIEW);
	sceGumLoadIdentity();

	sceGumMatrixMode(GU_MODEL);
	sceGumLoadIdentity();

	sceGuClearColor( 0x0 );
	sceGuClear(GU_COLOR_BUFFER_BIT|GU_FAST_CLEAR_BIT);
	sceGuFinish();
	sceGuSync(0,0);

	sceDisplayWaitVblankStart();
	sceGuDisplay(1);

	// finish
}



void GetVersion()
{

}

void InitGraphics()
{
	int i;
	InitGu();
	timer = timer_create();
	for (i=0;i<360;i++)
	{
		m_sintable[i] = sin(i*DEG2RAD);
		m_costable[i] = cos(i*DEG2RAD);
	}
}
void FiniGraphics()
{
	sceGuTerm();
	timer_free(timer);
}

void SetTexBlend(int src_blend, int des_blend)
{
	//if(src_blend==0&&des_blend==0)
		//glBlendFunc(BLEND_SRC_ALPHA,BLEND_ONE);
	//	sceGuBlendFunc(GU_ADD, BLEND_SRC_ALPHA,BLEND_ONE, 0, 0);
	//else{
		//glBlendFunc(src_blend,des_blend);
		int fixSrc = 0;
		int fixDest = 0;
		if (src_blend == BLEND_ZERO)
			src_blend = GU_FIX;
		else if (src_blend == BLEND_ONE)
		{
			src_blend = GU_FIX;
			fixSrc = 0x00FFFFFF;
		}
		if (des_blend == BLEND_ZERO)
			des_blend = GU_FIX;
		else if (des_blend == BLEND_ONE)
		{
			des_blend = GU_FIX;
			fixDest = 0x00FFFFFF;
		}

		//glBlendFunc(src, dest);
		sceGuBlendFunc(GU_ADD, src_blend, des_blend, fixSrc, fixDest);
	//}
}

void ResetTexBlend()
{
	//glBlendFunc(BLEND_SRC_ALPHA,BLEND_ONE_MINUS_SRC_ALPHA);
	sceGuBlendFunc(GU_ADD, BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA, 0, 0);
}

void BeginScene(uint8 clear)
{
	sceGuStart(GU_DIRECT,list);
	if(clear){
		sceGuDisable(GU_SCISSOR_TEST);
		sceGuClearColor(screen_color);
		sceGuClear(GU_COLOR_BUFFER_BIT);
		sceGuEnable(GU_SCISSOR_TEST);
	}
}

uint32 SetScreenColor(uint8 r,uint8 g,uint8 b,uint8 a)
{
	uint32 u_lastcolor;
	u_lastcolor = screen_color;
	screen_color = MAKE_RGBA_8888(r,g,b,a);
	return u_lastcolor;
}

void EndScene()
{
	if(use_vblank == 1)
		sceDisplayWaitVblankStart();
	sceGuFinish();
	sceGuSync(0,0);
	if(show_fps == 1){
		myShowFps();
	}
	else
		sceGuSwapBuffers();
}

void PutPix(float x, float y, int color,int dtype)
{
	struct Vertex* vertices = (struct Vertex*)sceGuGetMemory(1* sizeof(struct Vertex));
	vertices[0].color = color;
	vertices[0].x = x;
	vertices[0].y = y;
	vertices[0].z = 0.0f;

	sceGuDisable(GU_TEXTURE_2D);
	sceGuShadeModel(GU_FLAT);
	sceGuShadeModel(GU_SMOOTH);
	sceGuAmbientColor(0xffffffff);
	sceGumDrawArray(GU_POINTS, dtype|GU_VERTEX_32BITF|GU_TRANSFORM_3D, 1, 0, vertices);
	sceGuEnable(GU_TEXTURE_2D);
}


void DrawLine(float x1, float y1, float x2, float y2, int color,int dtype)
{
	struct Vertex* vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));
	vertices[0].color = color;
	vertices[0].x = x1;
	vertices[0].y = y1;
	vertices[0].z = 0.0f;

	vertices[1].color = color;
	vertices[1].x = x2;
	vertices[1].y = y2;
	vertices[1].z = 0.0f;

	sceGuDisable(GU_TEXTURE_2D);
	sceGuShadeModel(GU_FLAT);
	sceGuShadeModel(GU_SMOOTH);
	sceGuAmbientColor(0xffffffff);
	sceGumDrawArray(GU_LINES, dtype|GU_VERTEX_32BITF|GU_TRANSFORM_3D, 1*2, 0, vertices);
	sceGuEnable(GU_TEXTURE_2D);
}

void DrawLineEx(pointf p1,pointf p2 ,int color,int dtype)
{
	DrawLine(p1.x, p1.y, p2.x, p2.y,color,dtype);
}

void SetClip(int x,int y,int w,int h)
{
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuScissor(x,y,x+w,y+h);
}

void ResetClip()
{
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuScissor(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
}

void DrawRect(float x, float y, float width, float height,int color,int dtype)
{
	struct Vertex* vertices = (struct Vertex*)sceGuGetMemory(5 * sizeof(struct Vertex));

	vertices[0].color = color;
	vertices[0].x = x;
	vertices[0].y = y;
	vertices[0].z = 0.0f;

	vertices[1].color = color;
	vertices[1].x = x;
	vertices[1].y = y + height;
	vertices[1].z = 0.0f;

	vertices[2].color = color;
	vertices[2].x = x + width;
	vertices[2].y = y + height;
	vertices[2].z = 0.0f;

	vertices[3].color = color;
	vertices[3].x = x + width;
	vertices[3].y = y;
	vertices[3].z = 0.0f;

	vertices[4].color = color;
	vertices[4].x = x;
	vertices[4].y = y;
	vertices[4].z = 0.0f;

	sceGuDisable(GU_TEXTURE_2D);
	sceGuShadeModel(GU_SMOOTH);
	sceGuAmbientColor(0xffffffff);
	sceGuDrawArray(GU_LINE_STRIP, dtype|GU_VERTEX_32BITF|GU_TRANSFORM_2D, 5, 0, vertices);
	sceGuEnable(GU_TEXTURE_2D);

}

void DrawRectEx(rectf rect,int color,int dtype)
{
	DrawRect(rect.top, rect.left, rect.right-rect.left, rect.bottom-rect.top,color,dtype);
}

void FillRect(float x, float y, float width, float height,int color,int dtype)
{
	struct Vertex* vertices = (struct Vertex*)sceGuGetMemory(4 * sizeof(struct Vertex));

	vertices[0].color = color;
	vertices[0].x = x;
	vertices[0].y = y;
	vertices[0].z = 0.0f;

	vertices[1].color = color;
	vertices[1].x = x;
	vertices[1].y = y + height;
	vertices[1].z = 0.0f;

	vertices[3].color = color;
	vertices[3].x = x + width;
	vertices[3].y = y + height;
	vertices[3].z = 0.0f;

	vertices[2].color = color;
	vertices[2].x = x + width;
	vertices[2].y = y;
	vertices[2].z = 0.0f;

	sceGuDisable(GU_TEXTURE_2D);
	sceGuShadeModel(GU_SMOOTH);
	sceGuAmbientColor(0xffffffff);
	sceGuDrawArray(GU_TRIANGLE_STRIP, dtype|GU_VERTEX_32BITF|GU_TRANSFORM_2D, 4, 0, vertices);
	sceGuEnable(GU_TEXTURE_2D);

}

void FillCircle(float x, float y, float radius, int color,int dtype)
{
	int i,angle = 359;
	struct Vertex* vertices = (struct Vertex*)sceGuGetMemory(182 * sizeof(struct Vertex));

	vertices[0].color = color;
	vertices[0].x = x;
	vertices[0].y = y;
	vertices[0].z = 0.0f;

	for(i=0; i<180; i++)
	{
		vertices[i+1].color = color;
		vertices[i+1].x = x+radius*COSF(angle);
		vertices[i+1].y = y+radius*SINF(angle);
		vertices[i+1].z = 0.0f;
		angle -= 2;
		if (angle < 0)
			angle = 0;
	}

	vertices[181].color = color;
	vertices[181].x = x+radius*COSF(359);
	vertices[181].y = y+radius*SINF(359);
	vertices[181].z = 0.0f;

	sceGuDisable(GU_TEXTURE_2D);
	sceGuShadeModel(GU_SMOOTH);
	sceGuAmbientColor(0xffffffff);
	sceGuDrawArray(GU_TRIANGLE_FAN, dtype|GU_VERTEX_32BITF|GU_TRANSFORM_2D, 182, 0, vertices);
	sceGuEnable(GU_TEXTURE_2D);
}

void DrawCircle(float x, float y, float radius, int color,int dtype)
{
	int i,angle = 359;
	struct Vertex* vertices = (struct Vertex*)sceGuGetMemory(181 * sizeof(struct Vertex));

	for(i=0; i<180; i++)
	{
		vertices[i].color = color;
		vertices[i].x = x+radius*COSF(angle);
		vertices[i].y = y+radius*SINF(angle);
		vertices[i].z = 0.0f;
		angle -= 2;
		if (angle < 0)
			angle = 0;
	}

	vertices[180].color = color;
	vertices[180].x = x+radius*COSF(0);
	vertices[180].y = y+radius*SINF(0);
	vertices[180].z = 0.0f;


	sceGuDisable(GU_TEXTURE_2D);
	sceGuShadeModel(GU_SMOOTH);
	sceGuAmbientColor(0xffffffff);
	sceGuDrawArray(GU_LINE_STRIP, dtype|GU_VERTEX_32BITF|GU_TRANSFORM_2D, 181, 0, vertices);
	sceGuEnable(GU_TEXTURE_2D);
}

void FillEllipse(float x,float y ,float xradius,float yradius,int color,int dtype)
{
	int i,angle = 359;
	struct Vertex* vertices = (struct Vertex*)sceGuGetMemory(182 * sizeof(struct Vertex));

	vertices[0].color = color;
	vertices[0].x = x;
	vertices[0].y = y;
	vertices[0].z = 0.0f;

	for(i=0; i<180; i++)
	{
		vertices[i+1].color = color;
		vertices[i+1].x = x+COSF(angle)*xradius;
		vertices[i+1].y = y+SINF(angle)*yradius;
		vertices[i+1].z = 0.0f;
		angle -= 2;
		if (angle < 0)
			angle = 0;
	}

	vertices[181].color = color;
	vertices[181].x = x+COSF(359)*xradius;
	vertices[181].y = y+SINF(359)*yradius;
	vertices[181].z = 0.0f;

	sceGuDisable(GU_TEXTURE_2D);
	sceGuShadeModel(GU_SMOOTH);
	sceGuAmbientColor(0xffffffff);
	sceGuDrawArray(GU_TRIANGLE_FAN, dtype|GU_VERTEX_32BITF|GU_TRANSFORM_2D, 182, 0, vertices);
	sceGuEnable(GU_TEXTURE_2D);
}

void DrawEllipse(float x,float y ,float xradius,float yradius,int color,int dtype)
{
	int i,angle = 359;
	struct Vertex* vertices = (struct Vertex*)sceGuGetMemory(181 * sizeof(struct Vertex));

	for(i=0; i<180; i++)
	{
		vertices[i].color = color;
		vertices[i].x = x+COSF(angle)*xradius;
		vertices[i].y = y+SINF(angle)*yradius;
		vertices[i].z = 0.0f;
		angle -= 2;
		if (angle < 0)
			angle = 0;
	}

	vertices[180].color = color;
	vertices[180].x = x+COSF(0)*xradius;
	vertices[180].y = y+SINF(0)*yradius;
	vertices[180].z = 0.0f;


	sceGuDisable(GU_TEXTURE_2D);
	sceGuShadeModel(GU_SMOOTH);
	sceGuAmbientColor(0xffffffff);
	sceGuDrawArray(GU_LINE_STRIP, dtype|GU_VERTEX_32BITF|GU_TRANSFORM_2D, 181, 0, vertices);
	sceGuEnable(GU_TEXTURE_2D);
}

void DrawPolygon(float* x, float* y, int count, int color,int dtype)
{
	int i;
	struct Vertex* vertices = (struct Vertex*)sceGuGetMemory((count+1) * sizeof(struct Vertex));

	for(i=0; i<count; i++)
	{
		vertices[i].color = color;
		vertices[i].x = x[i];
		vertices[i].y = y[i];
		vertices[i].z = 0.0f;
	}

	vertices[count].color = color;
	vertices[count].x = x[0];
	vertices[count].y = y[0];
	vertices[count].z = 0.0f;

	sceGuDisable(GU_TEXTURE_2D);
	sceGuShadeModel(GU_SMOOTH);
	sceGuAmbientColor(0xffffffff);
	sceGuDrawArray(GU_LINE_STRIP, dtype|GU_VERTEX_32BITF|GU_TRANSFORM_2D, count+1, 0, vertices);
	sceGuEnable(GU_TEXTURE_2D);
}

void FillPolygon(float* x, float* y, int count, int color,int dtype)
{
	int i;
	struct Vertex* vertices = (struct Vertex*)sceGuGetMemory(count * sizeof(struct Vertex));

	for(i=0; i<count; i++)
	{
		vertices[i].color = color;
		vertices[i].x = x[i];
		vertices[i].y = y[i];
		vertices[i].z = 0.0f;
	}

	sceGuDisable(GU_TEXTURE_2D);
	sceGuShadeModel(GU_SMOOTH);
	sceGuAmbientColor(0xffffffff);
	sceGuDrawArray(GU_TRIANGLE_FAN, dtype|GU_VERTEX_32BITF|GU_TRANSFORM_2D, count, 0, vertices);
	sceGuEnable(GU_TEXTURE_2D);
}

void FillPolygonGrad(float* x, float* y, int count, int* colors,int dtype)
{
	int i;
	struct Vertex* vertices = (struct Vertex*)sceGuGetMemory(count * sizeof(struct Vertex));

	for(i=0; i<count; i++)
	{
		vertices[i].color = colors[i];
		vertices[i].x = x[i];
		vertices[i].y = y[i];
		vertices[i].z = 0.0f;
	}

	sceGuDisable(GU_TEXTURE_2D);
	sceGuShadeModel(GU_SMOOTH);
	sceGuAmbientColor(0xffffffff);
	sceGuDrawArray(GU_TRIANGLE_FAN, dtype|GU_VERTEX_32BITF|GU_TRANSFORM_2D, count, 0, vertices);
	sceGuEnable(GU_TEXTURE_2D);
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
void FillRectGrad(float x, float y, float width, float height,int* colors,int dtype)
{
	struct Vertex* vertices = (struct Vertex*)sceGuGetMemory(4 * sizeof(struct Vertex));

	vertices[0].color = colors[0];
	vertices[0].x = x;
	vertices[0].y = y;
	vertices[0].z = 0.0f;

	vertices[1].color = colors[1];
	vertices[1].x = x;
	vertices[1].y = y + height;
	vertices[1].z = 0.0f;

	vertices[3].color = colors[2];
	vertices[3].x = x + width;
	vertices[3].y = y + height;
	vertices[3].z = 0.0f;

	vertices[2].color = colors[3];
	vertices[2].x = x + width;
	vertices[2].y = y;
	vertices[2].z = 0.0f;

	sceGuDisable(GU_TEXTURE_2D);
	sceGuShadeModel(GU_SMOOTH);
	sceGuAmbientColor(0xffffffff);
	sceGuDrawArray(GU_TRIANGLE_STRIP, dtype|GU_VERTEX_32BITF|GU_TRANSFORM_2D, 4, 0, vertices);
	sceGuEnable(GU_TEXTURE_2D);

}

void FillRectGradEx(rectf rect,int* colors,int dtype)
{
	FillRectGrad(rect.top, rect.left, rect.right-rect.left, rect.bottom-rect.top,colors,dtype);
}

void RenderQuad(image_p tex,float sx,float sy,float sw,float sh,float dx,float dy,float xscale ,float yscale ,float angle ,int color)
{
	float su,sv,swf;
	float ustart,width,step;
	struct VertexUV *vertices;

	if(tex == 0 || tex->w == 0 || tex->h ==0)
		return ;
	if((xscale==0)||(yscale==0)){
		xscale = 1.0;
		yscale = 1.0;
	}
	if((sw == 0)||(sh == 0)){
		sw = tex->w * xscale;
		sh = tex->h * yscale;
	}
	else{
		sw = sw * xscale;
		sh = sh * yscale;
	}
	su = sw/tex->texw;
	sv = sh/tex->texh;
	swf = tex->texw;

	sceGumMatrixMode(GU_VIEW);
	sceGumLoadIdentity();

	sceGumMatrixMode(GU_MODEL);
	sceGumLoadIdentity();
	//平移,旋转,放缩变换
	m_transmatrix.x = dx;
	m_transmatrix.y = dy;
	sceGumTranslate(&m_transmatrix);
	ScePspFVector3 tmp;
	tmp.x = tex->rcentrex * xscale;
	tmp.y = tex->rcentrey * yscale;
	tmp.z = 0.0;
	sceGumTranslate(&tmp);
	sceGumRotateZ(angle * PSP_PI_DIV_180);
	tmp.x = -tex->rcentrex * xscale;
	tmp.y = -tex->rcentrey * yscale;
	tmp.z = 0.0;
	sceGumTranslate(&tmp);

	if(tex->swizzle == 0 && tex->dontswizzle ==0){
		swizzle_swap(tex);
	}
	sceGuTexMode(tex->mode,0,0,tex->swizzle);
	sceKernelDcacheWritebackAll();
	sceGuTexOffset(sx/tex->texw,sy/tex->texh);
	sceGuTexScale(1.0/xscale,1.0/yscale);
	if((tex->modified==1)||tex->texid != m_tex_in_ram){
		m_tex_in_ram = tex->texid;
		tex->modified = 0;
		sceGuTexImage(0,tex->texw,tex->texh,tex->texw,tex->data);
		sceKernelDcacheWritebackAll();
		//nge_print("hit \n");
	}
	//sceGuTexFunc(GU_TFX_MODULATE,GU_TCC_RGBA);
	//sceGuTexFilter(GU_LINEAR,GU_LINEAR);

	//psp dcache优化
	for(ustart = 0,step = 0;ustart<su;ustart += PSP_SLICE_F,step += swf * PSP_SLICE_F){
		vertices = (struct VertexUV*)sceGuGetMemory(4 * sizeof(struct VertexUV));
		width = (ustart + PSP_SLICE_F ) < su ? (PSP_SLICE_F) : su-ustart;

		vertices[0].u = ustart;
		vertices[0].v = 0;
		vertices[0].color = color;
		vertices[0].x = ustart * swf;
		vertices[0].y = 0;
		vertices[0].z = 0.0f;
		vertices[1].u = ustart + width;
		vertices[1].v = 0;
		vertices[1].color = color;
		vertices[1].x = step + width * swf;
		vertices[1].y = 0;
		vertices[1].z = 0.0f;
		vertices[2].u = ustart;
		vertices[2].v = sv;
		vertices[2].color = color;
		vertices[2].x = ustart * swf;
		vertices[2].y = sh;
		vertices[2].z = 0.0f;
		vertices[3].u = ustart + width;
		vertices[3].v = sv;
		vertices[3].color = color;
		vertices[3].x = (step + width * swf);
		vertices[3].y = sh;
		vertices[3].z = 0.0f;
		sceGumDrawArray(GU_TRIANGLE_STRIP,GU_TEXTURE_32BITF|(tex->dtype)|GU_VERTEX_32BITF|GU_TRANSFORM_3D,4,0,vertices);
	}
	//恢复坐标
	sceGumMatrixMode(GU_VIEW);
	sceGumLoadIdentity();

	sceGumMatrixMode(GU_MODEL);
	sceGumLoadIdentity();
}


/*
void DrawLargeImageMask(image_p tex,float sx , float sy, float sw, float sh, float dx, float dy, float dw, float dh,int mask)
{
	uint8 *data;
	float start, end;
	float cur_u,cur_x,x_end,slice,ustep;
	struct VertexUV *vertices;
	float poly_width,source_width;
	float u1 ,v1;
	int off ;

	if (tex==0 || tex->w==0 || tex->h==0 || dw == 0 || dh == 0) return;

	u1 = sx + sw;
	v1 = sy + sh;
	sceGuTexMode(tex->mode, 0, 0, tex->swizzle);
	data = tex->data;
	if (u1>512.f)
	{
		//off = (int)sx & ~31;
		data += ((int)sx)*tex->bpb;
		//off =256;
		//data += (off*tex->bpb);
		u1 -= sx;
		sx = 0.f;
		//sx -= off;
	}
	if (v1>512.f)
	{
		//off = (int)sy ;
		//data += off*tex->texw*tex->bpb;
		//v1 -= off;
		//sy -= off;

		data += ((int)sy)*tex->texw*tex->bpb;
		v1 -= sy;
		sy = 0.f;
	}

	m_tex_in_ram = tex->texid;
	sceGuTexImage(0, MIN(512,tex->texw), MIN(512,tex->texh), tex->texw, data);
	sceGuEnable(GU_TEXTURE_2D);


	cur_u = sx;
	cur_x = dx;
	x_end = dx + dw;
	slice = 64.f;
	ustep = (u1-sx)/dw * slice;

	// blit maximizing the use of the texture-cache
	for( start=0, end=dw; start<end; start+=slice )
	{
		vertices = (struct VertexUV*)sceGuGetMemory(2 * sizeof(struct VertexUV));

		poly_width = ((cur_x+slice) > x_end) ? (x_end-cur_x) : slice;
		source_width = ((cur_u+ustep) > u1) ? (u1-cur_u) : ustep;
		vertices[0].color = mask;
		vertices[0].u = cur_u;
		vertices[0].v = sy;
		vertices[0].x = cur_x;
		vertices[0].y = dy;
		vertices[0].z = 0;

		cur_u += source_width;
		cur_x += poly_width;
		vertices[1].color = mask;
		vertices[1].u = cur_u;
		vertices[1].v = v1;
		vertices[1].x = cur_x;
		vertices[1].y = (dy + dh);
		vertices[1].z = 0;

		sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF|(tex->dtype)|GU_VERTEX_32BITF|GU_TRANSFORM_2D,2,0,vertices);
	}

}

void DrawLargeImage(image_p tex,float sx , float sy, float sw, float sh, float dx, float dy, float dw, float dh)
{
	DrawLargeImageMask(tex,sx,sy,sw,sh,dx,dy,dw,dh,tex->mask);
}
*/

void DrawImage(image_p tex,float sx , float sy, float sw, float sh, float dx, float dy, float dw, float dh)
{
	DrawImageMask(tex,sx,sy,sw,sh,dx,dy,dw,dh,tex->mask);
}

void DrawImageMask(image_p tex,float sx , float sy, float sw, float sh, float dx, float dy, float dw, float dh,int mask)
{
	uint8 *data;
	float start, end;
	float cur_u,cur_x,x_end,slice,ustep;
	struct VertexUV *vertices;
	float poly_width,source_width;
	float u1 ,v1;

	if (tex==0 || tex->w==0 || tex->h==0) return;

	if(sw == 0 && sh ==0){
		//sw = tex->texw;
		//sh = tex->texh;
		sw = tex->w;
		sh = tex->h;

	}
	if(dw == 0){
		if(dh == 0){
			dw = tex->w;
			dh = tex->h;
		}
		else{
			return;
		}
	}

	u1 = sx+sw;
	v1 = sy+sh;
	if(tex->swizzle == 0 && tex->dontswizzle ==0){
		swizzle_swap(tex);
	}
	sceGuTexMode(tex->mode, 0, 0, tex->swizzle);

	if((tex->modified==1)||tex->texid != m_tex_in_ram){
		m_tex_in_ram = tex->texid;
		tex->modified = 0;
		sceGuTexImage(0, tex->texw,tex->texh,tex->texw, tex->data);
		sceKernelDcacheWritebackAll();
		//nge_print("hit \n");
	}

	sceGuEnable(GU_TEXTURE_2D);


	cur_u = sx;
	cur_x = dx;
	x_end = dx + dw;
	slice = 64.f;
	ustep = (u1-sx)/dw * slice;

	// blit maximizing the use of the texture-cache
	for( start=0, end=dw; start<end; start+=slice )
	{
		vertices = (struct VertexUV*)sceGuGetMemory(2 * sizeof(struct VertexUV));

		float poly_width = ((cur_x+slice) > x_end) ? (x_end-cur_x) : slice;
		float source_width = ((cur_u+ustep) > u1) ? (u1-cur_u) : ustep;
		vertices[0].color = mask;
		vertices[0].u = cur_u;
		vertices[0].v = sy;
		vertices[0].x = cur_x;
		vertices[0].y = dy;
		vertices[0].z = 0;

		cur_u += source_width;
		cur_x += poly_width;
		vertices[1].color = mask;
		vertices[1].u = cur_u;
		vertices[1].v = v1;
		vertices[1].x = cur_x;
		vertices[1].y = (dy + dh);
		vertices[1].z = 0;

		sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF|(tex->dtype)|GU_VERTEX_32BITF|GU_TRANSFORM_2D,2,0,vertices);
	}
}

void ImageToScreen(image_p tex,float dx,float dy)
{
	 DrawImage( tex,0, 0, tex->w, tex->h,dx, dy, tex->w, tex->h);
}

image_p ScreenToImage()
{
	void* temp;
	int width,format,dtype;
	image_p pimage = NULL;
	int i,x,y;
	uint8 r,g,b;
	uint32 color;
	uint32 *vram32;
	uint16 *vram16;
	uint8 *line;
	sceDisplayWaitVblankStart();  // if framebuf was set with PSP_DISPLAY_SETBUF_NEXTFRAME, wait until it is changed
	sceDisplayGetFrameBuf(&temp, &width, &format, PSP_DISPLAY_SETBUF_NEXTFRAME);
	pimage = image_create(SCREEN_WIDTH,SCREEN_HEIGHT,DISPLAY_PIXEL_FORMAT_8888);
	if(pimage == NULL)
		return NULL;
	vram32 = (uint32*) temp;
	vram16 = (uint16*) vram32;
	line = (uint8*)pimage->data;
	for (y = 0; y < SCREEN_HEIGHT; y++) {
				for (i = 0, x = 0; x < SCREEN_WIDTH; x++) {
					   switch (format) {
								case PSP_DISPLAY_PIXEL_FORMAT_565:
										color = vram16[x + y * width];
										r = (color & 0x1f) << 3;
										g = ((color >> 5) & 0x3f) << 2 ;
										b = ((color >> 11) & 0x1f) << 3 ;
										break;
								case PSP_DISPLAY_PIXEL_FORMAT_5551:
										color = vram16[x + y * width];
										r = (color & 0x1f) << 3;
										g = ((color >> 5) & 0x1f) << 3 ;
										b = ((color >> 10) & 0x1f) << 3 ;
										break;
								case PSP_DISPLAY_PIXEL_FORMAT_4444:
										color = vram16[x + y * width];
										r = (color & 0xf) << 4;
										g = ((color >> 4) & 0xf) << 4 ;
										b = ((color >> 8) & 0xf) << 4 ;
										break;
								case PSP_DISPLAY_PIXEL_FORMAT_8888:
										color = vram32[x + y * width];
										r = color & 0xff;
										g = (color >> 8) & 0xff;
										b = (color >> 16) & 0xff;
										break;
						}
						line[i++] = r;
						line[i++] = g;
						line[i++] = b;
						line[i++] = 0xff;
				}
				line = line+pimage->texw*pimage->bpb;
		}
	return pimage;
}


void ScreenShot(const char* filename)
{
	image_p pimage = ScreenToImage();
	if(filename == NULL||pimage == NULL)
		return;
	image_save(pimage,filename,1,1);
	image_free(pimage);
}

static image_p targetIMG = NULL;

BOOL BeginTarget(image_p _img){
	unsigned int offset = getStaticVramOffset();
	int width, height;
	if(!_img)
		return FALSE;
	targetIMG = _img;
	width = _img->texw; height = _img->texh;

	if(_img->swizzle)
		unswizzle_swap(_img);
	
	sceGuStart(GU_DIRECT,list);
	
	sceGuDrawBufferList(GU_PSM_8888,(void*)offset,BUF_WIDTH);
	sceGuCopyImage(GU_PSM_8888, 0, 0, width, height, width, _img->data, 0, 0, BUF_WIDTH, sceGeEdramGetAddr() + offset);
//	sceGuDispBuffer(width,height,offset + width * height * (_img->mode<3?2:4),BUF_WIDTH);
	sceGuOffset(2048 - (width/2), 2048 - (height/2));
	sceGuViewport(2048, 2048, width, height);
	// Scissoring
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuScissor(0, 0, width, height);
	
/*	gumLoadIdentity( (ScePspFMatrix4*)ProjectionMatrix[0] );
	gumOrtho( (ScePspFMatrix4*)ProjectionMatrix[0], 0.0f, 480.0f, 0.0f, 272.0f, -1.0f, 1.0f );

	gumLoadIdentity( (ScePspFMatrix4*)ProjectionMatrix[1] );
	ScePspFVector3 displace = { -0.002f, 0.00367f, 0.0f };	// ~ 1/480, 1/272
	gumTranslate( (ScePspFMatrix4*)ProjectionMatrix[1], &displace );
	gumMultMatrix( (ScePspFMatrix4*)ProjectionMatrix[1], (ScePspFMatrix4*)ProjectionMatrix[1], (ScePspFMatrix4*)ProjectionMatrix[0] );

	sceGumMatrixMode(GU_PROJECTION);
	sceGumLoadMatrix( (ScePspFMatrix4*)ProjectionMatrix[0] );*/
	return TRUE;
}

void EndTarget(){
	int x,y;
	uint32 *vram32, *line32;
	uint16 *vram16, *line16;
	int width;
	int format;
	if(!targetIMG)
		return;
	width = targetIMG->texw;
	format = targetIMG->dtype;
	sceGuDrawBufferList(GU_PSM_8888,m_drawbuf,BUF_WIDTH);
//	sceGuDispBuffer(SCR_WIDTH,SCR_HEIGHT,m_displaybuf,BUF_WIDTH);
	sceGuOffset(2048 - (SCREEN_WIDTH/2), 2048 - (SCREEN_HEIGHT/2));
	sceGuViewport(2048, 2048, SCREEN_WIDTH, SCREEN_HEIGHT);
	
	sceGuFinish();
	sceGuSync(0,0);
	
	// Scissoring
/*	sceGuEnable(GU_SCISSOR_TEST);
	sceGuScissor(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	
	gumLoadIdentity( (ScePspFMatrix4*)ProjectionMatrix[0] );
	gumOrtho( (ScePspFMatrix4*)ProjectionMatrix[0], 0.0f, 480.0f, 272.0f, 0.0f, -1.0f, 1.0f );

	gumLoadIdentity( (ScePspFMatrix4*)ProjectionMatrix[1] );
	ScePspFVector3 displace = { -0.002f, 0.00367f, 0.0f };	// ~ 1/480, 1/272
	gumTranslate( (ScePspFMatrix4*)ProjectionMatrix[1], &displace );
	gumMultMatrix( (ScePspFMatrix4*)ProjectionMatrix[1], (ScePspFMatrix4*)ProjectionMatrix[1], (ScePspFMatrix4*)ProjectionMatrix[0] );

	sceGumMatrixMode(GU_PROJECTION);
	sceGumLoadMatrix( (ScePspFMatrix4*)ProjectionMatrix[0] );*/
	
	vram32 = (uint32*) (sceGeEdramGetAddr() + getStaticVramOffset());
	vram16 = (uint16*) vram32;
	line16 = (uint16*)targetIMG->data;
	line32 = (uint32*)line16;
	switch (format) {
		case DISPLAY_PIXEL_FORMAT_565:
			for (y = 0; y < targetIMG->texh; y++) {
				memcpy(line16, vram16, targetIMG->texw * 2);
				vram16 += BUF_WIDTH;
			}
			break;
		case DISPLAY_PIXEL_FORMAT_5551:
			for (y = 0; y < targetIMG->texh; y++) {
				memcpy(line16, vram16, targetIMG->texw * 2);
				for (x = 0; x < targetIMG->texw; x++) {
					*(line16++) |= 0x8000;
				}
				vram16 += BUF_WIDTH;
			}
			break;
		case DISPLAY_PIXEL_FORMAT_4444:
			for (y = 0; y < targetIMG->texh; y++) {
				memcpy(line16, vram16, targetIMG->texw * 2);
				for (x = 0; x < targetIMG->texw; x++) {
					*(line16++) |= 0xF000;
				}
				vram16 += BUF_WIDTH;
			}
			break;
		case DISPLAY_PIXEL_FORMAT_8888:
			for (y = 0; y < targetIMG->texh; y++) {
				memcpy(line32, vram32, targetIMG->texw * 4);
				for (x = 0; x < targetIMG->texw; x++) {
					*(line32++) |= 0xFF000000;
				}
				vram32 += BUF_WIDTH;
			}
			break;
	}
	//targetIMG->data = sceGeEdramGetAddr() + getStaticVramOffset();
	targetIMG->modified = 1;
	targetIMG = NULL;
	return;
}