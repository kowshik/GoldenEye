package com.android.goldeneye.service;

/*
 * Service sends the result and does not wait for the client to return
 */
oneway interface IAuthenticationCallback {

    /*
     * aAuthenticationResult = true, if aUserName is authenticated
     */
    void onAuthenticationComplete(String aUserName, boolean aAuthenticationResult);
}