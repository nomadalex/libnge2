#ifndef DSOUNDOBJECT_H_
#define DSOUNDOBJECT_H_
#include <windows.h>
#include <Mmreg.h.>
#include <dsound.h>
#include <stdio.h>
#include "AudioType.h"

#define SIZE_DS_BUFFER 32 * 1024




typedef struct{
	HWND hWnd;
	IDirectSound* pDS;	
}SoundContext,*pSoundContext;

typedef struct  {
	DWORD				dThreadID;
	HANDLE				hNotifyThread;
	HANDLE				hEventNotify[3];
	DSBPOSITIONNOTIFY	sNotifyPos[2];
	WAVEFORMATEX		sFmtWave;
	IDirectSoundBuffer* pDsBuffer;
	IDirectSoundNotify* pDsNotify;
	DWORD				dWbpos;
	DWORD				dPbpos;
}BufferContext,*pBufferContext;

class IDSoundObject
{
public:
	IDSoundObject(SoundType type);
	virtual ~IDSoundObject();
	
	static int OpenAudio(HWND hwnd);
	static int CloseAudio();

	virtual int  Load(const char* file) = 0;
	virtual void ReWind();
	virtual int  Play();
	virtual int  Pause();
	virtual int  Resume();
	virtual int  Stop();
	virtual int  IsPlaying() const;
	virtual int	 IsEos() const;
	virtual void SetVolume(long vol);
	virtual int  GetVolume() const;
	virtual int  SetPlayPos(unsigned int pos) = 0;
	//virtual bool IsReady() const;
	//virtual double Duration() = 0;
	//virtual unsigned int GetPlayPos() const = 0;
	//virtual int GetFormatTag() const;
	/*virtual int GetChannels() const;
	virtual int GetSamples() const;
	virtual int GetBPS() const;
	virtual int GetBlockAlign() const;
	virtual int GetABPS() const;*/
protected:
	//thread handle
	int CreateNotifyThread();
	void ReleaseNotifyThread();
	static DWORD NotifyHandleProc(LPVOID param);
	virtual void ReleaseDSBuffer();
	virtual int  CreateDSBuffer();
	//stream
	virtual int InitDSData() = 0;
	virtual int LoadDSData(unsigned int start, unsigned int count) = 0;
	static pSoundContext mHandle;
	static int			 mOpened;
	pBufferContext		 mBufContext;
	SoundType			 mType;
	int					 mPos;
	volatile int		 mIsPlaying;
	volatile int		 mIsPaused;
	volatile int		 mIsEos;
private:
};


#endif
