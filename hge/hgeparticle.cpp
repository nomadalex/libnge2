/*
 * Thanks to Dr.Watson JGE++!
** Haaf's Game Engine 1.7
** Copyright (C) 2003-2007, Relish Games
** hge.relishgames.com
**
** hgeParticleSystem helper class implementation
*/

#include "hgeparticle.h"
#include "nge_io_file.h"
#include "nge_graphics.h"
#include <time.h>
#include <stddef.h>

unsigned int g_seed=0;

void Random_Seed(int seed)
{
	if(!seed) g_seed=0;//time(NULL);
	else g_seed=seed;
}

int Random_Int(int min, int max)
{
	g_seed=214013*g_seed+2531011;
	return min+(g_seed ^ g_seed>>15)%(max-min+1);
}

float Random_Float(float min, float max)
{
	g_seed=214013*g_seed+2531011;
	return min+(g_seed>>16)*(1.0f/65535.0f)*(max-min);
}

#define OP_LITTLEENDIAN '<'
#define OP_BIGENDIAN    '>'
#define OP_NATIVE       '='

inline static int doendian(int c)
{
	int x=1;
	int e=*(char*)&x;
	if (c==OP_LITTLEENDIAN) return !e;
	if (c==OP_BIGENDIAN) return e;
	if (c==OP_NATIVE) return 0;
	return 0;
}

inline static void doswap(int swap, void *p, size_t n)
{
	if (swap)
	{
		char *a=(char*)p;
		int i,j;
		for (i=0, j=n-1, n=n/2; n--; i++, j--)
		{
			char t=a[i]; a[i]=a[j]; a[j]=t;
		}
	}
}

#define DECL_ENDIAN() uint8 __swap__ = 0
#define SET_ENDIAN(endian) __swap__ = doendian(endian)

#define PUSH_STRUCT(type, ptr, mem)				\
	memcpy(mem, (ptr), sizeof(type));			\
	mem += sizeof(type)
#define POP_STRUCT(type, ptr, mem)				\
	memcpy((ptr), mem, sizeof(type));			\
	mem += sizeof(type)

#define PUSH_UINT8(val, mem)					\
	mem[0] = (uint8)val;						\
	mem += 1
#define POP_UINT8(val, mem)						\
	val = mem[0];								\
	mem += 1

#define PUSH_FLOAT(val, mem)					\
	memcpy(&(mem[0]), &(val), sizeof(float));	\
	mem += sizeof(float)
#define POP_FLOAT(val, mem)						\
	memcpy(&(val), &(mem[0]), sizeof(float));	\
	mem += sizeof(float)

#define PUSH_FLOATS(ptr, mem, size)					\
	memcpy(&(mem[0]), ptr, size * sizeof(float));	\
	mem += size * sizeof(float)
#define POP_FLOATS(ptr, mem, size)					\
	memcpy(ptr, &(mem[0]), size * sizeof(float));	\
	mem += size * sizeof(float)

#define PUSH_TYPE(type, val, mem)				\
	memcpy(&(mem[0]), &(val), sizeof(type));	\
	doswap(__swap__, &(mem[0]), sizeof(type));	\
	mem += sizeof(type)
#define POP_TYPE(type, val, mem)				\
	memcpy(&(val), &(mem[0]), sizeof(type));	\
	doswap(__swap__, &(val), sizeof(type));		\
	mem += sizeof(type)

const int hgeParticleSystem::infoSize = 3 + 4 + 5*4 + 1 + 14*4 + sizeof(hgeColor) * 2 + 2*4;

