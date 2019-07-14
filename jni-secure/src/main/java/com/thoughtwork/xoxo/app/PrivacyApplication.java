package com.thoughtwork.xoxo.app;

import android.app.Application;

public class PrivacyApplication extends Application {

    private static PrivacyApplication instance;

    @Override
    public void onCreate() {
        super.onCreate();
        instance = this;
    }
}
