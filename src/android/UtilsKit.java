package org.libnge.nge2;

import android.content.Context;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.telephony.TelephonyManager;
import android.util.Log;

public class UtilsKit extends Object{
	private static final String TAG = "LibUtilsKit";

	public int init() {
		return 0;
	}
	public static NGE2 app;
	/**
	 * @param adKey adkey
	 * @param adType 目前仅支持admob
	 * @return 广告袋标识符
	 */
	public int CreateAdHandle(final String adKey,String adType){

		return 1;
	}
	public void SetAdPos(int adHandle,int x,int y){
		//View adView = app.getAdView();
		//adView.setPadding(x, y, adView.getRight(), adView.getBottom());
		String msg = String.format("adHandle = %d ,x = %d,y = %d", adHandle , x, y);
		Log.d(TAG,msg);
	}
	public void ShowAd(int adHandle,int bShow){

	}
	public void DestroyAdHandle(int adHandle){
		//app.DestroyAdHandle(adHandle);
		String msg = String.format("adHandle = %d", adHandle);
		Log.d(TAG,msg);
	}


	/**
	 * 支付接口定义
	 * @param title 交易物品名称
	 * @param body 交易明细
	 * @param total 交易总价格
	 * @return 返回成功与否，目前0失败 1成功
	 */
	private int result = 0;
	private native void nativePayFeedBack(int error,String errStr);
	public int Pay(final int payHandle,final String title,final String body,final String total){
		result = 0;
		return result;
	}
	/**
	 * @param payType 目前只支持 alipay
	 * @param partner 账户ID。用签约支付宝账号登录ms.alipay.com后，在账户信息页面获取。
	 * @param seller 账户ID。用签约支付宝账号登录ms.alipay.com后，在账户信息页面获取。
	 * @param privateRSA 商户（RSA）私钥
	 * @param publicRSA 支付宝（RSA）公钥  用签约支付宝账号登录ms.alipay.com后，在密钥管理页面获取。
	 * @return
	 */
	public int CreatePayHandle(String payType,String partner,String seller,String privateRSA,String publicRSA){
		return 0;
	}


	public int Share(int type,final String str,final String imgName){
		return 0;
	}

	public void MoreGame(int type){
	}

	/**
	 * 获取机器的mac地址
	 * 需要添加权限
	 * <uses-permission android:name="android.permission.ACCESS_WIFI_STATE" />
	 * @return
	 */
	public static String getMacAddress() {
        WifiManager wifi = (WifiManager) app.getSystemService(Context.WIFI_SERVICE);
        WifiInfo info = wifi.getConnectionInfo();
        return info.getMacAddress();
    }

	public static String getCustomIMSI(){
		TelephonyManager telephonyManager=(TelephonyManager) app.getSystemService(Context.TELEPHONY_SERVICE);
		return telephonyManager.getSubscriberId();
	}

	/**
	 * 返回当前程序版本名
	 */
	public static String getAppVersionName() {
		return "";
	}

	public static  void registeredDevices(){
    }
}