bool hgeParticleSystem::ReadInfoFromBuf(hgeParticleSystemInfo& info, const uint8* mem, uint32 len) {
	DECL_ENDIAN();
	uint8 hdr[3];

	SET_ENDIAN(OP_LITTLEENDIAN);

	if (len < (uint32)infoSize)
		return false;
	memset(&info, 0, sizeof(hgeParticleSystemInfo));

	POP_UINT8(hdr[0], mem);
	POP_UINT8(hdr[1], mem);
	POP_UINT8(hdr[2], mem);
	if (hdr[0] != 'P' || hdr[1] != 'A' || hdr[2] != 'R')
		return false;

	// POP_STRUCT(rectf, &info.sprite->sprite_clip, mem);
	// POP_STRUCT(pointf, &info.sprite->sprite_center, mem);
	POP_TYPE(uint32, info.nEmission, mem);
	POP_FLOATS(&info.fLifetime, mem, 5);
	POP_UINT8(info.bRelative, mem);
	POP_FLOATS(&info.fSpeedMin, mem, 14);
	POP_STRUCT(hgeColor, &info.colColorStart, mem);
	POP_STRUCT(hgeColor, &info.colColorEnd, mem);
	POP_FLOATS(&info.fColorVar, mem, 2);
	return true;
}

bool hgeParticleSystem::ReadInfoFromFile(hgeParticleSystemInfo& info, const char* filename) {
	uint8 mem[infoSize];

	int handle = io_fopen(filename,IO_RDONLY);
	if(handle==0)
		return false;
	io_fread(mem,1,infoSize,handle);
	io_fclose(handle);

	return ReadInfoFromBuf(info, mem, infoSize);
}

uint32 hgeParticleSystem::SaveInfoToBuf(const hgeParticleSystemInfo& info, uint8* mem, uint32 len) {
	DECL_ENDIAN();

	SET_ENDIAN(OP_LITTLEENDIAN);

	if (len < (uint32)infoSize)
		return 0;

	PUSH_UINT8('P', mem);
	PUSH_UINT8('A', mem);
	PUSH_UINT8('R', mem);

	// PUSH_STRUCT(rectf, &info.sprite->sprite_clip, mem);
	// PUSH_STRUCT(pointf, &info.sprite->sprite_center, mem);
	PUSH_TYPE(uint32, info.nEmission, mem);
	PUSH_FLOATS(&info.fLifetime, mem, 5);
	PUSH_UINT8(info.bRelative, mem);
	PUSH_FLOATS(&info.fSpeedMin, mem, 14);
	PUSH_STRUCT(hgeColor, &info.colColorStart, mem);
	PUSH_STRUCT(hgeColor, &info.colColorEnd, mem);
	PUSH_FLOATS(&info.fColorVar, mem, 2);

	return infoSize;
}

bool hgeParticleSystem::SaveInfoToFile(hgeParticleSystemInfo& info, const char* filename) {
	uint8 mem[infoSize];

	int handle = io_fopen(filename,IO_WRONLY);
	if(handle==0)
		return false;

	uint32 ret = SaveInfoToBuf(info, mem, infoSize);
	if (ret == 0)
		return false;

	io_fwrite(mem,1,infoSize,handle);
	io_fclose(handle);

	return true;
}

hgeParticleSystem::hgeParticleSystem(const char *filename, sprite_p sprite)
{
	ReadInfoFromFile(info, filename);

	Random_Seed(0);
	info.sprite=sprite;
	if(info.sprite->sprite_src_blend == 0)
		info.sprite->sprite_src_blend = BLEND_SRC_ALPHA;
	if(info.sprite->sprite_des_blend == 0)
		info.sprite->sprite_des_blend = BLEND_ONE;

	vecLocation.x=vecPrevLocation.x=0.0f;
	vecLocation.y=vecPrevLocation.y=0.0f;
	fTx=fTy=0;

	fEmissionResidue=0.0f;
	nParticlesAlive=0;
	fAge=-2.0;

	rectBoundingBox.Clear();
	bUpdateBoundingBox=false;
}

hgeParticleSystem::hgeParticleSystem(hgeParticleSystemInfo *psi)
{
	memcpy(&info, psi, sizeof(hgeParticleSystemInfo));

	vecLocation.x=vecPrevLocation.x=0.0f;
	vecLocation.y=vecPrevLocation.y=0.0f;
	fTx=fTy=0;

	fEmissionResidue=0.0f;
	nParticlesAlive=0;
	fAge=-2.0;

	rectBoundingBox.Clear();
	bUpdateBoundingBox=false;
}

