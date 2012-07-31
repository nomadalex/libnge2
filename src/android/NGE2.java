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
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.os.Bundle;

import android.app.Activity;
import android.app.AlertDialog;
import android.opengl.GLSurfaceView;
import android.opengl.GLSurfaceView.Renderer;

import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.util.Log;
import android.view.MotionEvent;
import android.util.DisplayMetrics;

import android.media.MediaPlayer;
import android.widget.MediaController;
import android.widget.VideoView;

public class NGE2 extends Activity
{
	private NGE2View m_view;
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
	private native void nativeSetContext(int w,int h);
	private native void nativeResetContext();

	private native void nativePause();
	private native void nativeStop();
	private native void nativeResume();

	private boolean OP_Playing = false;
	private String OP_Path;
	private VideoView OP_View;
	private int OP_Pos = 0;

	private String TAG = "nge2";
	static {
		System.loadLibrary("openal");
		System.loadLibrary("nge2app-jni");
	}

	public boolean m_need_init = true;

	private void prepareGLView() {
		m_view = new NGE2View(this);
		m_renderer = new NGE2Renderer();
		m_view.setRenderer(m_renderer);

		setContentView(m_view);
	}

	@Override public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);

		nativeSetPackname(getPackageName());
		nativeCreate();

		getWindow().addFlags(android.view.WindowManager.LayoutParams.FLAG_FULLSCREEN);
		requestWindowFeature(android.view.Window.FEATURE_NO_TITLE);

		DisplayMetrics displayMetrics = new DisplayMetrics();
		getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);
		g_height = displayMetrics.heightPixels;
		g_width = displayMetrics.widthPixels;

		if (OP_Path != null) {
			OP_View = new VideoView(getApplicationContext());
			OP_View.setVideoPath(OP_Path);
			OP_View.setMediaController(new MediaController(this));
			OP_View.requestFocus();
			OP_View.setOnCompletionListener(new MediaPlayer.OnCompletionListener() {
					@Override
					public void onCompletion(MediaPlayer mp)
					{
						OP_Playing = false;
						prepareGLView();
					}
				});
			OP_Playing = true;
			setContentView(OP_View);
		}
		else
			prepareGLView();

		Log.i(TAG, "Create.");
	}

	@Override public void onPause() {
		Log.i(TAG, "Pause.");
		super.onPause();

		if (!OP_Playing) {
			m_view.onPause();
			nativePause();
		}
	}

	@Override public void onStop() {
		Log.i(TAG, "Stop.");

		super.onStop();
		if (OP_Playing) {
			OP_View.pause();
			OP_Pos = OP_View.getCurrentPosition();
		}
		else {
			nativeStop();
		}
	}

	@Override public void onResume()
	{
		Log.i(TAG, "Resume.");
		super.onResume();

		if (OP_Playing) {
			OP_View.seekTo(OP_Pos);
			OP_View.start();
		}
		else {
			m_view.onResume();
			nativeResume();
		}
	}

	@Override public void onDestroy()
	{
		Log.i(TAG, "Destory.");
		super.onDestroy();

		if (OP_Playing) {
			OP_View.destroyDrawingCache();
		}
		else {
			nativeFinalize();
			m_need_init = true;
		}
	}
	
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) { 
		if (keyCode == KeyEvent.KEYCODE_BACK) {
			
			new AlertDialog.Builder(this)
			.setTitle("NGE2")
			.setMessage("Quit this application?")
			.setPositiveButton(android.R.string.ok,
					new OnClickListener() {
						@Override
						public void onClick(DialogInterface dialog,
								int which) {
							dialog.dismiss();
							android.os.Process.killProcess(android.os.Process.myPid());
						}
					})
			.setNegativeButton(android.R.string.cancel,null).
			create().show();
			return true;
		}
		return super.onKeyDown(keyCode, event);
	}

	private class NGE2View extends GLSurfaceView
	{
		public NGE2View(Context context)
		{
			super(context);
		}

		@Override public void setRenderer(Renderer renderer)
		{
			super.setRenderer(renderer);
		}
		
		@Override
		public boolean onTouchEvent(final MotionEvent event) {
			queueEvent(new Runnable(){
										public void run(){
											nativeTouch(event.getAction(),(int)event.getX(),(int)event.getY());
										}
									 }
			);
			
			return true;
		}
	}

	private class NGE2Renderer implements Renderer
	{
		@Override public void onSurfaceCreated(GL10 gl, EGLConfig config) {
			Log.i(TAG, "onSurfaceCreated.");
			if (m_need_init) {
				m_need_init = false;
				Log.i(TAG, "nativeInitialize.");
				nativeInitialize();
			}
		}

		@Override public void onSurfaceChanged(GL10 gl, int width, int height) {
			Log.i(TAG, "onSurfaceChanged.");
			String msg = String.format("w = %d,h = %d", width, height);
			Log.d(TAG,msg);
			gl.glViewport(0, 0, width, height);
			nativeSetContext(width, height);
			nativeResetContext();
		}

		@Override public void onDrawFrame(GL10 gl)
		{
			if (nativeUpdate() == APP_QUIT)
				onDestroy();
		}
	}
}
