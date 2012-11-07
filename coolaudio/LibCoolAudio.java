
/**
 * @file  LibCoolAudio.java
 * @author Kun Wang <a href="mailto:ifreedom.cn@gmail.com">ifreedom.cn@gmail.com</a>
 * @date 2011/06/23 05:51:42
 *
 *  Copyright  2011  Kun Wang <ifreedom.cn@gmail.com>
 *
 */

package org.libnge.nge2;

import java.io.FileDescriptor;
import java.io.FileInputStream;

import android.media.MediaPlayer;
import android.util.Log;

public class LibCoolAudio extends Object
{
	private static final String TAG = "LibCoolAudio";
	private MediaPlayer mplayer = null;
	private Object mplayer_lock = new Object();
	private boolean isEof = false;
	private boolean isPaused = false;
	private boolean isSeeking = false;
	private int times = 0;
	private int volume = 100;
	private boolean hasError = false;
	private boolean loop = false;

	public int init() {
		Log.i(TAG, "libcoolaudio inited\n");
		try {
			if(mplayer == null) {
				mplayer = new MediaPlayer();
			}
			if(mplayer == null) return -1;
			mplayer.reset();
			mplayer.setOnErrorListener(new MediaPlayer.OnErrorListener() {
					public boolean onError(MediaPlayer mp, int what, int extra) {
						synchronized(mplayer_lock) {
							if(mplayer != null) {
								mplayer.reset();
								hasError = true;
							}
						}
						Log.e(TAG, "mplayer playback aborted with errors: " + what + ", " + extra);
						return false;
					}
				});
			mplayer.setOnCompletionListener(new MediaPlayer.OnCompletionListener() {
					public void onCompletion(MediaPlayer mp) {
						synchronized(mplayer_lock) {
							if (times > 1) {
								times--;
								mplayer.start();
							}
							else {
								if (loop)
									mplayer.start();
								else {
									Log.i(TAG, "mplayer playback completed");
									isEof = true;
								}
							}
						}
					}
				});
			mplayer.setOnSeekCompleteListener(new MediaPlayer.OnSeekCompleteListener() {
					public void onSeekComplete(MediaPlayer mp) {
						synchronized(mplayer_lock) {
							isSeeking = false;
							mplayer_lock.notify();
						}
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
		synchronized(mplayer_lock) {
			if(mplayer != null) {
				if(mplayer.isPlaying()) mplayer.stop();
				mplayer.release();
				mplayer = null;
			}
		}
	}

	public void load(String filename) {
		try {
			hasError = false;
			FileInputStream is = new FileInputStream(filename);
			if(is.available() > 0) {
				mplayer.reset();
				mplayer.setDataSource(is.getFD());
				mplayer.prepare();

				mplayer.setVolume(100, 100);
				Log.i(TAG, "ok!" + filename);
			}
			else
				Log.i(TAG, "err!" + filename);
		} catch (Exception e) {
			Log.e(TAG, "Exception in load(): " + e.toString());
		}
	}

	public void loadBuf(FileDescriptor fd, long offset, long length) {
		try {
			hasError = false;

			mplayer.reset();
			mplayer.setDataSource(fd, offset, length);
			mplayer.prepare();

			mplayer.setVolume(100, 100);
		} catch (Exception e) {
			Log.e(TAG, "Exception in loadbuf(): " + e.toString());
		}
	}

	public void play(int times_) {
		if (!hasError) {
			if (times_ == 0)
				loop = true;
			else
				loop = false;

			times = times_;
			isPaused = false;
			mplayer.start();
		}
	}

	public void pause() {
		if (!hasError) {
			mplayer.pause();
			isPaused = true;
		}
	}

	public void resume() {
		if (!hasError) {
			if (isPaused) {
				mplayer.start();
				isPaused = false;
			}
		}
	}

	public void stop() {
		if (!hasError) {
			times = 0;
			mplayer.stop();
			Log.i(TAG, "stop! ");
		}
	}

	public int volume(int volume_) {
		int oldvolume = volume;
		if (!hasError) {
			float v = volume_/100.0f;
			mplayer.setVolume(v,v);
			volume = volume_;
		}
		return oldvolume;
	}

	public void rewind() {
		if (!hasError) {
			try {
				isSeeking = true;
				mplayer.seekTo(0);
				synchronized(mplayer_lock) {
					while (isSeeking)
						mplayer_lock.wait();
				}
			} catch (Exception e) {
				Log.e(TAG, "Exception in rewind(): " + e.toString());
			}
		}
	}

	public static final int AUDIO_SEEK_SET = 0;
	public static final int AUDIO_SEEK_CUR = 1;
	public static final int AUDIO_SEEK_END = 2;

	public void seek(int ms, int flag) {
		if (!hasError) {
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
				synchronized(mplayer_lock) {
					while (isSeeking)
						mplayer_lock.wait();
				}
			} catch (Exception e) {
				Log.e(TAG, "Exception in seek(): " + e.toString());
			}
		}
	}

	public boolean iseof() {
		return isEof;
	}

	public boolean ispaused() {
		return isPaused;
	}
}