hgeParticleSystem::hgeParticleSystem(const hgeParticleSystem &ps)
{
	memcpy(this, &ps, sizeof(hgeParticleSystem));
}

void hgeParticleSystem::Update(float fDeltaTime)
{
	int i;
	float ang;
	hgeParticle *par;
	hgeVector vecAccel, vecAccel2;
	if(fAge >= 0)
	{
		fAge += fDeltaTime;
		if(fAge >= info.fLifetime) fAge = -2.0f;
	}

	mTimer += fDeltaTime;
	if (mTimer < 0.01f)
		return;

	fDeltaTime = mTimer;
	mTimer = 0.0f;


	// update all alive particles

	if(bUpdateBoundingBox) rectBoundingBox.Clear();
	par=particles;

	for(i=0; i<nParticlesAlive; i++)
	{
		par->fAge += fDeltaTime;
		if(par->fAge >= par->fTerminalAge)
		{
			nParticlesAlive--;
			memcpy(par, &particles[nParticlesAlive], sizeof(hgeParticle));
			i--;
			continue;
		}

		vecAccel = par->vecLocation-vecLocation;
		vecAccel.Normalize();
		vecAccel2 = vecAccel;
		vecAccel *= par->fRadialAccel;

		// vecAccel2.Rotate(M_PI_2);
		// the following is faster
		ang = vecAccel2.x;
		vecAccel2.x = -vecAccel2.y;
		vecAccel2.y = ang;

		vecAccel2 *= par->fTangentialAccel;
		par->vecVelocity += (vecAccel+vecAccel2)*fDeltaTime;
		par->vecVelocity.y += par->fGravity*fDeltaTime;

		//par->vecVelocity.y = 0.1f;
		par->vecLocation += par->vecVelocity;

		par->fSpin += par->fSpinDelta*fDeltaTime;
		par->fSize += par->fSizeDelta*fDeltaTime;
		par->colColor += par->colColorDelta*fDeltaTime;

		if(bUpdateBoundingBox) rectBoundingBox.Encapsulate(par->vecLocation.x, par->vecLocation.y);

		par++;
	}

	// generate new particles

	if(fAge != -2.0f)
	{
		float fParticlesNeeded = info.nEmission*fDeltaTime + fEmissionResidue;
		int nParticlesCreated = (unsigned int)fParticlesNeeded;
		fEmissionResidue=fParticlesNeeded-nParticlesCreated;

		par=&particles[nParticlesAlive];

		for(i=0; i<nParticlesCreated; i++)
		{
			if(nParticlesAlive>=MAX_PARTICLES) break;

			par->fAge = 0.0f;
			par->fTerminalAge = Random_Float(info.fParticleLifeMin, info.fParticleLifeMax);

			par->vecLocation = vecPrevLocation+(vecLocation-vecPrevLocation)*Random_Float(0.0f, 1.0f);
			par->vecLocation.x += Random_Float(-2.0f, 2.0f);
			par->vecLocation.y += Random_Float(-2.0f, 2.0f);

			ang=info.fDirection-M_PI_2+Random_Float(0,info.fSpread)-info.fSpread/2.0f;
			if(info.bRelative) ang += (vecPrevLocation-vecLocation).Angle()+M_PI_2;
			par->vecVelocity.x = cosf(ang);
			par->vecVelocity.y = sinf(ang);
			par->vecVelocity *= Random_Float(info.fSpeedMin, info.fSpeedMax);

			par->fGravity = Random_Float(info.fGravityMin, info.fGravityMax);
			par->fRadialAccel = Random_Float(info.fRadialAccelMin, info.fRadialAccelMax);
			par->fTangentialAccel = Random_Float(info.fTangentialAccelMin, info.fTangentialAccelMax);

			par->fSize = Random_Float(info.fSizeStart, info.fSizeStart+(info.fSizeEnd-info.fSizeStart)*info.fSizeVar);
			par->fSizeDelta = (info.fSizeEnd-par->fSize) / par->fTerminalAge;

			par->fSpin = Random_Float(info.fSpinStart, info.fSpinStart+(info.fSpinEnd-info.fSpinStart)*info.fSpinVar);
			par->fSpinDelta = (info.fSpinEnd-par->fSpin) / par->fTerminalAge;

			par->colColor.r = Random_Float(info.colColorStart.r, info.colColorStart.r+(info.colColorEnd.r-info.colColorStart.r)*info.fColorVar);
			par->colColor.g = Random_Float(info.colColorStart.g, info.colColorStart.g+(info.colColorEnd.g-info.colColorStart.g)*info.fColorVar);
			par->colColor.b = Random_Float(info.colColorStart.b, info.colColorStart.b+(info.colColorEnd.b-info.colColorStart.b)*info.fColorVar);
			par->colColor.a = Random_Float(info.colColorStart.a, info.colColorStart.a+(info.colColorEnd.a-info.colColorStart.a)*info.fAlphaVar);

			par->colColorDelta.r = (info.colColorEnd.r-par->colColor.r) / par->fTerminalAge;
			par->colColorDelta.g = (info.colColorEnd.g-par->colColor.g) / par->fTerminalAge;
			par->colColorDelta.b = (info.colColorEnd.b-par->colColor.b) / par->fTerminalAge;
			par->colColorDelta.a = (info.colColorEnd.a-par->colColor.a) / par->fTerminalAge;

			if(bUpdateBoundingBox) rectBoundingBox.Encapsulate(par->vecLocation.x, par->vecLocation.y);

			nParticlesAlive++;
			par++;
		}
	}

	vecPrevLocation=vecLocation;
}

