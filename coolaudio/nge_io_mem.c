#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nge_io_mem.h"



typedef struct tagMemFile{
	const char* memptr;
	int			msize;
	int			mflag;
	int			mpos;
	int         eosflag;
}MFILE;


int io_mopen(const char* mem,int size,int flag)
{
	MFILE* mf;
	if(mem ==NULL || size==0)
		return 0;
	mf = (MFILE*)malloc(sizeof(MFILE));
	memset(mf,0,sizeof(MFILE));
	mf->memptr = mem;
	mf->msize  = size;
	mf->mflag  = IO_RDONLY;
	return (int)mf;
}

int io_mread(void* buffer,int count,int size,int handle)
{
	MFILE* mf = (MFILE*)handle;
	char* buf = buffer;
	int len = count*size;
	if(mf == NULL||len <= 0)
		return -1;
	if(mf->eosflag == 1)
		return -1;

	if(mf->msize-mf->mpos>=len){
		memcpy((char*)buffer,mf->memptr+mf->mpos,len);
		mf->mpos += len;
		if(mf->mpos == mf->msize)
			mf->eosflag = 1;
	}
	else{
		len = mf->msize-mf->mpos;
		memcpy((char*)buffer,mf->memptr+mf->mpos,len);
		mf->eosflag = 1;
	}
	return len;

}

int io_mwrite(void* buffer,int count,int size,int handle)
{
	MFILE* mf = (MFILE*)handle;
	if(mf == NULL)
		return 0;
	return 1;
}

int io_mseek(int handle,int offset,int flag)
{
	MFILE* mf = (MFILE*)handle;
	if(mf == NULL)
		return -1;
	switch(flag) 
	{
	case IO_SEEK_SET:
		mf->mpos = offset;
		if(mf->mpos>mf->msize)
			mf->mpos = mf->msize;
		if(mf->mpos <0)
			mf->mpos = 0;
		break;
	case IO_SEEK_END:
		mf->mpos = mf->msize - offset;
		if(mf->mpos>mf->msize)
			mf->mpos = mf->msize;
		if(mf->mpos <0)
			mf->mpos = 0;
		break;
	case IO_SEEK_CUR:
		mf->mpos +=offset;
		if(mf->mpos>mf->msize)
			mf->mpos = mf->msize;
		if(mf->mpos <0)
			mf->mpos = 0;
		break;
	default:
		break;
	}
	mf->eosflag = 0;
	return 0;
}

int io_mtell(int handle)
{
	MFILE* mf = (MFILE*)handle;
	if(mf == NULL)
		return -1;
	if(mf->eosflag)
		return -1;
	return mf->mpos;
}

int io_mclose(int handle)
{
	MFILE* mf = (MFILE*)handle;
	if(mf == NULL)
		return -1;
	free(mf);
	return 0;
}

int io_msize(int handle)
{
	MFILE* mf = (MFILE*)handle;
	if(mf == NULL)
		return -1;
	return mf->msize;
}




