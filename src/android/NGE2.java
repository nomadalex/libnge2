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
import com.iacger.reversi.R;

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

import org.libnge.nge2.LibCoolAudio;

public class NGE2 extends Activity
{
	private NGE2View m_view;
	private NGE2Renderer m_renderer;
	private LibCoolAudio m_audio;
	public int g_height;
	public int g_width;

	private static native void nativeSetContext(int w,int h);
	private static native void nativeSetWorkPath(String packname);

	private static native void nativeInitialize();

	public static final int APP_NORMAL = 0;
	public static final int APP_QUIT = 1;
	private static native int nativeUpdate();

	private static native void nativeFinalize();
	private static native void nativePause();
	private static native void nativeResetContext();
	private static native void nativeResume();
	private static native void nativeTouch(int action, int x, int y);

	static {
		System.loadLibrary("nge2app-jni");
	}

	private void processResource(Activity activity)
	{
		String[] files;
		try
		{
			files = activity.getAssets().list("");
		}
		catch (IOException e1)
		{
			return;
		}

		String mPathName = "/data/data/"+activity.getPackageName()+"/resource/";
		File mWorkPath = new File(mPathName);
		if(!mWorkPath.exists())
		{
			if(!mWorkPath.mkdirs())
			{
				new AlertDialog.Builder(activity)
				.setTitle("NGE2 ERROR")
				.setMessage("FAILED_DIR_CREATE")
				.setPositiveButton(android.R.string.ok, new OnClickListener(){
					@Override
					public void onClick(DialogInterface dialog, int which)
					{
						dialog.dismiss();
					}
				})
				.create()
				.show();
			}
		}
		for(int i = 0; i < files.length; i++)
		{
			try
			{
				String fileName = files[i];

				if(fileName.compareTo("images") == 0 ||
				   fileName.compareTo("sounds") == 0 ||
				   fileName.compareTo("webkit") == 0)
				{
					continue;
				}

				File outFile = new File(mWorkPath, fileName);
				if(outFile.exists()) continue;

				InputStream in = activity.getAssets().open(fileName);
				OutputStream out = new FileOutputStream(outFile);

				// Transfer bytes from in to out
				byte[] buf = new byte[1024];
				int len;
				while ((len = in.read(buf)) > 0)
				{
					out.write(buf, 0, len);
				}

				in.close();
				out.close();
			}
			catch (FileNotFoundException e)
			{
				e.printStackTrace();
			}
			catch (IOException e)
			{
				e.printStackTrace();
			}
		}
	}

	@Override public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);

		getWindow().addFlags(android.view.WindowManager.LayoutParams.FLAG_FULLSCREEN);
		requestWindowFeature(android.view.Window.FEATURE_NO_TITLE);

		DisplayMetrics displayMetrics = new DisplayMetrics();
		getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);
		g_height = displayMetrics.heightPixels;
		g_width = displayMetrics.widthPixels;
		nativeSetContext(g_width,g_height);
		processResource(this);
		Log.d("nge2",getApplication().getApplicationContext().getFilesDir().getAbsolutePath());
		nativeSetWorkPath(getPackageName());
		m_audio = new LibCoolAudio();
		m_view = new NGE2View(this);
		m_renderer = new NGE2Renderer();
		m_view.setRenderer(m_renderer);
		setContentView(m_view);
	}

	@Override public void onPause()
	{
		super.onPause();

		m_view.onPause();
		nativePause();
	}

	@Override public void onResume()
	{
		super.onResume();

		m_view.onResume();
		nativeResume();
	}

	@Override public void onStop() {
		super.onStop();
	}

	@Override public void onDestroy()
	{
		super.onDestroy();

		nativeFinalize();
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

		@Override public boolean onTouchEvent(MotionEvent event)
		{
			nativeTouch(event.getAction(), (int)event.getX(), (int)event.getY());
			return true;
		}
	}

	private class NGE2Renderer implements Renderer
	{
		private int w;
		private int h;
		public void onSurfaceCreated(GL10 gl, EGLConfig config) {
			// null
		}

		public void onSurfaceChanged(GL10 gl, int width, int height) {
			w = width;
			h = height;
			String msg = String.format("w = %d,h = %d", g_width,g_height);
			Log.d("nge2",msg);
			nativeSetContext(w,h);
			nativeResetContext();
		}

		private boolean m_is_first = true;
		public void onDrawFrame(GL10 gl)
		{
			if (m_is_first)
			{
				m_is_first = false;
				nativeInitialize();
			}
			if (nativeUpdate() == APP_QUIT)
				onDestroy();
		}
	}
}
