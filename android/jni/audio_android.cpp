/**
 * @file  audio_android.cpp
 * @author Kun Wang <ifreedom.cn@gmail.com>
 * @date 2011/06/12 14:56:05
 *
 *  Copyright  2011  Kun Wang <ifreedom.cn@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include "audio_interface.h"
#include "media/mediaplayer.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "unistd.h"

using namespace android;

typedef struct audio_media_player
{
	fd_load       load;
	fd_load_fp    load_fp;
	fd_load_buf   load_buf;
	fd_play       play;
	fd_playstop   playstop;
	fd_pause      pause;
	fd_stop       stop;
	fd_resume     resume;
	fd_volume     volume;
	fd_rewind     rewind;
	fd_seek       seek;
	fd_iseof      iseof;
	fd_ispaused   ispaused;
	fd_destroy    destroy;

// private:
	int fd, times;
	MediaPlayer* mp;
	int volumev;
	volatile bool isEof, isPause, isSeeking, isDestroy;
	unsigned char isWav;
} audio_media_player_t;

void CoolAudioDefaultInit() {}
void CoolAudioDefaultFini() {}

class CMediaPlayerListener : public MediaPlayerListener
{
public:
	CMediaPlayerListener(audio_media_player_t* pAudio) : m_pAudio(pAudio) {}
	virtual void notify(int msg, int ext1, int ext2) {
		switch (msg){
		case MEDIA_PREPARED:
			break;
		case MEDIA_PLAYBACK_COMPLETE:
			if (m_pAudio->times > 0) {
				m_pAudio->times--;
				m_pAudio->mp->start();
			}
			else
				m_pAudio->isEof = true;
			break;
		case MEDIA_SEEK_COMPLETE:
			m_pAudio->isSeeking = false;
			break;
		default:
			break;
		}
	}

private:
	audio_media_player_t* m_pAudio;
};

#define _METHOD(name) audio_##name
#define MAKE_METHOD(return_type,name,args) static return_type _METHOD(name) args

MAKE_METHOD(int, load, (audio_media_player_t* p, const char* filename))
{
	int fsize;
	int fd = open(filename, O_RDONLY);
	if (fd < 0) return -1;
	fsize = lseek(fd,0,SEEK_END);
	lseek(fd,0,SEEK_SET);

	p->mp->setDataSource(fd, 0, fsize);

	// release fd
	if (p->fd)
		close(p->fd);
	p->fd = fd;

	p->mp->prepare();
}

MAKE_METHOD(int, load_buf, (audio_media_player_t* p, const char* buf, int size))
{
	int fd[2];
	if (pipe(fd) < 0) {
		printf("Can not open pipe!\n");
		return -1;
	}

	write(fd[1], (void*)buf, size);

	p->mp->setDataSource(fd[0], 0, size);

	if (p->fd)
		close(p->fd);
	p->fd = fd[1];

	close(fd[0]);

	p->mp->prepare();
}

MAKE_METHOD(int, load_fp, (audio_media_player_t* p, int handle, char closed_by_me))
{
	printf("Not implementation!\n");
}

MAKE_METHOD(int, play, (audio_media_player_t* p, int times, int free_when_stop))
{
	p->times = times;
	p->isPause = false;
	p->mp->start();
}

MAKE_METHOD(int, playstop, (audio_media_player_t* p))
{
	p->times = 1;
	p->isPause = false;
	p->mp->start();
}

MAKE_METHOD(void, pause, (audio_media_player_t* p))
{
	p->mp->pause();
	p->isPause = true;
}

MAKE_METHOD(void, resume, (audio_media_player_t* p))
{
	p->mp->start();
	p->isPause = false;
}

MAKE_METHOD(int, stop, (audio_media_player_t* p))
{
	p->mp->stop();
	close(p->fd);
	p->fd = 0;
	p->times = 0;
}

MAKE_METHOD(int, volume, (audio_media_player_t* p, int volume))
{
	int oldvolume = p->volumev;
	p->mp->setVolume(volume, volume);
	p->volumev = volume;

	return oldvolume;
}

MAKE_METHOD(void, rewind, (audio_media_player_t* p))
{
	p->isSeeking = true;
	p->mp->seekTo(0);
	while (p->isSeeking);
}

MAKE_METHOD(void, seek, (audio_media_player_t* p, int ms, int flag))
{
	if (flag == AUDIO_SEEK_SET) {
		p->isSeeking = true;
		p->mp->seekTo(ms);
	}
	else if (flag == AUDIO_SEEK_CUR) {
		int cur = 0;
		p->mp->getCurrentPosition(&cur);
		p->isSeeking = true;
		p->mp->seekTo(cur + ms);
	}
	while (p->isSeeking);
}

MAKE_METHOD(int, iseof, (audio_media_player_t* p))
{
	return p->isEof;
}

MAKE_METHOD(int, ispaused, (audio_media_player_t* p))
{
	return p->isPause;
}

MAKE_METHOD(int, destroy, (audio_media_player_t* p))
{
	delete p->mp;
	if (p->isWav)
		delete p;
	else
		p->isDestroy = true;
}

MAKE_METHOD(void, init, (audio_media_player_t* p))
{
	CMediaPlayerListener* listener = new CMediaPlayerListener(p);
	p->mp = new MediaPlayer();
	p->mp->setListener(listener);

	p->volumev = 100;
	p->mp->setVolume(100, 100);

	p->fd = 0;
	p->times = 0;
	p->isSeeking = false;
	p->isEof = false;
	p->isPause = false;
	p->isDestroy = false;

#define LOAD_METHOD(name) p->name = (fp_##name) _METHOD(name)
	LOAD_METHOD(load);
	LOAD_METHOD(load_buf);
	LOAD_METHOD(load_fp);
	LOAD_METHOD(play);
	LOAD_METHOD(playstop);
	LOAD_METHOD(pause);
	LOAD_METHOD(stop);
	LOAD_METHOD(resume);
	LOAD_METHOD(volume);
	LOAD_METHOD(rewind);
	LOAD_METHOD(seek);
	LOAD_METHOD(iseof);
	LOAD_METHOD(ispaused);
	LOAD_METHOD(destroy);
#undef LOAD_METHOD
}

#undef MAKE_METHOD

audio_play_p CreateWavPlayer()
{
	audio_media_player_t* pAudio = new audio_media_player_t;

	pAudio->isWav = TRUE;

	_METHOD(init)(pAudio);

	return static_cast<audio_play_p>(pAudio);
}

audio_play_p CreateMp3Player()
{
	static audio_media_player_t* pAudio = NULL;

	if (!pAudio)
	{
		pAudio = new audio_media_player_t;
		pAudio->isDestroy = true;
		pAudio->isWav = FALSE;
	}

	if (pAudio->isDestroy)
		_METHOD(init)(pAudio);

	return static_cast<audio_play_p>(pAudio);
}

audio_play_p CreateOggPlayer()
{
	static audio_media_player_t* pAudio = NULL;

	if (!pAudio)
	{
		pAudio = new audio_media_player_t;
		pAudio->isDestroy = true;
		pAudio->isWav = FALSE;
	}

	if (pAudio->isDestroy)
		_METHOD(init)(pAudio);

	return static_cast<audio_play_p>(pAudio);
}
