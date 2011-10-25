
/*
 *	2009-04-09
 *	作者: 白枫
 *	说明: 打包小工具,部分代码参考自"sijiu"的作品"MpkPackage",编译不通过是由于NGE2引擎未配置正确,
 *	只需要把#define __HIDE_PSP_FUNC_的注释去掉即可让代码通过编译,不过会引起程序部分功能缺失
 */
#ifndef _PACKAGE_H_
#define _PACKAGE_H_

#ifdef WIN32
	#include <io.h>
#endif

#include <stdio.h>
#include <string.h>

//#define __HIDE_PSP_FUNC_	// 去掉注释则隐藏所有PSP用函数

typedef struct packDate	{	/* 文件索引表数据 */
	unsigned char name[64-8];		/* 文件名 */
	long offset;			/* 文件位移 */
	long size;				/* 文件大小 */
} packDate;

typedef struct packInfo	{	/* 文件头 */
	int VER;				/* 库版本 */
	int nums;				/* 文件数目 */
	char info[128-8];		/* 打包信息 */
} packInfo;

typedef struct packItem	{	/* 索引表链表 */
	packDate date;			/* 数据 */
	struct packItem* next;	/* 链表 */
}  packItem;

#ifdef __cplusplus
extern "C" {
#endif

	static packInfo* fileInfo = NULL;
	static packItem* fileItem = NULL;
	static packItem* fileItem_temp = NULL;

void packageInit();		// 初始化索引表和文件头信息为0
void packageFini();
void add_fileItem();	// 添加一个索引,如果索引头为空则建立一个
void del_fileItem();	// 从索引链的尾部删除一个索引
void add_fileInfo();	// 创建一个文件头数据,不管调用几次只能创造一个
void del_fileInfo();	// 删除文件头数据
void save_fileName(const char* filename);
void save_fileSize(const long size);
void save_fileOffset(const int fileID);	// 把信息保存到当前索引表
void save_fileItem(FILE* fp);	// 保存索引表数据到文件尾部
void save_fileInfo(FILE* fp);	// 保存文件头数据到文件尾部
void set_fileInfo(const char* text);
void save_fileAddfile(const char* filename1,const char* filename2);	// 文件2添加到文件1尾部
void save_itemTofile(char* filename,char* filetype);	// 从压缩包中解压文件
int get_fileID();		// 得到索引表个数
long get_fileSize(const char* filename);	// 得到磁盘上一个文件的大小
void printf_allDate();			// 调试信息
void* safe_malloc(const long size);
void safe_free(void* p);

#ifndef __HIDE_PSP_FUNC_
/*
 *	以下PSP用函数 -- 编译不通过时把PSP用函数全部注释掉就行
 *-------------------------------------------------------------------
 */

/*
 *	功能:初始化系统并加载资源到缓冲,必须操作
 *	const char* filename 需要加载的打包文件
 *	return 文件加载失败返回-1,成功返回文件指针fd
 */
int		packInit		(const char* filename);
/*
 *	功能:打开打包文件中名为filename的文件,不可进行文件关闭操作
 *			文件关闭操作由系统完成,只是单纯想得到某个文件建议用此函数
 *	{
 *		packInit("DATA.BIN");
 *		int fd = packOpen("image001.jpg");
 *		int size = packSize("image001.jpg");
 *		.....;
 *		packFini();
 *	}
 *	const char* filename 需要打开的文件名
 *	return 打开失败返回-1,打开成功返回文件指针fd
 */
int		packOpen		(const char* filename);
/*
 *	功能:异步打开打包文件中名为filename的文件,流读取建议用此函数,
 *			使用完毕需要自行进行文件关闭操作,例如:
 *	{
 *		packInit("DATA.BIN");
 *		int fd = packOpenAsync("bgmusic.mp3");
 *		int size = packSize("bgmusic.mp3");
 *		.....;
 *		io_fclose(fd);
 *		packFini();
 *	}
 *	const char* filenmae 需要打开的文件名
 *	return 打开失败返回-1,打开成功返回文件指针fd
 */
int		packOpenAsync	(const char* filename);
/*
 *	功能:得到打包文件中名为filename的文件大小
 *	const char* filename 需要打开的文件名
 *	return 打开失败返回-1,打开成功返回文件大小
 */
long	packSize		(const char* filename);
/*
 *	功能:得到打包文件中名为filename的文件偏移量
 *	const char* filename 需要打开的文件名
 *	return 打开失败返回-1,打开成功返回文件偏移量offset
 */
long	packOffset		(const char* filename);
/*
 *	功能:关闭系统并清空资源
 *	return 无
 */
void	packFini		(void);

#endif

#ifdef __cplusplus
}
#endif

#endif
