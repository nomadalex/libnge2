
#ifndef _PACKAGE_CPP_
#define _PACKAGE_CPP_

#ifdef WIN32
#include <direct.h>
#endif

#include <stdlib.h>
#include <nge_io_file.h>
#include "package.h"

#ifdef __cplusplus
extern "C" {
#endif

static	int		fd		=	-1;		// PSP用变量
static	char*	name	=	NULL;	// PSP用变量

extern packInfo* fileInfo;
extern packItem* fileItem;
extern packItem* fileItem_temp;

void* safe_malloc(const long size)
{
	void* p = NULL;
	p = malloc (size);
	memset(p,0x00,size);
	return p;
}

void safe_free(void* p)
{
	if(p!=NULL) {
		free(p);
		p = NULL;
	}
}

void packageInit()
{
	packageFini();
}

void packageFini()
{
	while(fileItem!=NULL)
		del_fileItem();
	del_fileInfo();
}

void add_fileItem()
{
	if(fileItem_temp!=NULL && fileItem_temp->next==NULL) {
		fileItem_temp->next = (packItem*) safe_malloc (sizeof(packItem));
		fileItem_temp = fileItem_temp->next;
		fileItem_temp->next = NULL; }
	if(fileItem==NULL) {
		fileItem = (packItem*) safe_malloc (sizeof(packItem));
		fileItem->next = NULL;
		fileItem_temp = fileItem; }
	add_fileInfo();
}

void del_fileItem()
{
	packItem* temp1 = fileItem;
	packItem* temp2 = NULL;
	if(fileItem==NULL)
		return;
	if(temp1->next==NULL) {
		safe_free(temp1);
		fileItem = NULL;
		return; }
	else
		while(temp1->next->next!=NULL)
		temp1 = temp1->next;
	temp2 = temp1->next;
	temp1->next = NULL;
	safe_free(temp2);
}

void add_fileInfo()
{
	if(fileInfo==NULL)
		fileInfo = (packInfo*) safe_malloc (sizeof(packInfo));
}

void del_fileInfo()
{
	if(fileInfo!=NULL)
		safe_free(fileInfo);
}

void save_fileName(const char* filename)
{
	if(strlen(filename)>60) {
		printf("Save filaname: <%s> fail!\n",filename);
		return; }
	if(fileItem_temp==NULL) {
		printf("<fileItem_temp NULL>\n");
		return; }
	strcpy((char*)fileItem_temp->date.name,filename);
}

void save_fileSize(const long size)
{
	fileItem_temp->date.size = size;
}

void save_fileOffset(const int fileID)
{
	int i = 0;
	long offset = 0;
	packItem* temp = fileItem;
	if(fileID<=0)
		return;
	else if(fileID==1) {
		fileItem_temp->date.offset = 0;
		return; }
	for(i=0;i<fileID;i++) {
		if(i!=0) {
		offset += temp->date.size;
		temp = temp->next; }
	}
	fileItem_temp->date.offset = offset;
}

void save_fileItem(FILE* fp)
{
	packItem* temp = fileItem;
	if(fileItem==NULL) {
		printf("Save fileItem fail!\n");
		return; }
	if(fp==NULL)
		return;
	else {
		fseek(fp,0,SEEK_END);
		while(temp!=NULL) {
			fwrite(&temp->date,sizeof(packDate),1,fp);
			temp = temp->next; }
	}
}

void save_fileInfo(FILE* fp)
{
	if(fp==NULL || fileInfo==NULL)
		return;
	else {
		fseek(fp,0,SEEK_END);
		fileInfo->nums = get_fileID();
		fileInfo->VER = 0x01000000;
		fwrite(fileInfo,sizeof(packInfo),1,fp);
	}
}

void set_fileInfo(const char* text)
{
	if(fileInfo!=NULL)
		if( strlen(text) < (sizeof(fileInfo->info)/sizeof(char)) )
			strcpy(fileInfo->info,text);
}

void save_fileAddfile(const char* filename1,const char* filename2)
{
	long size = 0;
	unsigned char* buf = NULL;
	FILE* fpIn = NULL;	// filename2
	FILE* fpOut = NULL;	// filename1
	fpIn = fopen(filename2,"rb");
	if(fpIn==NULL)
		return;
	fpOut = fopen(filename1,"ab");
	if(fpOut==NULL)
		fpOut = fopen(filename1,"wb");
	fseek(fpIn,0,SEEK_END);
	size = ftell(fpIn);
	buf = (unsigned char*) safe_malloc (size);
	rewind(fpIn);
	fread(buf,size,1,fpIn);
	fwrite(buf,size,1,fpOut);
	safe_free(buf);
	fclose(fpIn);
	fclose(fpOut);
}

void save_itemTofile(char* filename,char* filetype)
{
#ifndef __HIDE_PSP_FUNC_
	char path[256];
	FILE* fp = NULL;
	FILE* out = NULL;
	packItem* temp = NULL;
	unsigned char* type = NULL;
	unsigned char* buf = NULL;
	fp = fopen(filename,"rb");
	if(fp==NULL) return;
	if(packInit(filename)==-1) {
		printf("------------------------------------------------------------\n");
		printf("解包失败,这不是一个符合规格的打包文件...\n\n");
		return;
	}
	temp = fileItem;
	printf("------------------------------------------------------------\n");
	while(temp!=NULL)
	{
		if( !strcmp(filetype,"*") )
			type=temp->date.name;
		else
			type = (unsigned char*)filetype;
		if( strstr((char*)type,"jpg") || strstr((char*)type,"JPG") || \
			strstr((char*)type,"bmp") || strstr((char*)type,"BMP") || \
			strstr((char*)type,"gif") || strstr((char*)type,"GIF") || \
			strstr((char*)type,"png") || strstr((char*)type,"PNG") || \
			strstr((char*)type,"tag") || strstr((char*)type,"TAG") ) {
			if( strstr((char*)temp->date.name,(char*)type) ) {
				strcpy(path,"images");
#ifdef WIN32
				mkdir(path);
#endif
				buf = (unsigned char*) safe_malloc (temp->date.size);
				fseek(fp,temp->date.offset,SEEK_SET);
				fread(buf,temp->date.size,1,fp);
				strcat(path,"/");
				strcat(path,(char*)temp->date.name);
				out = fopen(path,"wb");
				fwrite(buf,temp->date.size,1,out);
				safe_free(buf);
				fclose(out);
				printf("解包文件 <%s> 成功...\n",temp->date.name);
			}
			if(type != temp->date.name)
				if(!strcmp((char*)temp->date.name,(char*)type)) break;
		}
		if( strstr((char*)type,"mp3") || strstr((char*)type,"MP3") || \
			strstr((char*)type,"wav") || strstr((char*)type,"WAV") || \
			strstr((char*)type,"ogg") || strstr((char*)type,"OGG") ) {
			if( strstr((char*)temp->date.name,(char*)type) ) {
				strcpy(path,"sounds");
#ifdef WIN32
				mkdir(path);
#endif
				buf = (unsigned char*) safe_malloc (temp->date.size);
				fseek(fp,temp->date.offset,SEEK_SET);
				fread(buf,temp->date.size,1,fp);
				strcat(path,"/");
				strcat(path,(char*)temp->date.name);
				out = fopen(path,"wb");
				fwrite(buf,temp->date.size,1,out);
				safe_free(buf);
				fclose(out);
				printf("解包文件 <%s> 成功...\n",temp->date.name);
			}
			if(type != temp->date.name)
				if(!strcmp((char*)temp->date.name,(char*)type)) break;
		}
		if( strstr((char*)type,"pmp") || strstr((char*)type,"PMP") ) {
			if( strstr((char*)temp->date.name,(char*)type) ) {
				strcpy(path,"videos");
#ifdef WIN32
				mkdir(path);
#endif
				buf = (unsigned char*) safe_malloc (temp->date.size);
				fseek(fp,temp->date.offset,SEEK_SET);
				fread(buf,temp->date.size,1,fp);
				strcat(path,"/");
				strcat(path,(char*)temp->date.name);
				out = fopen(path,"wb");
				fwrite(buf,temp->date.size,1,out);
				safe_free(buf);
				fclose(out);
				printf("解包文件 <%s> 成功...\n",temp->date.name);
			}
			if(type != temp->date.name)
				if(!strcmp((char*)temp->date.name,(char*)type)) break;
		}
		temp = temp->next;
	}
	printf("------------------------------------------------------------\n");
	printf("解包完毕!\n\n");
	packFini();
#endif
}

int get_fileID()
{
	int num = 0;
	packItem* temp = fileItem;
	if(fileItem==NULL)
		return num;
	while(1) {
		if(temp!=NULL) {
			temp = temp->next;
			num += 1; }
		else
			break;
	}
	return (num);
}

long get_fileSize(const char* filename)
{
	long size;
	FILE* fp;
	fp = fopen(filename,"rb");
	if(fp==NULL) return -1;
	fseek(fp,0,SEEK_END);
	size =  ftell(fp);
	fclose(fp);
	return size;
}

void printf_allDate()
{
#ifdef WIN32
	packItem* temp = fileItem;
	if(fileItem==NULL || fileInfo==NULL) {
		printf("------------------------------------------------------------\n");
		printf("文件名无效或文件有破损,无法正常打开...\n\n");
		return;
	}
	if(get_fileID()<=0)
		return;
	for(int i=0;i<get_fileID();i++) {
		printf("------------------------------------------------------------\n");
		printf("NO.%d 名字:     <%s>\n",i+1,temp->date.name);
		printf("NO.%d 大小:     <%d>\n",i+1,temp->date.size);
		printf("NO.%d 位移量:   <%d>\n",i+1,temp->date.offset);
		temp = temp->next; }
	printf("文件数量: %-4d\n",get_fileID());
	if(fileInfo!=NULL)
		printf("文件头信息: %s\n",fileInfo->info);
	printf("------------------------------------------------------------\n\n");
#endif
}

//  -------PSP用函数--------

#ifndef __HIDE_PSP_FUNC_

void packCleanRes()
{
	if(fd>0) {
		io_fclose(fd);
		fd = -1; }
	if(name!=NULL) {
		free(name);
		name = NULL; }
	packageInit();
}

int packInit(const char* filename)
{
	int i = 0;
	packItem* temp = NULL;
	packCleanRes();
	fd = io_fopen(filename,IO_RDONLY);
	if(fd <= 0)
		return -1;
	fileInfo = (packInfo*) safe_malloc (sizeof(packInfo));
	io_fseek(fd,(int)(-sizeof(packInfo)),IO_SEEK_END);
	io_fread(fileInfo,sizeof(packInfo),1,fd);
	if(fileInfo->VER!=0x01000000) {	// 如果不是pack压缩包则取消读取
		packCleanRes();
		return -1; }
	io_fseek(fd,(int)(-(sizeof(packInfo)+sizeof(packDate)*fileInfo->nums)),IO_SEEK_END);
	for(i=0;i<fileInfo->nums;i++)
	{
		if(fileItem==NULL) {
			fileItem = (packItem*) safe_malloc (sizeof(packItem));
			temp = fileItem;
			io_fread(&temp->date,sizeof(packDate),1,fd);
			temp->next = NULL; }
		if((i!=0) && (temp->next==NULL)) {
			temp->next = (packItem*) safe_malloc (sizeof(packItem));
			temp = temp->next;
			io_fread(&temp->date,sizeof(packDate),1,fd);
			temp->next = NULL; }
	}
	if(name==NULL) {
		name = (char*) safe_malloc (strlen(filename));
		strcpy(name,filename); }
	//rewind((FILE*)fd);
	io_fseek(fd,0,IO_SEEK_SET);
	//printf("first fd address: 0x%x\n",fd);
	return fd;
}

int packOpen(const char* filename)
{
	packItem* temp = NULL;
	temp = fileItem;
	if(fileItem==NULL) return -1;
	while( strcmp(filename,(char*)temp->date.name) ) {
		temp = temp->next;
		if(temp==NULL)
			return -1; }
	io_fseek(fd,temp->date.offset,IO_SEEK_SET);
	return fd;
}

int packOpenAsync(const char* filename)
{
	int fd = -1;
	packItem* temp = NULL;
	temp = fileItem;
	if(fileItem==NULL) return -1;
	while( strcmp(filename,(char*)temp->date.name) ) {
		temp = temp->next;
		if(temp==NULL)
			return -1; }
	fd = io_fopen(name,IO_RDONLY);
	io_fseek(fd,temp->date.offset,IO_SEEK_SET);
	return fd;
}

long packSize(const char* filename)
{
	packItem* temp = NULL;
	temp = fileItem;
	if(fileItem==NULL) return -1;
	while( strcmp(filename,(char*)temp->date.name) ) {
		temp = temp->next;
		if(temp==NULL)
			return -1; }
	return temp->date.size;
}

long packOffset(const char* filename)
{
	packItem* temp = NULL;
	temp = fileItem;
	if(fileItem==NULL) return -1;
	while( strcmp(filename,(char*)temp->date.name) ) {
		temp = temp->next;
		if(temp==NULL)
			return -1; }
	return temp->date.offset;
}

void packFini(void)
{
	packCleanRes();
}

#endif

#ifdef __cplusplus
}
#endif

#endif