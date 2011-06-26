/**
 * @file  LibCoolAudio.java
 * @author Kun Wang <a href="mailto:ifreedom.cn@gmail.com">ifreedom.cn@gmail.com</a>
 * @date 2011/06/23 05:51:42
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

package org.libnge;

import android.media.MediaPlayer;
import java.io.FileDescriptor;
import android.util.Log;

public class LibCoolAudio extends Object
{
	private static final String TAG = "LibCoolAudio";
	private MediaPlayer mplayer = null;
	private Object mplayer_lock= new Object();
	private boolean isPrepared = false;
	private boolean isEof = false;
	private boolean isPaused = false;
	private boolean isSeeking = false;
	private int times = 0;
	private int volume = 100;

	public int init() {
		try {
			if(mplayer == null) {
				mplayer = new MediaPlayer();
			}
			if(mplayer == null) return -1;
			mplayer.setOnErrorListener(new MediaPlayer.OnErrorListener() {
					public boolean onError(MediaPlayer mp, int what, int extra) {
						if(mplayer != null)
							synchronized(mplayer_lock) {
								mplayer.release(); 	mplayer = null;
							}
						Log.e(TAG, "mplayer playback aborted with errors: " + what + ", " + extra);
						mplayer_lock.notify();
						return false;
					}
				});
			mplayer.setOnCompletionListener(new MediaPlayer.OnCompletionListener() {
					public void onCompletion(MediaPlayer mp) {
						synchronized(mplayer_lock) {
							if (times > 0) {
								times--;
								mplayer.start();
							}
							else {
								Log.i(TAG, "mplayer playback completed");
								isEof = true;
							}
						}
						mplayer_lock.notify();
					}
				});
			mplayer.setOnPreparedListener(new MediaPlayer.OnPreparedListener() {
					public void onPrepared(MediaPlayer mp) {
						isPrepared = true;
						mplayer_lock.notify();
					}
				});
			mplayer.setOnSeekCompleteListener(new MediaPlayer.OnSeekCompleteListener() {
					public void onSeekComplete(MediaPlayer mp) {
						isSeeking = false;
						mplayer_lock.notify();
					}
				});
		} catch (Exception e) {
			Log.e(TAG, "Exception in init(): " + e.toString());
			return -1;
		}
		return 0;
	}

	@Override
	public void finalize() {
		if(mplayer != null) {
			synchronized(mplayer_lock) {
				if(mplayer.isPlaying()) mplayer.stop();
				mplayer.release();
				mplayer = null;
			}
		}
	}

	public void load(String filename) {
		try {
			mplayer.setDataSource(filename);

			mplayer.prepare();
			while (!isPrepared)
				mplayer_lock.wait();

			mplayer.setVolume(100, 100);
		} catch (Exception e) {
			Log.e(TAG, "Exception in init(): " + e.toString());
		}
	}

	public void loadBuf(FileDescriptor fd, long offset, long length) {
		try {
			mplayer.setDataSource(fd, offset, length);

			mplayer.prepare();
			while (!isPrepared)
				mplayer_lock.wait();

			mplayer.setVolume(100, 100);
		} catch (Exception e) {
			Log.e(TAG, "Exception in init(): " + e.toString());
		}
	}

	public void play(int times_) {
		times = times;
		isPaused = false;
		mplayer.start();
	}

	public void pause() {
		mplayer.pause();
		isPaused = true;
	}

	public void resume() {
		mplayer.start();
		isPaused = false;
	}

	public void stop() {
		mplayer.stop();
		times = 0;
	}

	public int volume(int volume_) {
		int oldvolume = volume;
		mplayer.setVolume(volume_,volume_);
		volume = volume_;
		return oldvolume;
	}

	public void rewind() {
		isSeeking = true;
		mplayer.seekTo(0);
		try {
			while (isSeeking)
				mplayer_lock.wait();
		} catch (Exception e) {
			Log.e(TAG, "Exception in init(): " + e.toString());
		}
	}

	public static final int AUDIO_SEEK_SET = 0;
	public static final int AUDIO_SEEK_CUR = 1;
	public static final int AUDIO_SEEK_END = 2;

	public void seek(int ms, int flag) {
		isSeeking = true;
		if (flag == AUDIO_SEEK_SET) {
			mplayer.seekTo(ms);
		}
		else if (flag == AUDIO_SEEK_CUR) {
			int cur = mplayer.getCurrentPosition();
			mplayer.seekTo(cur + ms);
		}
		else if (flag == AUDIO_SEEK_END) {
			int end = mplayer.getDuration();
			mplayer.seekTo(end - ms);
		}
		try {
			while (isSeeking)
				mplayer_lock.wait();
		} catch (Exception e) {
			Log.e(TAG, "Exception in init(): " + e.toString());
		}
	}

	public boolean iseof() {
		return isEof;
	}

	public boolean ispaused() {
		return isPaused;
	}
}
