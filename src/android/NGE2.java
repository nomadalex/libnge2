/*
 * Copyright (C) 2008-2011 TOPOC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package org.libnge.nge2;

import java.io.File;
import java.util.Date;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.app.Activity;
import android.content.Context;
import android.media.MediaPlayer;
import android.opengl.GLSurfaceView;
import android.opengl.GLSurfaceView.Renderer;
import android.os.Bundle;
import android.os.Environment;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.widget.MediaController;
import android.widget.RelativeLayout;
import android.widget.RelativeLayout.LayoutParams;
import android.widget.Toast;
import android.widget.VideoView;

public class NGE2 extends Activity {
	public NGE2View m_view;
	private NGE2Renderer m_renderer;
	public int g_height;
	public int g_width;

	private native void nativeSetPackname(String packname);

	private native void nativeCreate();

	private native void nativeInitialize();

	private native void nativeFinalize();

	public static final int APP_NORMAL = 0;
	public static final int APP_QUIT = 1;

	private native int nativeUpdate();

	private native void nativeTouch(int action, int x, int y);

	private native void nativeSetContext(int w, int h);

	private native void nativeResetContext();

	private native void nativePause();

	private native void nativeStop();

	private native void nativeResume();

	private native void nativeSetWorkPath(String packname);

	private boolean OP_Playing = false;
	private String OP_Path;
	private VideoView OP_View;
	private int OP_Pos = 0;
	private String mPathName = "";
	private String TAG = "nge2";
	static {
		System.loadLibrary("nge2app-jni");
	}
	public boolean m_need_init = true;

	private void prepareGLView() {
		RelativeLayout layout = new RelativeLayout(this);
		// 添加游戏绘画试图
		m_view = new NGE2View(this);
		DisplayMetrics dm = new DisplayMetrics();
		getWindowManager().getDefaultDisplay().getMetrics(dm);
		// 計算屏幕比例，以滿足自适应
		int scrWidth = dm.widthPixels;
		int scrHeight = dm.heightPixels;
		if ((scrWidth / (double) scrHeight) >= (640 / 960.0)) {
			scrWidth = scrHeight * 2 / 3;
		} else {
			scrHeight = scrWidth * 3 / 2;
		}
		LayoutParams params = new LayoutParams(scrWidth, scrHeight);
		params.addRule(RelativeLayout.CENTER_IN_PARENT);
		layout.addView(m_view, 0, params);
		// 添加广告视图
		params = new LayoutParams(LayoutParams.FILL_PARENT, LayoutParams.WRAP_CONTENT);
		params.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM);
		setContentView(layout);
		m_renderer = new NGE2Renderer();
		m_view.setRenderer(m_renderer);
	}

	@Override
	public void onCreate(Bundle savedInstanceState) {
		mPathName = Environment.getExternalStorageDirectory() + File.separator + getPackageName() + File.separator;
		super.onCreate(savedInstanceState);
		UtilsKit.app = this;
		//UtilsKit.registeredDevices();
		//检查资源文件是否存在，如果存在直接启动
		if (new File(mPathName).exists()) {
			init();
		} else {//如果不存在则解压一份
			new UnZipFile(this, new UnZipFile.CallBack() {
				@Override
				public void onCallBack() {
					init();
				}
			}).execute("Resource.zip", mPathName);
		}
	}
	//初始化NGE2
	private void init() {
		nativeSetWorkPath(mPathName);
		nativeSetPackname(getPackageName());
		nativeCreate();
		DisplayMetrics displayMetrics = new DisplayMetrics();
		getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);
		g_height = displayMetrics.heightPixels;
		g_width = displayMetrics.widthPixels;
		nativeSetContext(g_height, g_width);
		if (OP_Path != null) {
			OP_View = new VideoView(NGE2.this);

			//Fix relative path to absolute
			File file = new File(OP_Path);
			String fn = null;
			if (file.isAbsolute())
				fn = OP_Path;
			else
				fn = mPathName + OP_Path;

			OP_View.setVideoPath(fn);
			OP_View.setMediaController(new MediaController(this));
			OP_View.requestFocus();
			OP_View.setOnCompletionListener(new MediaPlayer.OnCompletionListener() {
				@Override
				public void onCompletion(MediaPlayer mp) {
					OP_Playing = false;
					prepareGLView();
				}
			});
			OP_Playing = true;
			setContentView(OP_View);
		} else
			prepareGLView();
		Log.i(TAG, "Create.");
	}

	@Override
	public void onPause() {
		Log.i(TAG, "Pause.");
		super.onPause();
		if (!OP_Playing) {
			m_view.onPause();
			nativePause();
		}
	}

	@Override
	public void onStop() {
		Log.i(TAG, "Stop.");
		super.onStop();
		if (OP_Playing) {
			OP_View.pause();
			OP_Pos = OP_View.getCurrentPosition();
		} else {
			nativeStop();
		}
	}

	@Override
	public void onResume() {
		Log.i(TAG, "Resume.");
		super.onResume();
		if (OP_Playing) {
			OP_View.seekTo(OP_Pos);
			OP_View.start();
		} else {
			if (m_view != null) {
				m_view.onResume();
				nativeResume();
			}
		}
	}

	@Override
	public void onDestroy() {
		Log.i(TAG, "Destory.");
		super.onDestroy();
		if (OP_Playing) {
			OP_View.destroyDrawingCache();
		} else {
			nativeFinalize();
			m_need_init = true;
		}
	}

	private long lastback = 0;

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		long now = new Date().getTime();
		if (keyCode == KeyEvent.KEYCODE_BACK) {
			if (now - lastback <= 2500) {
				android.os.Process.killProcess(android.os.Process.myPid());
			} else {
				Toast.makeText(NGE2.this, "再按一次退出游戏", Toast.LENGTH_LONG).show();
				lastback = now;
			}
			return true;
		}
		return super.onKeyDown(keyCode, event);
	}

	private class NGE2View extends GLSurfaceView {
		public NGE2View(Context context) {
			super(context);
		}

		@Override
		public void setRenderer(Renderer renderer) {
			super.setRenderer(renderer);
		}

		@Override
		public boolean onTouchEvent(MotionEvent event) {
			final int eventAction = event.getAction();
			final int eventX = (int) event.getX();
			final int eventY = (int) event.getY();
			queueEvent(new Runnable() {
				public void run() {
					nativeTouch(eventAction, eventX, eventY);
				}
			});
			return true;
		}
	}

	private class NGE2Renderer implements Renderer {
		@Override
		public void onSurfaceCreated(GL10 gl, EGLConfig config) {
			Log.i(TAG, "onSurfaceCreated.");
			if (m_need_init) {
				m_need_init = false;
				Log.i(TAG, "nativeInitialize.");
				nativeInitialize();
			}
		}

		@Override
		public void onSurfaceChanged(GL10 gl, int width, int height) {
			Log.i(TAG, "onSurfaceChanged.");
			String msg = String.format("w = %d,h = %d", width, height);
			Log.d(TAG, msg);
			gl.glViewport(0, 0, width, height);
			nativeSetContext(width, height);
			nativeResetContext();
		}

		@Override
		public void onDrawFrame(GL10 gl) {
			if (nativeUpdate() == APP_QUIT)
				onDestroy();
		}
	}
}
