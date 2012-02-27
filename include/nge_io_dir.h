#ifndef NGE_DIR_IO_H
#define NGE_DIR_IO_H

#include "nge_common.h"

enum FileFlags{
	FIO_F_DIR,//dir
	FIO_F_FILE//file
};

#define MAX_NAME 256
typedef struct  {
	int    flag;// FileFlags
	char   name[MAX_NAME];// name
}dir_desc_t,*dir_desc_p;

typedef void (*file_travel_cb)(dir_desc_p dir_desc,void* data);

#ifdef __cplusplus
extern "C"{
#endif

/**
 * 目录遍历访问函数,只支持英文路径和文件名.
 * 注意在PSP上全小写的文件名会以全部大写显示.
 *@param const char* directory, 待访问目录名
 *@param file_travel_cb callback, 访问回调函数
 *@param void* user_data,用户参数
 *@return int ,返回值0,1
 */
	NGE_API int io_dir_travel(const char* directory, file_travel_cb callback,void* user_data);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif
