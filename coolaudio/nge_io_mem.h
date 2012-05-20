#ifndef NGE_IO_MEM_H_
#define NGE_IO_MEM_H_

#define IO_RDONLY 0
#define IO_WRONLY 1
#define IO_APPEND 2


#define IO_SEEK_CUR SEEK_CUR
#define IO_SEEK_END SEEK_END
#define IO_SEEK_SET SEEK_SET

#ifdef __cplusplus
extern "C"{
#endif
	/**
	*打开MEM,注意确保MEM有效
	*@param const char*,mem指针
	*@param int,size
	*@param int,打开标志(IO_RDONLY,IO_WRONLY,IO_APPEND的一种)
	*@return int,文件句柄,读写均用此文件句柄进行操作
	*/
	int io_mopen(const char* mem,int size,int flag);
	/**
	*读MEM文件,与fread类似
	*@param void*,保存读取数据的缓存
	*@param int,读取个数
	*@param int,读取大小
	*@param int,文件句柄
	*@return,实际读取数量
	*/
	int io_mread(void* buffer,int count,int size,int handle);
	/**
	*写MEM文件,与fwrite类似
	*@param void*,待写数据的缓存
	*@param int,待写块个数
	*@param int,待写块大小
	*@param int,文件句柄
	*@return,实际写入数量
	*/
	int io_mwrite(void* buffer,int count,int size,int handle);
	/**
	*文件指针偏移设置,与fseek类似
	*@param int,文件句柄
	*@param int offset,偏移量
	*@param int flag,seek标志,IO_SEEK_CUR,IO_SEEK_END,IO_SEEK_SET
	*@return int,返回0成功,-1失败
	*/
	int io_mseek(int handle,int offset,int flag);
	/**
	*获得当前文件偏移值
	*@param int handle,文件句柄
	*@return int,文件偏移值
	*/
	int io_mtell(int handle);
	/**
	*关闭MEM文件
	*@param int handle,文件句柄
	*@return int,-1失败
	*/
	int io_mclose(int handle);
	/**
	*MEM size大小
	*@param int handle,文件句柄
	*@return int,MEM size大小
	*/
	int io_msize(int handle);
	
#ifdef __cplusplus
}
#endif

#endif

