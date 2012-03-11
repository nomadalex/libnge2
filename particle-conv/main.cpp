#include "libnge2.h"
#include "hgeparticle.h"
#include <stdlib.h>
#include <stdio.h>
/**
 * 粒子文件转换为新格式
 */
struct hgeParticleSystemInfo_Old
{
	sprite_p	sprite;    // texture+rectf
	int			nEmission; // particles per sec
	float		fLifetime;

	float		fParticleLifeMin;
	float		fParticleLifeMax;

	float		fDirection;
	float		fSpread;
	int /*BOOL*/ bRelative;

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

#define COPY_PROP(prop) \
	info.prop = old_info.prop

#define COPY_FLOATS(prop, size) \
	memcpy(&info.prop, &old_info.prop, size * sizeof(float))

#undef main
extern "C"
int main(int argc, char* argv[])
{
	hgeParticleSystemInfo info;
	hgeParticleSystemInfo_Old old_info;

	if (argc != 3)
		return -1;

	char* filename = argv[1];
	char* outname = argv[2];

	int handle = io_fopen(filename,IO_RDONLY);
	if(handle==0)
		return -1;
	io_fread(&old_info,1,sizeof(hgeParticleSystemInfo_Old),handle);
	io_fclose(handle);

	COPY_PROP(nEmission);
	COPY_FLOATS(fLifetime, 5);
	info.bRelative = (uint8)old_info.bRelative;
	COPY_FLOATS(fSpeedMin, 14);
	COPY_PROP(colColorStart);
	COPY_PROP(colColorEnd);
	COPY_FLOATS(fColorVar, 2);

	if (!hgeParticleSystem::SaveInfoToFile(info, outname))
		return -1;
	return 0;
}
