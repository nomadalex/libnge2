#include "nge_dirent.hpp"
#include <iostream>
#include <algorithm>

void NGEDirent::closeDirent()
{
	vec.clear();
}

void NGEDirent::sortDirent()
{
	vector<DIR> dirVec;
	vector<DIR> fileVec;

	vector<DIR>::iterator p=vec.begin();

	while( p!=vec.end() )
	{
		if( (*p).getType()==IS_DIR )
		{
			dirVec.push_back
				(
					DIR( (*p).getName() , (*p).getPath() , (*p).getSize() ,IS_DIR,(*p).getCreatTime(),(*p).getModityTime() )
				);
		}
		else
		{
			fileVec.push_back
				(
					DIR( (*p).getName() , (*p).getPath() , (*p).getSize() ,IS_FILE,(*p).getCreatTime(),(*p).getModityTime() )
				);
		}

		p++;
	}

	sort( dirVec.begin(),dirVec.end(),DIRComparatorByName());
	sort( fileVec.begin(),fileVec.end(),DIRComparatorByName());


	vec.clear();

	int i;
	for(i=0;i<dirVec.size();i++ )
	{
		vec.push_back
			(
				DIR( dirVec[i].getName() , dirVec[i].getPath() , dirVec[i].getSize() ,IS_DIR,dirVec[i].getCreatTime(),dirVec[i].getModityTime() )
			);
	}

	for(i=0;i<fileVec.size();i++ )
	{
		vec.push_back
			(
				DIR( fileVec[i].getName() , fileVec[i].getPath() , fileVec[i].getSize() ,IS_FILE,fileVec[i].getCreatTime(),fileVec[i].getModityTime() )
			);
	}

	dirVec.clear();
	fileVec.clear();
}

void NGEDirent::sortDirentByName()
{
	sort( vec.begin(),vec.end(),DIRComparatorByName());
}

void NGEDirent::sortDirentByType()
{
	sort( vec.begin(),vec.end(),DIRComparatorByType());
}

void NGEDirent::sortDirentBySize()
{
	sort( vec.begin(),vec.end(),DIRComparatorBySize());
}

void NGEDirent::sortDirentByCreatTime()
{
	sort( vec.begin(),vec.end(),DIRComparatorByCreatTime());
}

void NGEDirent::sortDirentByModityTime()
{
	sort( vec.begin(),vec.end(),DIRComparatorByModityTime());
}
//////////////////////////////////////////////////////////////////////////
//for win32 采用nge_io_dir修改
//////////////////////////////////////////////////////////////////////////

#ifdef WIN32

#include <windows.h>

#define MAX_NAME 256

enum FileFlags
{
	FIO_F_DIR,//dir
	FIO_F_FILE//file
};

typedef struct
{
	int    flag;// FileFlags
	char   name[MAX_NAME];// name
	char   path[MAX_NAME];// path
	int	   size;

	file_time ctime;
	file_time mtime;
}dir_desc_t,*dir_desc_p;


