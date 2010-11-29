package com.android.goldeneye.service;

import com.android.goldeneye.service.IAuthenticationCallback;
import com.android.goldeneye.service.IAuthenticationService;

import java.util.Random;

import android.app.Service;
import android.content.Intent;
import android.os.Handler;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;

public class AuthenticationService extends Service {

	private Handler serviceHandler;
	private int counter;
	private Task myTask = new Task();
	
	boolean iInitialized = false;
	
	//TODO: Map these to application
	//TODO: Potential Achilles heal!
	IAuthenticationCallback iOnAuthenticationComplete = null;

	@Override
	public IBinder onBind(Intent arg0) {
		Log.d(getClass().getSimpleName(), "onBind() ++");
		return iAuthStub;
	}

	private IAuthenticationService.Stub iAuthStub = new IAuthenticationService.Stub() {
		public int getCounter() throws RemoteException {
			return counter;
		}

		public int startAuthentication(String aUserName) throws RemoteException {
			// TODO Auto-generated method stub
			Log.i("Service", "Starting Authentication");
			Intent i = new Intent(getBaseContext(),com.android.goldeneye.StartScreen.class);
			i.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
			startActivity(i);
			if(iInitialized == true) {
				Log.i("Service","Starting Handler");
/*				serviceHandler = new Handler();
				serviceHandler.postDelayed(myTask, 1000L);
*/			}
			return counter;
		}
		
	    public int setOnAuthentication(IAuthenticationCallback aOnAuthenticationComplete) {
	    	iOnAuthenticationComplete = aOnAuthenticationComplete;
	    	return 0;
	    }

		public int setAuthenticationResult(boolean aAuthenticationResult)
				throws RemoteException {
			// TODO Auto-generated method stub
			Log.i("AuthenticationResult", "SetOnAuthenticationResult");
			iOnAuthenticationComplete.onAuthenticationComplete("user", aAuthenticationResult);
			return 0;
		}

	};
	
	@Override
	public void onCreate() {
		super.onCreate();
		Log.d(getClass().getSimpleName(),"onCreate()");
		iInitialized = false;
	}
	
	@Override
	public void onDestroy() {
		super.onDestroy();
		serviceHandler.removeCallbacks(myTask);
		serviceHandler = null;
		Log.d(getClass().getSimpleName(),"onDestroy()");
		iInitialized = false;
	}
	
	@Override
	public void onStart(Intent intent, int startId) {
		super.onStart(intent, startId);
		Log.d(getClass().getSimpleName(), "onStart()");
		if(!iInitialized && serviceHandler == null) {
/*			serviceHandler = new Handler();
			serviceHandler.postDelayed(myTask, 1000L);
*/		}
		iInitialized = true;
	}
	
	class Task implements Runnable {
		public void run() {
			counter+=10;
			if(counter >= 100) {
				if(iOnAuthenticationComplete != null)
					try {
						Random generator = new Random();
						boolean val = generator.nextBoolean();
						iOnAuthenticationComplete.onAuthenticationComplete("user", val);
						Log.i(getClass().getSimpleName(), "Done!");
						counter = 0;
						//serviceHandler = null;
						return;
					} catch (RemoteException e) {
						// TODO Auto-generated catch block
						// e.printStackTrace();
						Log.e("AuthenticationService", "onAuthenticationComplete");
					}
			}
			else {
				serviceHandler.postDelayed(this,1000L);
			}
			Log.i(getClass().getSimpleName(), "Incrementing counter in the run method, " + String.valueOf(counter));
		}
	}
}
