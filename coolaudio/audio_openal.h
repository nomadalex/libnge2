/**
 * @file  audio_openal.h
 * @author Kun Wang <ifreedom.cn@gmail.com>
 * @date 2012/03/22 16:45:51
 *
 *  Copyright  2012  Kun Wang <ifreedom.cn@gmail.com>
 *
 */

#ifndef _AUDIO_OPENAL_H
#define _AUDIO_OPENAL_H
#include "nge_common.h"
#include <AL/al.h>
#include <AL/alc.h>

#define BOOLEAN unsigned char
#define FALSE 0
#define TRUE 1

typedef struct PlayerOperation_s PlayerOperation;

typedef struct {
	PlayerOperation* op;
} IPlayer;

struct PlayerOperation_s {
	void (*CheckUpdate)(IPlayer* player);
};

#ifdef __cplusplus
extern "C" {
#endif

	BOOLEAN InitAL();
	void DeInitAL();
	inline void LockAudio();
	inline void UnlockAudio();
	void AddActivePlayer(IPlayer* player);
	void RemoveActivePlayer(IPlayer* player);

#ifdef __cplusplus
}
#endif

#endif /* _AUDIO_OPENAL_H */
