/**
 * @file  RawAudio.java
 * @author Kun Wang <a href="mailto:ifreedom.cn@gmail.com">ifreedom.cn@gmail.com</a>
 * @date 2011/10/30 04:47:29
 *
 *  Copyright  2011  Kun Wang <ifreedom.cn@gmail.com>
 *
 */

package org.libnge.nge2;

import android.media.AudioTrack;
import android.media.AudioManager;

public class RawAudio
{
	public class PlaybackListener implements AudioTrack.OnPlaybackPositionUpdateListener
	{
		int cbPtr, cookiePtr;

		public PlaybackListener(int cbPtr, int cookiePtr) {
			this.cbPtr = cbPtr;
			this.cookiePtr = cookiePtr;
		}

		native void nativeNotify(int cbPtr, int cookiePtr);

		public void onMarkerReached(AudioTrack track) {
			synchronized(PlaybackListener.class) {
				nativeNotify(cbPtr, cookiePtr);
			}
		}

		public void	onPeriodicNotification(AudioTrack track) {
		}
	}

	AudioTrack mTrack;
	float volume = 0f;
	int marker = -1;

	static public int getMinBufferSize(int rate, int channel, int format) {
		return AudioTrack.getMinBufferSize(rate, channel, format);
	}

	public final void init(int rate, int channel, int format, int size, int mode) {
		if (mTrack != null)
			mTrack.release();

		mTrack = new AudioTrack(AudioManager.STREAM_MUSIC, rate, channel, format, size, mode);
		setVolume(1.0f);
		marker = -1;
	}
	public final void release() {
		if (mTrack != null) {
			mTrack.release();
			mTrack = null;
		}
	}

	public final void setListener(int cbPtr, int cookiePtr) {
		mTrack.setPlaybackPositionUpdateListener(new PlaybackListener(cbPtr, cookiePtr));
	}
	public final void clearListener() {
		mTrack.setPlaybackPositionUpdateListener(null);
	}

	public final void play() {
		mTrack.play();
	}
	public final void stop() {
		mTrack.stop();
	}
	public final void pause() {
		mTrack.pause();
	}

	public final int getPlayState() {
		return mTrack.getPlayState();
	}

	public final int reloadStaticData() {
		return mTrack.reloadStaticData();
	}

	public final int setLoopPoints(int startInFrames, int endInFrames, int loopCount) {
		return mTrack.setLoopPoints(startInFrames, endInFrames, loopCount);
	}

	public final int getPosition() {
		return mTrack.getPlaybackHeadPosition();
	}
	public final int setPosition(int frames) {
		return mTrack.setPlaybackHeadPosition(frames);
	}

	public final int write(byte[] buffer, int offsetInBytes, int sizeInBytes) {
		return mTrack.write(buffer, offsetInBytes, sizeInBytes);
	}

	public final int setVolume(float volume) {
		this.volume = volume;
		return mTrack.setStereoVolume(volume, volume);
	}
	public final float getVolume() {
		return volume;
	}

	public final int setMarker(int markerInFrames) {
		marker = markerInFrames;
		return mTrack.setNotificationMarkerPosition(markerInFrames);
	}
	public final int getMarker() {
		return marker;
	}
}
