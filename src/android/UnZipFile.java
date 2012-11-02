package org.libnge.nge2;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.text.MessageFormat;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

import org.libnge.ngetemplate.R;

import android.app.Dialog;
import android.content.Context;
import android.os.AsyncTask;
import android.widget.ProgressBar;
import android.widget.TextView;

/**
 * 解压数据库文件操作类
 */
public class UnZipFile extends AsyncTask<String, Integer, Integer> {
	private Context mContext = null;
	private ProgressBar progress = null;
	private TextView progress_label = null;
	private Dialog customDialog;
	private int streamLength = 0;
	private CallBack mCallBack;

	/**
	 * 解压zip数据库文件
	 * @param context activity
	 * @param SavePath 保存路径
	 * @param ResourcesIndex 资源文件索引
	 */
	public UnZipFile(Context pContext,CallBack pCallBack) {
		mContext = pContext;
		mCallBack = pCallBack;
	}

	@Override
	protected Integer doInBackground(String... params) {
		int byteLength = 4096*1000;
		int progress = 0;
		try {
			InputStream ZipIs = mContext.getAssets().open(params[0]);// 获取文件流
			BufferedOutputStream bufferStream = null; // 缓冲输出流
			ZipInputStream zis = new ZipInputStream(new BufferedInputStream(ZipIs));
			streamLength = (int)(ZipIs.available()*2);// 获取文件大小
			ZipEntry entry; // 每个zip条目的实例
			while ((entry = zis.getNextEntry()) != null) {
				try {
					int count;
					byte data[] = new byte[byteLength];
					String entryFileName = entry.getName();
					File saveFile = new File(params[1] + "/" + entryFileName);
			        if(entry.isDirectory()){
		                String dirstr = params[1] + entryFileName;
		                dirstr = new String(dirstr.getBytes("8859_1"), "GB2312");
					    File f=new File(dirstr);
					    f.mkdirs();
		                continue;
			        }
					FileOutputStream fos = new FileOutputStream(saveFile);
					bufferStream = new BufferedOutputStream(fos, byteLength);
					while ((count = zis.read(data, 0, byteLength)) != -1) {
						bufferStream.write(data, 0, count);
						progress += count;
						publishProgress(progress); // 更新进度条
					}
					bufferStream.flush();
					bufferStream.close();
				} catch (Exception ex) {
					ex.printStackTrace();
				}
			}
			zis.close();
		} catch (Exception ex) {
			ex.printStackTrace();
		}
		return (int)streamLength;
	}

	@Override
	protected void onPostExecute(Integer result) {
		super.onPostExecute(result);
		progress.setProgress(result);
		if (customDialog != null)
			customDialog.dismiss();
		if(mCallBack != null){
			mCallBack.onCallBack();
		}
	}

	@Override
	protected void onPreExecute() {
		super.onPreExecute();
		customDialog = new Dialog(mContext);
		customDialog.setTitle("文件解压");
		customDialog.setContentView(R.layout.progress_dialog);
		progress = (ProgressBar)customDialog.findViewById(R.id.progressBar);
		progress_label = (TextView)customDialog.findViewById(R.id.progress_label);
	    customDialog.show();
	}

	private MessageFormat mf = new MessageFormat("第一次启动解压资源文件，请稍后({0}%)...");
	@Override
	protected void onProgressUpdate(Integer... values) {
		super.onProgressUpdate(values);
		/**
		 * 更改进度条状态
		 */
		if (streamLength != progress.getMax()){
			progress.setMax((int)streamLength);
		}
		progress.setProgress(values[0]);
		if(values[0]>streamLength){
			values[0] = streamLength;
		}
		int value = values[0]*100/streamLength;
		if(value<0)value = 0;
		if(value>100)value = 100;
		progress_label.setText(mf.format(new String[]{String.valueOf(value)}));
	}
	public interface CallBack{
		public void onCallBack();
	}
}
