/**
 * @file  SoundpoolManager.java
 * @author Kun Wang <a href="mailto:ifreedom.cn@gmail.com">ifreedom.cn@gmail.com</a>
 * @date 2012/11/06 13:36:37
 *
 *  Copyright  2012  Kun Wang <ifreedom.cn@gmail.com>
 *
 */

package org.libnge.nge2;

import java.io.FileDescriptor;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.Queue;

import android.media.SoundPool;
import android.media.AudioManager;
import android.os.SystemClock;

import android.util.SparseArray;

public class SoundpoolManager
{
	@SuppressWarnings("unused")
	private static final String TAG = "SoundpoolManager";

	private static final int maxSoundPerPool = 4;

	private class PlayerData {
		private int id;
		public boolean isDestroy = false;
		public int loadStatus = 0;
		public int soundId = -1;
		public int streamId = -1;
		public boolean isPaused = false;
		public float volume = 1.0f;

		public PlayerData(int id) {
			this.id = id;
		}
		public int getId() { return id; }

		public void reset() {
			isDestroy = false;
			loadStatus = 0;
			soundId = -1;
			streamId = -1;
			isPaused = false;
			volume = 1.0f;
		}
	}

	private class MyPool implements SoundPool.OnLoadCompleteListener {
		private SparseArray<PlayerData> soundMap = new SparseArray<PlayerData>();
		private SoundPool pool = null;
		public int priority = 1;

		public SoundPool getSoundPool() { return pool; }
		public void initSoundPool() {
			pool = new SoundPool(maxSoundPerPool, AudioManager.STREAM_MUSIC, 100);
			pool.setOnLoadCompleteListener(this);
		}
		public void releaseSoundPool() {
			pool.release();
			priority = 1;
			pool = null;
		}

		public void add(int soundId, PlayerData data) {
			synchronized(soundMap) {
				soundMap.append(soundId, data);
			}
		}
		public void onLoadComplete(SoundPool soundPool, int soundId, int status) {
			synchronized(soundMap) {
				PlayerData data = soundMap.get(soundId);
				soundMap.remove(soundId);

				synchronized(data) {
					if (status == 0)
						data.loadStatus = 1;
					else
						data.loadStatus = -1;

					data.notify();
				}
			}
		}
	}

	private List<MyPool> poolList = new ArrayList<MyPool>();
	private List<PlayerData> playerList = new ArrayList<PlayerData>();
	private Queue<PlayerData> freeList = new LinkedList<PlayerData>();

	private int getPoolIdx(int playerId) {
		return playerId / maxSoundPerPool;
	}
	private MyPool getPoolByPlayerId(int playerId) {
		return poolList.get(getPoolIdx(playerId));
	}

	private void createNewPool() {
		int count = playerList.size();
		MyPool pool = new MyPool();
		pool.initSoundPool();
		poolList.add(pool);

		PlayerData data;
		for (int i=0; i<maxSoundPerPool; i++) {
			data = new PlayerData(count + i);
			playerList.add(data);
			freeList.offer(data);
		}
	}

	public int create() {
		if (freeList.size() == 0) {
			createNewPool();
		}

		PlayerData p = freeList.poll();
		if (p.isDestroy) {
			p.reset();

			MyPool pool = getPoolByPlayerId(p.getId());
			if (pool.getSoundPool() == null)
				pool.initSoundPool();
		}
		return p.getId();
	}
	public void delete(int playerId) {
		PlayerData data = playerList.get(playerId);
		freeList.offer(data);

		MyPool pool = getPoolByPlayerId(playerId);
		SoundPool p = pool.getSoundPool();
		if (data.streamId != -1) {
			p.stop(data.streamId);
		}
		p.unload(data.soundId);
		data.isDestroy = true;

		boolean canDestroyPool = true;
		int idx = playerId % maxSoundPerPool;
		idx = playerId - idx;
		for (int i=0; i<maxSoundPerPool; i++) {
			data = playerList.get(idx + i);
			if (!data.isDestroy)
				canDestroyPool = false;
		}

		if (canDestroyPool) {
			pool.releaseSoundPool();
		}
	}

	void load_wait(PlayerData data, MyPool pool) {
		pool.add(data.soundId, data);

		synchronized(data) {
			while (data.loadStatus == 0)
				try {
					data.wait();
				} catch (InterruptedException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
		}
	}

	public int loadFd(int playerId, FileDescriptor fd, int length) {
		PlayerData data = playerList.get(playerId);
		if (data.isDestroy) return -1;

		MyPool pool = getPoolByPlayerId(playerId);

		data.soundId = pool.getSoundPool().load(fd, 0, length, 10);

		load_wait(data, pool);

		return data.loadStatus;
	}

	public int load(int playerId, String path) {
		PlayerData data = playerList.get(playerId);
		if (data.isDestroy) return -1;

		MyPool pool = getPoolByPlayerId(playerId);

		data.soundId = pool.getSoundPool().load(path, 10);

		load_wait(data, pool);

		return data.loadStatus;
	}

	public void play(int playerId, int times) {
		PlayerData data = playerList.get(playerId);
		if (data.isDestroy) return;

		MyPool pool = getPoolByPlayerId(playerId);

		int abortCount = 20;
		do {
			data.streamId = pool.getSoundPool().play(data.soundId, data.volume, data.volume, pool.priority, times-1, 1f);
			if(data.streamId == 0) {
				SystemClock.sleep(10);
			}
		} while(data.streamId == 0 && abortCount-- > 0);

		pool.priority++;
	}

	public void pause(int playerId) {
		PlayerData data = playerList.get(playerId);
		if (data.isDestroy) return;

		MyPool pool = getPoolByPlayerId(playerId);

		pool.getSoundPool().pause(data.streamId);
		data.isPaused = true;
	}

	public void resume(int playerId) {
		PlayerData data = playerList.get(playerId);
		if (data.isDestroy) return;

		MyPool pool = getPoolByPlayerId(playerId);

		pool.getSoundPool().resume(data.streamId);
		data.isPaused = false;
	}

	public void stop(int playerId) {
		PlayerData data = playerList.get(playerId);
		if (data.isDestroy) return;

		MyPool pool = getPoolByPlayerId(playerId);

		pool.getSoundPool().stop(data.streamId);
	}

	public float setVolume(int playerId, float v) {
		PlayerData data = playerList.get(playerId);
		if (data.isDestroy) return 0f;

		MyPool pool = getPoolByPlayerId(playerId);

		pool.getSoundPool().setVolume(data.streamId, v, v);
		float lastV = data.volume;
		data.volume = v;

		return lastV;
	}

	public boolean isPaused(int playerId) {
		PlayerData data = playerList.get(playerId);
		return data.isPaused;
	}
}
