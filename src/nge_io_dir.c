#include "nge_debug_log.h"
#include "nge_io_dir.h"
#ifdef WIN32
#include <windows.h>
#endif
#include <string.h>

//////////////////////////////////////////////////////////////////////////
//for win32
//////////////////////////////////////////////////////////////////////////
#ifdef WIN32
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
		if(FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY){
			dir_entry->flag = FIO_F_DIR;
			memcpy(dir_entry->name,FindFileData.cFileName,MAX_NAME);
		}
		else{
			memcpy(dir_entry->name,FindFileData.cFileName,MAX_NAME);
			dir_entry->flag = FIO_F_FILE;
		}

		return 1;
	}
	return 0;
}

static BOOL st_FindCloseFile(HANDLE handle )
{
	return (FindClose(handle)!=0);
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

int io_dir_travel(const char* directory, file_travel_cb callback,void* user_data)
{
	int ret_code = 0,find_code = 0;
	HANDLE handle;
	char dir_full[MAX_NAME] = {0};
	char buf[MAX_NAME] = {0};
	char work_buf[MAX_NAME] = {0};
	dir_desc_p	dir_entry = NULL;
	st_FixPath(dir_full,directory);
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
				strncpy(dir_entry->name,work_buf,MAX_NAME);

				if ( dir_entry->flag ==  FIO_F_DIR)
				{
					ret_code = io_dir_travel(dir_entry->name,callback,user_data);
				}
				callback(dir_entry,user_data);
				if(dir_entry){
					free(dir_entry);
					dir_entry = NULL;
				}
			}
		}
		st_FindCloseFile(handle);
	}
	return ret_code;
}

//////////////////////////////////////////////////////////////////////////
//for psp and linux
//////////////////////////////////////////////////////////////////////////
#elif defined _PSP || defined __linux__
#ifdef __linux__
#include <unistd.h>
#endif
#include <dirent.h>
#include <sys/stat.h>

static void   dir_scan(const char* path,const char* file,file_travel_cb callback,void* user_data)
{
	   struct   stat   s;
	   DIR           *dir;
	   struct   dirent   *dt;
	   char   dirname[256];
	   dir_desc_t  dir_entry;
	   memset(dirname,   0,   256);
	  strncpy(dirname,path,256);

	  if(stat(file,   &s)   <   0){
			   printf("stat   error!\n");
			   return;
	  }
	 if(S_ISDIR(s.st_mode)){
			strcpy(dirname+strlen(dirname),   file);
			strcpy(dirname+strlen(dirname),   "/");
			if((dir   =   opendir(file))   ==   NULL){
				   printf("opendir   %s   error!\n",file);
				   return;
			}
			if(chdir(file)   <   0)   {
				   printf("chdir   error!\n");
				   return;
			}
			while((dt   =   readdir(dir))   !=   NULL){
				   if(dt->d_name[0]   ==   '.'){
							   continue;
				   }
				  dir_scan(dirname,dt->d_name, callback,user_data);
		   }
		  if(chdir("..")   <   0){
				   printf("chdir   error!\n");
				   return;
		  }
		  closedir(dir);
		  memset(&dir_entry,0,sizeof(dir_desc_t));
	  sprintf(dir_entry.name,"%s",dirname);
	  if(dir_entry.name[strlen(dir_entry.name)-1]=='/'){
	  	dir_entry.name[strlen(dir_entry.name)-1]= 0;
			 if((strcmp(dir_entry.name,file)==0)&&(strlen(dir_entry.name)==strlen(file))){
				//current dir ommited
				return;
	  		 }
			 dir_entry.flag = FIO_F_DIR;
				 callback(&dir_entry,user_data);

	}

   }else{
		  memset(&dir_entry,0,sizeof(dir_desc_t));
	  sprintf(dir_entry.name,"%s%s",dirname,file);
	  dir_entry.flag = FIO_F_FILE;
		  callback(&dir_entry,user_data);
	}
}

int io_dir_travel(const char* directory, file_travel_cb callback,void* user_data)
{
		char workdir[MAX_NAME]={0};
		char backdir[MAX_NAME]={0};
		getcwd(backdir,MAX_NAME);
		if(directory==NULL)
				return 0;
		strncpy(workdir,directory,MAX_NAME);
		if(workdir[strlen(workdir)-1] == '/')
				workdir[strlen(workdir)-1] = 0;
		dir_scan("",workdir,callback,user_data);
		chdir(backdir);
		return 1;
}

#endif
