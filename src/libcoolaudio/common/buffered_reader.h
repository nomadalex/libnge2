/* 
 *	Copyright (C) 2006 cooleyes
 *	eyes.cooleyes@gmail.com 
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */
 
#ifndef __COOLEYES_BUFFERED_READER_H__
#define __COOLEYES_BUFFERED_READER_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef void* buffered_reader_t;

buffered_reader_t* buffered_reader_open(const char* path, int32_t buffer_size, int32_t seek_mode);

int32_t buffered_reader_length(buffered_reader_t* reader);

int32_t buffered_reader_seek(buffered_reader_t* reader, const int32_t position);

int32_t buffered_reader_position(buffered_reader_t* reader);

uint32_t buffered_reader_read(buffered_reader_t* reader, void* buffer, uint32_t size);

void buffered_reader_close(buffered_reader_t* reader);

#ifdef __cplusplus
}
#endif

#endif
