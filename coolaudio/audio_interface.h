#ifndef AUDIO_INTERFACE_H_
#define AUDIO_INTERFACE_H_


// lib = libLSAudio.a

#define AUDIO_SEEK_SET	0
#define AUDIO_SEEK_CUR	1
#define	AUDIO_SEEK_END	2

/*
 * 本文件定义了audio的公共接口
 */
struct audio_play;
/**
 *从文件加载一个audio文件到内存
 */
typedef int (*fd_load) (struct audio_play* This,const char* filename);
/**
 *从内存加载一个audio文件到内存
 */
typedef int (*fd_load_buf) (struct audio_play* This,const char* buf,int size);
/**
 *从文件指针加载一个audio文件到内存
 */
typedef int (*fd_load_fp) (struct audio_play* This,int handle,char closed_by_me);

/**
 *播放一个声音
 *times=0为循环播放，time=n就播放几次，播放完成后
 *处于末尾(eof()==true)并等待（不释放内存，可以再次调用rewind到开头或者seek到某个开始处然后play播放，或者调用stop释放内存），
 *free_when_stop是播放完毕后是否释放内存（这个主要是用于播放音效使用）。
 */
typedef int (*fd_play) (struct audio_play* This,int times,int free_when_stop);
/**
 * 播放的简化版，等价于fd_play(This,1,1),用于播放音效;
 */
typedef int (*fd_playstop) (struct audio_play* This);

//暂停播放一个声音,用resume可以恢复。
typedef void (*fd_pause) (struct audio_play* This);

//停止声音并释放内存和资源。只有再次load才能重新加载声音。
typedef int (*fd_stop) (struct audio_play* This);

//恢复播放一个pause的声音。
typedef void (*fd_resume)(struct audio_play* This);


//设置音量
typedef int (*fd_volume) (struct audio_play* This,int volume);



//回到声音开头。同于seek(this,0,SEEK_BEGIN);
typedef void (*fd_rewind)(struct audio_play* This);

//ms，相对于flag的时间单位为ms，flag是Begin，end，例如：
//seek(this,0,Begin);就是回到开头，等价于rewind。
typedef void (*fd_seek) (struct audio_play* This,int ms,int flag);

//判定函数：是否到达声音尾部
typedef int (*fd_iseof) (struct audio_play* This);
//判定函数：是否暂停：
typedef int (*fd_ispaused)(struct audio_play* This);

//释放函数，释放这个播放器的公有资源
//包括player本身.以后此player不能再使用了,
//player变为一个无效指针，要再使用必须重新create
typedef int (*fd_destroy)(struct audio_play* This);
/*
 * 外部接口.
 */
typedef struct audio_play{
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
}audio_play_t,*audio_play_p;

//the interface
#ifdef __cplusplus
extern "C"{
#endif
//init and finish
void CoolAudioDefaultInit();
void CoolAudioDefaultFini();

//MP3
audio_play_p CreateMp3Player();
//wav
audio_play_p CreateWavPlayer();
//ogg
audio_play_p CreateOggPlayer();

#ifdef __cplusplus
}
#endif



#endif

