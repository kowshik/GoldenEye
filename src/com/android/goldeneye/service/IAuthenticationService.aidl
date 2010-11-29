package com.android.goldeneye.service;

import com.android.goldeneye.service.IAuthenticationCallback;

interface IAuthenticationService {

    int startAuthentication(String aUserName);
    int setOnAuthentication(IAuthenticationCallback aOnAuthenticationComplete);
    
    int setAuthenticationResult(boolean aAuthenticationResult);
}