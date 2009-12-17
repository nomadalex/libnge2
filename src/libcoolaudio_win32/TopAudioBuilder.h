#ifndef TOPAUDIOBUILDER_H_
#define TOPAUDIOBUILDER_H_
#include "AudioType.h"
#include "DSoundObject.h"
class IDSoundObject;

class CTopAudioBuilder
{
public:
	CTopAudioBuilder();
	CTopAudioBuilder(int hwnd);
	~CTopAudioBuilder();
	IDSoundObject* BuildSoundObject(int type);
protected:
private:
};

#endif
