/**
 * @file  nge_io_file_internal.h
 * @author Kun Wang <ifreedom.cn@gmail.com>
 * @date 2012/03/18 04:40:28
 *
 *  Copyright  2012  Kun Wang <ifreedom.cn@gmail.com>
 *
 */

#ifndef _NGE_IO_FILE_INTERNAL_H
#define _NGE_IO_FILE_INTERNAL_H

typedef struct ngeVFOperation_s ngeVFOperation;

typedef struct
{
	void* ptr;
	ngeVFOperation* op;
} ngeVF;

struct ngeVFOperation_s
{
	/**
	   Close a file.

	   Return 1 if anything went well, 0 to throw an error.
	*/
	int (*Close)(ngeVF *f);

	/**
	   Read in a file. (optional)

	   Returns the number of bytes effectively read.
	*/
	int (*Read)(void *buf, size_t size, size_t n, ngeVF* f);

	/**
	   Write in a file (optional)

	   Returns the number of bytes effectively written.
	*/
	int (*Write)(const void *buf, size_t size, size_t n, ngeVF* f);

	/**
	   Moving in the file

	   Sets the current file position and returns the old one. The flag parameter uses the same values as stdio (IO_SEEK_SET, IO_SEEK_CUR, IO_SEEK_END).
	*/
	int (*Seek)(ngeVF *f, int offset, int flag);

	/**
	   Get current file position

	   Returns the current pointer position in the file.
	*/
	int (*Tell)(ngeVF *f);

	/**
	   End of file (optional)

	   Returns true (1) if it's the end of the file, false (0) else.
	*/
	int (*Eof)(ngeVF *f);

	/**
	   Size of file (optional)
	*/
	int (*Size)(ngeVF *f);
};

#ifdef __cplusplus
extern "C" {
#endif

	NGE_API int ngeVFRegister(ngeVF* f);

#ifdef __cplusplus
}
#endif

#endif /* _NGE_IO_FILE_INTERNAL_H */
