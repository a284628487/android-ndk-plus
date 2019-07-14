package com.thoughtwork.xoxo;

import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.Signature;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

public class MainActivity extends AppCompatActivity {

    public final static String SHA1 = "MD5";

    private final String TAG = "MainActivity";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        TextView tv = (TextView) findViewById(R.id.sample_text);
        tv.setText(SignatureUtils.stringFromJNI());
    }

    @Override
    protected void onResume() {
        super.onResume();

        try {
            PackageInfo packageInfo = getPackageManager().getPackageInfo("com.thoughtwork.xoxo", PackageManager.GET_SIGNATURES);
            Signature[] signs = packageInfo.signatures;
            for (Signature sig : signs) {
                String tmp = getSignatureString(sig, SHA1);
                Log.e(TAG, tmp);

                String sig2 = SignatureUtils.getSignatureByType(SHA1);
                Log.e("getSignatureByType", "" + sig2);
            }
        } catch (PackageManager.NameNotFoundException e) {
            e.printStackTrace();
        }

        Log.e(TAG, "application = " + getApplication());
        Log.e(TAG, "applicationContext = " + getApplicationContext());
        Log.e(TAG, "applicationInfo.procName = " + getApplication().getApplicationInfo().processName);
        Log.e(TAG, "applicationInfo.pkgName = " + getApplication().getApplicationInfo().packageName);
        Log.e(TAG, "getSecureKey = " + SignatureUtils.getSecureKey());
        Log.e(TAG, "getApplicationFromNative = " + SignatureUtils.getApplicationFromNative());
    }

    public String getSignatureString(Signature sig, String type) {
        byte[] hexBytes = sig.toByteArray();
        String fingerprint = "error!";
        try {
            MessageDigest digest = MessageDigest.getInstance(type);
            if (digest != null) {
                byte[] digestBytes = digest.digest(hexBytes);
                StringBuilder sb = new StringBuilder();
                for (byte digestByte : digestBytes) {
                    sb.append((Integer.toHexString((digestByte & 0xFF) | 0x100)).substring(1, 3));
                }
                fingerprint = sb.toString();
            }
        } catch (NoSuchAlgorithmException e) {
            e.printStackTrace();
        }

        return fingerprint;
    }

}
