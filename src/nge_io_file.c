#include "nge_platform.h"
#include "nge_debug_log.h"
#include "nge_io_file.h"
#include <stdio.h>
#include <stdlib.h>

int io_fopen(const char* fname,int flag)
{
#if defined NGE_PSP
	int fd = 0;
	if(flag == IO_RDONLY)
		fd = sceIoOpen(fname, PSP_O_RDONLY, 0777);
	else
		fd = sceIoOpen(fname, PSP_O_RDWR|PSP_O_CREAT, 0777);
	return fd <=0 ? 0: fd;
#else
	char io_array[][4]={"rb","wb","ab"};
	FILE* fp;
	if(flag > 3){
		flag = 0;
	}
	fp = fopen(fname,io_array[flag]);
	return (int)fp;
#endif
}

int io_fread(void* buffer,int count,int size,int handle)
{
#if defined NGE_PSP
	return sceIoRead(handle, (char*)buffer, size*count);
#else
	return fread((char*)buffer,count,size,(FILE*)handle);
#endif
}

int io_fwrite(void* buffer,int count,int size,int handle)
{
#if defined NGE_PSP
	return sceIoWrite(handle, (char*)buffer, size*count);
#else
	return fwrite((char*)buffer,count,size,(FILE*)handle);
#endif
}

int io_fseek(int handle,int offset,int flag)
{
#if defined NGE_PSP
	int ret = sceIoLseek(handle, offset, flag);
	if(ret >= 0)
		return 0;
	else
		return -1;
#else
	return fseek((FILE*)handle,offset,flag);
#endif
}

int io_ftell(int handle)
{
#if defined NGE_PSP
	int ret = sceIoLseek(handle, 0, IO_SEEK_CUR);
	return ret;
#else
	return ftell((FILE*)handle);
#endif
}

int io_fclose(int handle)
{
#if defined NGE_PSP
	if (handle >= 0)
		return sceIoClose(handle);
#else
	if (handle != 0)
		return fclose((FILE*)handle);
#endif
	return -1;
}


int io_fsize(int handle)
{
	int size,cur;
	//if(handle <= 0)
	//	return 0;
#if defined NGE_PSP
	cur = sceIoLseek(handle, 0, IO_SEEK_CUR);
	size = sceIoLseek(handle, 0, IO_SEEK_END);
	sceIoLseek(handle, cur, IO_SEEK_SET);
	return size;
#else
	cur = ftell((FILE*)handle);
	fseek((FILE*)handle,0,IO_SEEK_END);
	size = ftell((FILE*)handle);
	fseek((FILE*)handle,cur,IO_SEEK_SET);
	return size;
#endif
}