void hgeParticleSystem::MoveTo(float x, float y, BOOL bMoveParticles)
{
	int i;
	float dx,dy;
	x=x-info.sprite->sprite_center.x;
	y=y-info.sprite->sprite_center.y;
	if(bMoveParticles)
	{
		dx=x-vecLocation.x;
		dy=y-vecLocation.y;

		for(i=0;i<nParticlesAlive;i++)
		{
			particles[i].vecLocation.x += dx;
			particles[i].vecLocation.y += dy;
		}

		vecPrevLocation.x=vecPrevLocation.x + dx;
		vecPrevLocation.y=vecPrevLocation.y + dy;
	}
	else
	{
		if(fAge==-2.0) { vecPrevLocation.x=x; vecPrevLocation.y=y; }
		else { vecPrevLocation.x=vecLocation.x;	vecPrevLocation.y=vecLocation.y; }
	}

	vecLocation.x=x;
	vecLocation.y=y;
}

void hgeParticleSystem::FireAt(float x, float y)
{
	Stop();
	MoveTo(x,y,false);
	Fire();
}

void hgeParticleSystem::Fire()
{
	mTimer = 0.0f;

	if(info.fLifetime==-1.0f) fAge=-1.0f;
	else fAge=0.0f;
}

void hgeParticleSystem::Stop(BOOL bKillParticles)
{
	fAge=-2.0f;
	if(bKillParticles)
	{
		nParticlesAlive=0;
		rectBoundingBox.Clear();
	}
}

void hgeParticleSystem::Render()
{
	int i;
 	hgeParticle *par=particles;
	rectf *prectf = &info.sprite->sprite_clip;
	info.sprite->sprite_image->rcentrex = info.sprite->sprite_center.x;
	info.sprite->sprite_image->rcentrey = info.sprite->sprite_center.y;
	SetTexBlend(info.sprite->sprite_src_blend,info.sprite->sprite_des_blend);
	for(i=0; i<nParticlesAlive; i++)
	{
		RenderQuad(info.sprite->sprite_image,
			       (float)prectf->left,(float)prectf->top,(float)prectf->right-(float)prectf->left,(float)prectf->bottom-(float)prectf->top,//子图4元组
			       (float)(par->vecLocation.x+fTx), (float)(par->vecLocation.y+fTy), //屏幕坐标dx,dy
				   (float)par->fSize, (float)par->fSize,//size scale
				   (float)(par->fSpin*par->fAge*360),//angel rot
				   (int)(par->colColor.GetHWColor())//color mask
				   );
		par++;
	}
	ResetTexBlend();
}