static void st_GetProperty(dir_desc_p dir_entry,const char* path)
{
	HANDLE hFile;
	//*********************************************
	//[获取文件和文件夹属性]
	hFile = CreateFile(path,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL|FILE_FLAG_BACKUP_SEMANTICS,
			NULL);

	if(hFile != INVALID_HANDLE_VALUE)
	{
		FILETIME ftCreate,ftModify,ftAccess,ftLocal;

		SYSTEMTIME sysTime;

		GetFileTime(hFile, &ftCreate, &ftAccess, &ftModify);

		//创建时间
		FileTimeToLocalFileTime(&ftCreate, &ftLocal);
		FileTimeToSystemTime(&ftLocal, &sysTime);

		dir_entry->ctime.year=sysTime.wYear;
		dir_entry->ctime.month=sysTime.wMonth;
		dir_entry->ctime.day=sysTime.wDay;

		dir_entry->ctime.hour=sysTime.wHour;
		dir_entry->ctime.min=sysTime.wMinute;
		dir_entry->ctime.sec=sysTime.wSecond;

		//修改时间
		FileTimeToLocalFileTime(&ftModify, &ftLocal);
		FileTimeToSystemTime(&ftLocal, &sysTime);

		dir_entry->mtime.year=sysTime.wYear;
		dir_entry->mtime.month=sysTime.wMonth;
		dir_entry->mtime.day=sysTime.wDay;

		dir_entry->mtime.hour=sysTime.wHour;
		dir_entry->mtime.min=sysTime.wMinute;
		dir_entry->mtime.sec=sysTime.wSecond;

		CloseHandle(hFile);
	}
	else
	{
		//创建时间
		dir_entry->ctime.year=0;
		dir_entry->ctime.month=0;
		dir_entry->ctime.day=0;

		dir_entry->ctime.hour=0;
		dir_entry->ctime.min=0;
		dir_entry->ctime.sec=0;

		//修改时间
		dir_entry->mtime.year=0;
		dir_entry->mtime.month=0;
		dir_entry->mtime.day=0;

		dir_entry->mtime.hour=0;
		dir_entry->mtime.min=0;
		dir_entry->mtime.sec=0;
	}
}

//fix path
static void st_FixPath(char* desstr,const char* srcstr)
{
	int srclen = strlen(srcstr);
	strncpy(desstr,srcstr,srclen);
	if(srcstr[srclen-1] == '/')
		strcat(desstr,"*");
	else
		strcat(desstr,"/*");
}

static HANDLE st_FindFirstFile(const char* path)
{
	//win32
	WIN32_FIND_DATA FindFileData;
	HANDLE handle = FindFirstFile(path, &FindFileData);
	return handle;
}


