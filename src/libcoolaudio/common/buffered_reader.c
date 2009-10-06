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

#include <pspiofilemgr.h>
#include <stdlib.h>
#include <string.h>
#include "mem64.h"

#define BUFFERED_READER_BUFFER_SIZE 262144//131072//65536 //262144

typedef struct {
	SceUID handle;
	int32_t length;
	int32_t buffer_size;
	int32_t seek_mode;
	uint8_t* buffer_0;//[BUFFERED_READER_BUFFER_SIZE];
	uint8_t* buffer_1;//[BUFFERED_READER_BUFFER_SIZE];
	uint8_t* buffer_2;//[BUFFERED_READER_BUFFER_SIZE];
	uint8_t* first_buffer;
	uint8_t* second_buffer;
	uint8_t* third_buffer;
	int32_t position_0;
	int32_t position_1;
	int32_t position_2;
	int32_t position_3;
	int32_t current_position;
	
} buffered_reader_t;

void buffered_reader_close(buffered_reader_t* reader) {
	if (reader) {
		if ( !( reader->handle < 0 ) )
			sceIoClose(reader->handle); 
		if ( reader->buffer_0 != 0 )
			free_64(reader->buffer_0);
		if ( reader->buffer_1 != 0 )
			free_64(reader->buffer_1);
		if ( reader->buffer_2 != 0 )
			free_64(reader->buffer_2);
		free(reader);
	}
}

buffered_reader_t* buffered_reader_open(const char* path, int32_t buffer_size, int32_t seek_mode) {
	buffered_reader_t *reader = malloc(sizeof(buffered_reader_t));
	if ( reader == 0 )
		return 0;
	memset(reader, 0, sizeof(buffered_reader_t));
	reader->handle = -1;
	reader->buffer_size = buffer_size;
	reader->seek_mode = seek_mode;
	
	reader->buffer_0 = malloc_64(reader->buffer_size);
	if ( reader->buffer_0 == 0 ) {
		buffered_reader_close(reader);
		return 0;
	}
	
	reader->buffer_1 = malloc_64(reader->buffer_size);
	if ( reader->buffer_1 == 0 ) {
		buffered_reader_close(reader);
		return 0;
	}
	
	reader->buffer_2 = malloc_64(reader->buffer_size);
	if ( reader->buffer_2 == 0 ) {
		buffered_reader_close(reader);
		return 0;
	}
	
	reader->handle = sceIoOpen(path, PSP_O_RDONLY, 0777);
	
	if (reader->handle < 0) {
		buffered_reader_close(reader);
		return 0;
	}
	if (sceIoChangeAsyncPriority(reader->handle, 0x10) < 0) {
		buffered_reader_close(reader);
		return 0;
	}
	
	reader->length = sceIoLseek32(reader->handle, 0, PSP_SEEK_END);
	
	reader->first_buffer = reader->buffer_0;
	reader->second_buffer = reader->buffer_1;
	reader->third_buffer = reader->buffer_2;
	
	long long result;
	sceIoLseek32(reader->handle, reader->position_0, PSP_SEEK_SET);
	sceIoReadAsync(reader->handle, reader->first_buffer, reader->position_1 - reader->position_0);
	sceIoWaitAsync(reader->handle, &result);
	sceIoLseek32(reader->handle, reader->position_1, PSP_SEEK_SET);
	sceIoReadAsync(reader->handle, reader->second_buffer, reader->position_2 - reader->position_1);
	sceIoWaitAsync(reader->handle, &result);
	sceIoLseek32(reader->handle, reader->position_2, PSP_SEEK_SET);
	sceIoReadAsync(reader->handle, reader->third_buffer, reader->position_3 - reader->position_2);
	return reader;
}

int32_t buffered_reader_length(buffered_reader_t* reader) {
	return reader->length;
}

