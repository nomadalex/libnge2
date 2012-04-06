/***************************************************************************
 *            nge_audio_linux.c
 *
 *  2011/05/16 13:08:52
 *  Copyright  2011  Kun Wang <ifreedom.cn@gmail.com>
 ****************************************************************************/
/*
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

#include <gst/gst.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

/*
// mov not support on x64
// for get argc and argv outside of main
char** get_main_argv(int* pargc)
{
	static char **preset_argv=NULL;
	static int argc = 0;

	if (!preset_argv)
	{
		extern char **environ;
		char ***argvp;
		void *p;
		asm ("mov %%esp, %0" : "=r" (p));
		argvp = p;
		while (*argvp != environ) {
			argvp++;
		}

		argvp--;
		preset_argv = *argvp;

		while (preset_argv[argc] != 0) {
			argc++;
		}
	}
	*pargc = argc;
	return preset_argv;
}
*/

void CoolAudioDefaultInit()
{
	/* int argc = 0; */
	char* argv[1] = { (char*)0 };

	/* argv = get_main_argv(&argc); */

	gst_init(0, (char***)&argv);
	/* gst_init(&argc, &argv); */
}

void CoolAudioDefaultFini()
{
}

typedef struct {
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
	GstElement *m_play, *m_src;
	guint8* m_buf;
	gint m_buf_length, m_times;
	gboolean m_isEOF;
	gboolean m_isDestroy;
} linux_audio_t;

static int audio_load(linux_audio_t* p,const char* filename)
{
	char name[256], tempbuf[256];

	getcwd(name, 256);
	sprintf(tempbuf, "file://%s/%s", name, filename);

	g_object_set(p->m_play, "uri", tempbuf, NULL);
	return 0;
}

static void start_feed (GstElement * src, guint size, linux_audio_t* p)
{
	GstBuffer *buffer;
	GstFlowReturn ret;

	buffer = gst_buffer_new ();
	GST_BUFFER_DATA (buffer) = p->m_buf;
	GST_BUFFER_SIZE (buffer) = p->m_buf_length;

	g_signal_emit_by_name (p->m_src, "push-buffer", buffer, &ret);
	gst_buffer_unref (buffer);

	g_signal_emit_by_name (p->m_src, "end-of-stream", &ret);
	gst_object_unref(p->m_src);
	p->m_src = NULL;
}

static void found_source (GObject * object, GObject * orig, GParamSpec * pspec, linux_audio_t* p)
{
	g_object_get (orig, pspec->name, &(p->m_src), NULL);

	g_object_set (p->m_src, "size", (gint64) p->m_buf_length, NULL);

	g_signal_connect (p->m_src, "need-data", G_CALLBACK (start_feed), (gpointer) p);
}

static int audio_load_buf(linux_audio_t* p,const char* buf,int size)
{
	p->m_buf_length = size;
	p->m_buf = (guint8*) buf;
	g_object_set(p->m_play, "uri", "appsrc://", NULL);
	g_signal_connect (p->m_play, "deep-notify::source",
					  G_CALLBACK(found_source), (gpointer) p);
	return 1;
}

static int audio_load_fp(linux_audio_t* p,int handle,char closed_by_me)
{
	printf("Sorry, %s not implement now.\n", __FUNCTION__);
	return 1;
}

static int audio_play(linux_audio_t* p,int times,int free_when_stop)
{
	p->m_isEOF = FALSE;
	p->m_times = times;
	gst_element_set_state (p->m_play, GST_STATE_PLAYING);
	return 1;
}

static int audio_playstop(linux_audio_t* p)
{
	audio_play(p, 1, 0);
	return 1;
}

static void audio_pause(linux_audio_t* p)
{
	gst_element_set_state (p->m_play, GST_STATE_PAUSED);
}

static int audio_stop(linux_audio_t* p)
{
	gst_element_set_state (p->m_play, GST_STATE_NULL);
	return 1;
}

static void audio_resume(linux_audio_t* p)
{
	gst_element_set_state (p->m_play, GST_STATE_PLAYING);
}

static int audio_volume(linux_audio_t* p,int volume)
{
	gdouble oldv = 0;
	g_object_get(p->m_play, "volume", &oldv, NULL);
	g_object_set(p->m_play, "volume", (double)(volume/100.0f), NULL);
	return (int)(oldv*100);
}

static void audio_rewind(linux_audio_t* p)
{
	gst_element_set_state (p->m_play, GST_STATE_NULL);
}

static void audio_seek(linux_audio_t* p,int ms,int flag)
{
	gst_element_seek_simple(p->m_play,  GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT, ms * GST_MSECOND);
}

static int audio_iseof(linux_audio_t* p)
{
	return p->m_isEOF;
}

static int audio_ispaused(linux_audio_t* p)
{

	GstState state = 0;
	gst_element_get_state(p->m_play, &state, NULL, 3 * GST_SECOND) ;

	if(state == GST_STATE_PAUSED)
		return TRUE;

	return FALSE;
}

static int audio_destroy(linux_audio_t* p)
{
	p->m_isDestroy = TRUE;
	gst_element_set_state (p->m_play, GST_STATE_NULL);
	gst_object_unref(p->m_play);
	return 0;
}

void finish_one_play(GstElement* play, linux_audio_t* p)
{
	p->m_times--;
	if( p->m_times == 0)
		p->m_isEOF = TRUE;
	else {
		gst_element_set_state (p->m_play, GST_STATE_PLAYING);
	}
}

void audio_constructor(linux_audio_t* player)
{
#define METHOD_(n) player->n = (fd_##n) audio_##n
	METHOD_(load);
	METHOD_(load_fp);
	METHOD_(load_buf);
	METHOD_(play);
	METHOD_(playstop);
	METHOD_(pause);
	METHOD_(stop);
	METHOD_(resume);
	METHOD_(volume);
	METHOD_(rewind);
	METHOD_(seek);
	METHOD_(iseof);
	METHOD_(ispaused);
	METHOD_(destroy);
#undef METHOD_

	player->m_isDestroy = FALSE;
	player->m_isEOF = FALSE;
	player->m_times = 0;
	player->m_play = gst_element_factory_make("playbin2", NULL);
	g_assert(player->m_play);

	g_signal_connect (player->m_play, "about-to-finish",
					  G_CALLBACK(finish_one_play), (gpointer) player);
}

audio_play_p CreateMp3Player()
{
	static linux_audio_t* player = NULL;

	if (!player) {
		player = (linux_audio_t*) malloc(sizeof(linux_audio_t));
		memset(player, 0, sizeof(linux_audio_t));
		player->m_isDestroy = TRUE;
	}

	if(player->m_isDestroy)
		audio_constructor(player);

	return (audio_play_p) player;
}

audio_play_p CreateWavPlayer()
{
	static linux_audio_t* player = NULL;

	if (!player) {
		player = (linux_audio_t*) malloc(sizeof(linux_audio_t));
		memset(player, 0, sizeof(linux_audio_t));
		player->m_isDestroy = TRUE;
	}

	if(player->m_isDestroy)
		audio_constructor(player);

	return (audio_play_p) player;
}

audio_play_p CreateOggPlayer()
{
	static linux_audio_t* player = NULL;

	if (!player) {
		player = (linux_audio_t*) malloc(sizeof(linux_audio_t));
		memset(player, 0, sizeof(linux_audio_t));
		player->m_isDestroy = TRUE;
	}

	if(player->m_isDestroy)
		audio_constructor(player);

	return (audio_play_p) player;
}