static BOOL st_FindNextFile(dir_desc_p dir_entry, HANDLE handle )
{
	WIN32_FIND_DATA FindFileData;
	const int error_code = FindNextFile(handle,&FindFileData);

	if ( error_code != 0 )
	{
		if(FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
		{
			memcpy(dir_entry->name,FindFileData.cFileName,MAX_NAME);
			memcpy(dir_entry->path,FindFileData.cFileName,MAX_NAME);

			dir_entry->flag = FIO_F_DIR;
			dir_entry->size=FindFileData.nFileSizeLow;
		}
		else
		{
			memcpy(dir_entry->name,FindFileData.cFileName,MAX_NAME);
			memcpy(dir_entry->path,FindFileData.cFileName,MAX_NAME);
			dir_entry->flag = FIO_F_FILE;

			dir_entry->size=FindFileData.nFileSizeLow;
		}

		return 1;
	}
	return 0;
}

static BOOL st_FindCloseFile(HANDLE handle )
{
	return (FindClose(handle)!=0);
}

int NGEDirent::openDirent(string directory,vector<DIR> &vec)
{
	//等待重写
	return 1;
}
int NGEDirent::openDirent(string directory)
{
	currentPath=directory;

	int ret_code = 0,find_code = 0;
	HANDLE handle;
	char dir_full[MAX_NAME] = {0};
	char buf[MAX_NAME] = {0};
	char work_buf[MAX_NAME] = {0};
	dir_desc_p	dir_entry = NULL;
	st_FixPath(dir_full,directory.c_str());

	if ((handle = st_FindFirstFile(dir_full)) != (void*)-1 )
	{
		ret_code = 1;
		find_code = 1;
		while (find_code && ret_code)
		{
			dir_entry = (dir_desc_p)malloc(sizeof(dir_desc_t));
			memset(dir_entry,0,sizeof(dir_desc_t));
			find_code = (st_FindNextFile(dir_entry,handle) == 1);

			if(find_code == 0)
				continue;

			if ( strcmp(dir_entry->name, ".")!=0 && strcmp(dir_entry->name ,"..")!=0 )
			{
				strncpy(buf,dir_full,strlen(dir_full)-2);
				memset(work_buf,0,MAX_NAME);
				strcat(work_buf,buf);
				strcat(work_buf,"/");
				strcat(work_buf,dir_entry->name);
				strncpy(dir_entry->path,work_buf,MAX_NAME);

				if( dir_entry->flag == FIO_F_DIR )
				{
					st_GetProperty(dir_entry,dir_entry->path);
					vec.push_back( DIR( dir_entry->name ,dir_entry->path,dir_entry->size,IS_DIR,dir_entry->ctime,dir_entry->mtime) );
				}
				else
				{
					st_GetProperty(dir_entry,dir_entry->path);
					vec.push_back( DIR( dir_entry->name ,dir_entry->path,dir_entry->size,IS_FILE,dir_entry->ctime,dir_entry->mtime) );
				}

				if(dir_entry)
				{
					free(dir_entry);
					dir_entry = NULL;
				}
			}
		}

		st_FindCloseFile(handle);
	}
	return ret_code;
}

int NGEDirent::openDeepDirent(string directory)
{
	//等待重写
	return 1;
}


NGEDirent::NGEDirent()
{
	cout<< "NGEDirent init" <<endl;
}


NGEDirent::~NGEDirent()
{
	cout<< "NGEDirent destroy" <<endl;
	closeDirent();
}


//////////////////////////////////////////////////////////////////////////
//for PSP
//////////////////////////////////////////////////////////////////////////

#elif defined _PSP
#include <pspkernel.h>
#include "fat.h"
#include "directory.h"
#include "miniconv.h"
#include "nge_charsets.h"

NGEDirent::NGEDirent()
{
	//初始化
	fat_init(sceKernelDevkitVersion());
}


NGEDirent::~NGEDirent()
{
	closeDirent();
	fat_free();
}


int NGEDirent::openDirent(string directory,vector<DIR> &vec)
{
	return 1;
}

int NGEDirent::openDirent(string directory)
{
	//保存当前工作目录 getcwd()
	currentPath=directory;

	char spath[256];

	directory_item_struct* dirItems = NULL;

	int itemNum = open_ms0_directory(directory.c_str(),spath,&dirItems);

	if (itemNum <= 0)
	{
		return -1;
	}

	string shortpath=spath;
	if ( shortpath.at(shortpath.length()-1)!='/' )
	{
		shortpath=shortpath+"/";
	}

	for(int i=0;i<itemNum;i++)
	{
		if (dirItems[i].longname != 0 && dirItems[i].longname[0] != '.')
		{
			//[shortpath]
			string shortFullPath=shortpath+dirItems[i].shortname;

			//[longname] UTF-8->
			charsets_utf8_conv(    (const uint8 *) (dirItems[i].longname),   (uint8 *) dirItems[i].longname);

			//创建 修改时间
			file_time ctime;
			file_time mtime;
			ctime.year=(dirItems[i].cdate >> 9) + 1980;
			ctime.month=(dirItems[i].cdate & 0x01FF) >> 5;
			ctime.day=dirItems[i].cdate & 0x01F;

			ctime.hour=dirItems[i].ctime >> 11;
			ctime.min=(dirItems[i].ctime & 0x07FF) >> 5;
			ctime.sec=(dirItems[i].ctime & 0x01F) * 2;

			mtime.year=(dirItems[i].mdate >> 9) + 1980;
			mtime.month=(dirItems[i].mdate & 0x01FF) >> 5;
			mtime.day=dirItems[i].mdate & 0x01F;

			mtime.hour=dirItems[i].mtime >> 11;
			mtime.min=(dirItems[i].mtime & 0x07FF) >> 5;
			mtime.sec=(dirItems[i].mtime & 0x01F) * 2;


			if (dirItems[i].filetype == FS_DIRECTORY)
			{
				vec.push_back
					(
						DIR( dirItems[i].longname , shortFullPath ,dirItems[i].filesize,IS_DIR,ctime,mtime)
					);
			}
			else
			{
				vec.push_back
					(
						DIR( dirItems[i].longname , shortFullPath ,dirItems[i].filesize,IS_FILE,ctime,mtime)
					);
			}
		}
	}

	free(dirItems);

	return vec.size();
}

#endif
