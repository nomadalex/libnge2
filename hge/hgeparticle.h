/*
** Haaf's Game Engine 1.7
** Copyright (C) 2003-2007, Relish Games
** hge.relishgames.com
**
** hgeParticleSystem helper class header
*/
#ifndef HGEPARTICLE_H
#define HGEPARTICLE_H

#include "nge_common.h"
#include "hgevector.h"
#include "hgecolor.h"
#include "hgerect.h"
#include "nge_graphics.h"
#include <string.h>

#define MAX_PARTICLES	500
#define MAX_PSYSTEMS	100
#ifndef M_PI_2
#define M_PI_2	1.57079632679489661923f
#endif

typedef struct {
	image_p sprite_image;
	rectf   sprite_clip;
	pointf  sprite_center;
	int     sprite_src_blend;
	int     sprite_des_blend;
}sprite_t,*sprite_p;

struct hgeParticle
{
	hgeVector	vecLocation;
	hgeVector	vecVelocity;

	float		fGravity;
	float		fRadialAccel;
	float		fTangentialAccel;

	float		fSpin;
	float		fSpinDelta;

	float		fSize;
	float		fSizeDelta;

	hgeColor	colColor;		// + alpha
	hgeColor	colColorDelta;

	float		fAge;
	float		fTerminalAge;
};

struct hgeParticleSystemInfo
{
	sprite_p	sprite;    // texture+rectf
	int			nEmission; // particles per sec
	float		fLifetime;

	float		fParticleLifeMin;
	float		fParticleLifeMax;

	float		fDirection;
	float		fSpread;
	uint8 /*BOOL*/ bRelative;

	float		fSpeedMin;
	float		fSpeedMax;

	float		fGravityMin;
	float		fGravityMax;

	float		fRadialAccelMin;
	float		fRadialAccelMax;

	float		fTangentialAccelMin;
	float		fTangentialAccelMax;

	float		fSizeStart;
	float		fSizeEnd;
	float		fSizeVar;

	float		fSpinStart;
	float		fSpinEnd;
	float		fSpinVar;

	hgeColor	colColorStart; // + alpha
	hgeColor	colColorEnd;
	float		fColorVar;
	float		fAlphaVar;
};

class NGE_CLASS hgeParticleSystem
{
public:
	hgeParticleSystemInfo info;

	hgeParticleSystem(const char *filename, sprite_p sprite);
	hgeParticleSystem(hgeParticleSystemInfo *psi);
	hgeParticleSystem(const hgeParticleSystem &ps);
	~hgeParticleSystem() { }

	hgeParticleSystem&	operator= (const hgeParticleSystem &ps);

	static bool ReadInfoFromBuf(hgeParticleSystemInfo& info, const uint8* mem, uint32 len);
	static bool ReadInfoFromFile(hgeParticleSystemInfo& info, const char* filename);
	static uint32 SaveInfoToBuf(const hgeParticleSystemInfo& info, uint8* mem, uint32 len);
	static bool SaveInfoToFile(hgeParticleSystemInfo& info, const char* filename);

	void				Render();
	void				FireAt(float x, float y);
	void				Fire();
	void				Stop(BOOL bKillParticles = 0);
	void				Update(float fDeltaTime);
	void				MoveTo(float x, float y, BOOL bMoveParticles = 0);
	void				Transpose(float x, float y) { fTx=x; fTy=y; }
	void				TrackBoundingBox(BOOL bTrack) { bUpdateBoundingBox=bTrack; }

	int					GetParticlesAlive() const { return nParticlesAlive; }
	float				GetAge() const { return fAge; }
	void				GetPosition(float *x, float *y) const { *x=vecLocation.x; *y=vecLocation.y; }
	void				GetTransposition(float *x, float *y) const { *x=fTx; *y=fTy; }
	hgeRect*			GetBoundingBox(hgeRect *rect) const { memcpy(rect, &rectBoundingBox, sizeof(hgeRect)); return rect; }

	static const int infoSize;

private:
	hgeParticleSystem();

	float				fAge;
	float				fEmissionResidue;

	hgeVector			vecPrevLocation;
	hgeVector			vecLocation;
	float				fTx, fTy;

	int					nParticlesAlive;
	hgeRect				rectBoundingBox;
	BOOL				bUpdateBoundingBox;

	hgeParticle			particles[MAX_PARTICLES];

	float				mTimer;
};

class NGE_CLASS hgeParticleManager
{
public:
	hgeParticleManager();
	~hgeParticleManager();

	void				Update(float dt);
	void				Render();

	hgeParticleSystem*	SpawnPS(hgeParticleSystemInfo *psi, float x, float y);
	BOOL				IsPSAlive(hgeParticleSystem *ps) const;
	void				Transpose(float x, float y);
	void				GetTransposition(float *dx, float *dy) const {*dx=tX; *dy=tY;}
	void				KillPS(hgeParticleSystem *ps);
	void				KillAll();

private:
	hgeParticleManager(const hgeParticleManager &);
	hgeParticleManager&	operator= (const hgeParticleManager &);

	int					nPS;
	float				tX;
	float				tY;
	hgeParticleSystem*	psList[MAX_PSYSTEMS];
};


#endif