int32_t buffered_reader_seek(buffered_reader_t* reader, const int32_t position) {
	int32_t seek = 0;
	if ( position >= reader->position_0 && position < reader->position_2 ) {
		reader->current_position = position;
		return position;
	}
	else {
		long long result;
		sceIoWaitAsync(reader->handle, &result);
		if ( position >= reader->position_2 && position < reader->position_3 ) {
			uint8_t* temp = reader->first_buffer;
			reader->first_buffer = reader->second_buffer;
			reader->second_buffer = reader->third_buffer;
			reader->third_buffer = temp;
			reader->position_0 = reader->position_1;
			reader->position_1 = reader->position_2;
			reader->position_2 = reader->position_3;
			reader->current_position = position;
			reader->position_3 = reader->position_2 + reader->buffer_size;
			if ( reader->position_3 >= reader->length )
				reader->position_3 = reader->length;
			sceIoLseek32(reader->handle, reader->position_2, PSP_SEEK_SET);
			sceIoReadAsync(reader->handle, reader->third_buffer, reader->position_3 - reader->position_2);
			return position;
		}
		else {
			if ( position < reader->position_0 )
				seek = -1;
			else
				seek = 1;
			//if (reader->seek_mode == 0 ) {
			{
				int32_t old_position_0 = reader->position_0;
				int32_t old_position_1 = reader->position_1;
				int32_t old_position_2 = reader->position_2;
				int32_t old_position_3 = reader->position_3;
				
				reader->position_0 = position;
				reader->position_1 = reader->position_0 + reader->buffer_size;
				if ( reader->position_1 >= reader->length )
					reader->position_1 = reader->length;
				reader->position_2 = reader->position_1 + reader->buffer_size;
				if ( reader->position_2 >= reader->length )
					reader->position_2 = reader->length;
				reader->position_3 = reader->position_2 + reader->buffer_size;
				if ( reader->position_3 >= reader->length )
					reader->position_3 = reader->length;
					
				reader->current_position = reader->position_0;
				
				if ( seek > 0 || (reader->position_3 <= old_position_0) ) {
					sceIoLseek32(reader->handle, reader->position_0, PSP_SEEK_SET);
					sceIoRead(reader->handle, reader->first_buffer, reader->position_1 - reader->position_0);
					sceIoLseek32(reader->handle, reader->position_1, PSP_SEEK_SET);
					sceIoRead(reader->handle, reader->second_buffer, reader->position_2 - reader->position_1);
					sceIoLseek32(reader->handle, reader->position_2, PSP_SEEK_SET);
					sceIoReadAsync(reader->handle, reader->third_buffer, reader->position_3 - reader->position_2);
				}
				else {
					int32_t copy;
					uint8_t* dest;
					uint8_t* src;
					if ( old_position_0 >= reader->position_2 ) {
						copy = reader->position_3-old_position_0;
						dest = reader->third_buffer+(old_position_0-reader->position_2);
						src = reader->first_buffer;
						memcpy(dest, src, copy);
						
						sceIoLseek32(reader->handle, reader->position_0, PSP_SEEK_SET);
						sceIoRead(reader->handle, reader->first_buffer, reader->position_1 - reader->position_0);
						sceIoLseek32(reader->handle, reader->position_1, PSP_SEEK_SET);
						sceIoRead(reader->handle, reader->second_buffer, reader->position_2 - reader->position_1);
						sceIoLseek32(reader->handle, reader->position_2, PSP_SEEK_SET);
						sceIoReadAsync(reader->handle, reader->third_buffer, old_position_0 - reader->position_2);
					}
					else if ( old_position_0 >= reader->position_1 ) {
						
						copy = reader->position_3-old_position_1;
						dest = reader->third_buffer+(old_position_1-reader->position_2);
						src = reader->second_buffer;
						memcpy(dest, src, copy);
						
						copy = old_position_1-reader->position_2;
						dest = reader->third_buffer;
						src = reader->first_buffer+(reader->position_2-old_position_0);
						memcpy(dest, src, copy);
						
						copy = reader->position_2-old_position_0;
						dest = reader->second_buffer+(old_position_0-reader->position_1);
						src = reader->first_buffer;
						memcpy(dest, src, copy);
						
						sceIoLseek32(reader->handle, reader->position_0, PSP_SEEK_SET);
						sceIoRead(reader->handle, reader->first_buffer, reader->position_1 - reader->position_0);
						sceIoLseek32(reader->handle, reader->position_1, PSP_SEEK_SET);
						sceIoRead(reader->handle, reader->second_buffer, old_position_0 - reader->position_1);
						sceIoLseek32(reader->handle, reader->position_2, PSP_SEEK_SET);
						sceIoReadAsync(reader->handle, reader->third_buffer, 0);
					}
					else {
						copy = reader->position_3-old_position_2;
						dest = reader->third_buffer+(old_position_2-reader->position_2);
						src = reader->third_buffer;
						memmove(dest, src, copy);
						
						copy = old_position_2-reader->position_2;
						dest = reader->third_buffer;
						src = reader->second_buffer+(reader->position_2-old_position_1);
						memcpy(dest, src, copy);
						
						copy = reader->position_2-old_position_1;
						dest = reader->second_buffer+(old_position_1-reader->position_1);
						src = reader->second_buffer;
						memmove(dest, src, copy);
						
						copy = old_position_1-reader->position_1;
						dest = reader->second_buffer;
						src = reader->first_buffer+(reader->position_1-old_position_0);
						memcpy(dest, src, copy);
						
						copy = reader->position_1-old_position_0;
						dest = reader->first_buffer+(old_position_0-reader->position_0);
						src = reader->first_buffer;
						memmove(dest, src, copy);
						
						
						sceIoLseek32(reader->handle, reader->position_0, PSP_SEEK_SET);
						sceIoRead(reader->handle, reader->first_buffer, old_position_0 - reader->position_0);
						sceIoLseek32(reader->handle, reader->position_1, PSP_SEEK_SET);
						sceIoRead(reader->handle, reader->second_buffer, 0);
						sceIoLseek32(reader->handle, reader->position_2, PSP_SEEK_SET);
						sceIoReadAsync(reader->handle, reader->third_buffer, 0);
					}
				}
				return position;
			}
//			else {
//				int32_t old_position_0 = reader->position_0;
//				int32_t old_position_1 = reader->position_1;
//				int32_t old_position_2 = reader->position_2;
//				int32_t old_position_3 = reader->position_3;
//				
//				reader->position_1 = position;
//				reader->position_0 = reader->position_1 - reader->buffer_size;
//				if ( reader->position_0 < 0 )
//					reader->position_0 = 0;
//				reader->position_2 = reader->position_1 + reader->buffer_size;
//				if ( reader->position_2 >= reader->length )
//					reader->position_2 = reader->length;
//				reader->position_3 = reader->position_2 + reader->buffer_size;
//				if ( reader->position_3 >= reader->length )
//					reader->position_3 = reader->length;
//					
//				reader->current_position = reader->position_1;
//				
//				sceIoLseek32(reader->handle, reader->position_0, PSP_SEEK_SET);
//				sceIoRead(reader->handle, reader->first_buffer, reader->position_1 - reader->position_0);
//				sceIoLseek32(reader->handle, reader->position_1, PSP_SEEK_SET);
//				sceIoRead(reader->handle, reader->second_buffer, reader->position_2 - reader->position_1);
//				sceIoLseek32(reader->handle, reader->position_2, PSP_SEEK_SET);
//				sceIoReadAsync(reader->handle, reader->third_buffer, reader->position_3 - reader->position_2);
//				return position;
//			}
			
		}
	}
	return 0;
}

