package com.android.goldeneye.service;

import java.util.Timer;
import java.util.TimerTask;

import com.android.goldeneye.*;
import com.android.goldeneye.service.AuthenticationService.Task;

import com.android.goldeneye.service.IAuthenticationService;
import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;
import android.widget.TextView;

public class FaceDetector extends Activity {

	private Handler serviceHandler;
	private int counter = 0;
	private TextView iTextView = null;
	private Task myTask = new Task();
	private boolean iServiceStarted = false;
	private boolean iServiceBound = false;
	private AuthServiceConnection iAuthServiceConnection = null;
	private IAuthenticationService iAuthService = null;
	
	protected void onCreate(Bundle aSavedState) {
		super.onCreate(aSavedState);
		
		setContentView(R.layout.main_goldeneye_tmp);
		serviceHandler = new Handler();
		serviceHandler.postDelayed(myTask, 1000L);
		Log.i("FaceRecognizerActivity", "onCreate");

		if(!iServiceStarted) {
			int err = StartService();
			if(err == -1)
				return;
			
			assert(iServiceStarted);
		}
		
		if(!iServiceBound) {
			int err = DoBindService();
			if(err == -1)
				return;
			
			assert(iServiceBound);
		}
	}
	
	protected int StartService() {
		Intent i = new Intent();
		i.setClassName("in.net.shashank.android.authenticationservice", "in.net.shashank.android.authenticationservice.AuthenticationService");
		
		ComponentName s = startService(i);
		if(s != null)
			iServiceStarted = true;
		
		if(!iServiceStarted)
			return -1;
		
		return 0;
	}
	
	protected int StopService() {
		return 0;
	}
	
	protected int DoBindService() {
		Intent i = new Intent();
		i.setClassName("in.net.shashank.android.authenticationservice", "in.net.shashank.android.authenticationservice.AuthenticationService");
		
		iAuthServiceConnection = new AuthServiceConnection();
		
		iServiceBound = bindService(i,(ServiceConnection)iAuthServiceConnection,Context.BIND_AUTO_CREATE);
		if(!iServiceBound) {
			Log.i("AuthServiceConnection", "Error binding to the service");
			return -1;
		}
		
		Log.i("AuthServiceConnection", "Bound to the service!");
	
		return 0;
	}

	protected int UnbindService() {
		unbindService(iAuthServiceConnection);
		return 0;
	}
	
	protected void onDestroy() {
		UnbindService();
		super.onDestroy();
		Log.i("FaceRecognizerActivity","onDestroy");
	}
	
	class Task implements Runnable {
		public void run() {
			counter+=10;
			if(counter >= 50) {
				//setResult(1);
				//iTextView.setText("Done!");
				iAuthServiceConnection.SendAuthenticationResult(true);
				finish();
				}
			else {
				serviceHandler.postDelayed(this,1000L);
			}
			Log.i(getClass().getSimpleName(), "Incrementing counter in the run method, " + String.valueOf(counter));
		}
	}

	class AuthServiceConnection implements ServiceConnection {

		public void onServiceConnected(ComponentName name, IBinder service) {
			// We are now connected to the service
			iAuthService = IAuthenticationService.Stub.asInterface(service);
			Log.i("AuthServiceConnection", "onServiceConnected()");
		}

		public void onServiceDisconnected(ComponentName name) {
			// Disconnected form the service
			iAuthService = null;
			Log.i("AuthServiceConnection", "onServiceDisconnected()");
		}
		
		public void SendAuthenticationResult(boolean aResult) {
			
			Log.i("AuthServiceConnection", "SendAuthenticationResult()");

			// continue with authentication
			if(iAuthService != null) {
				try {
					iAuthService.setAuthenticationResult(true);
				} catch (RemoteException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}
		}
	}
}