uint32_t buffered_reader_read(buffered_reader_t* reader, void* buffer, uint32_t size) {
	if ( reader->current_position == reader->length )
		return 0;
	if ( size <= reader->position_2 - reader->current_position ) {
		if ( reader->current_position < reader->position_1 ) {
			if ( size <= reader->position_1 - reader->current_position ) {
				memcpy( buffer, 
					reader->first_buffer + (reader->current_position - reader->position_0),
					size );
			}
			else {
				memcpy( buffer,
					reader->first_buffer + (reader->current_position - reader->position_0),
					reader->position_1 - reader->current_position);
				memcpy( buffer + (reader->position_1 - reader->current_position),
					reader->second_buffer,
					size - (reader->position_1 - reader->current_position) );
			}
		} 
		else {
			memcpy( buffer, 
				reader->second_buffer + (reader->current_position - reader->position_1),
				size );
		}
		reader->current_position += size;
		if ( reader->current_position == reader->position_2 )
			buffered_reader_seek(reader, reader->position_2);
		return size; 
	}
	else {
		uint32_t data_size;
		data_size = reader->position_2 - reader->current_position;
		buffered_reader_read(reader, buffer, data_size);
		data_size += buffered_reader_read(reader, buffer + data_size, size - data_size);
		return data_size;
	}
}

int32_t buffered_reader_position(buffered_reader_t* reader) {
	return reader->current_position;
}
