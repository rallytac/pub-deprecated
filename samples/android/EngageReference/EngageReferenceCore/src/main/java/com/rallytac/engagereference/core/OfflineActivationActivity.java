//
//  Copyright (c) 2019 Rally Tactical Systems, Inc.
//  All rights reserved.
//

package com.rallytac.engagereference.core;

import android.content.Intent;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.support.v7.app.ActionBar;
import android.support.v7.app.AppCompatActivity;
import android.view.MenuItem;
import android.view.View;
import android.view.animation.AlphaAnimation;
import android.widget.ImageView;

import com.google.zxing.integration.android.IntentIntegrator;
import com.google.zxing.integration.android.IntentResult;

public class OfflineActivationActivity extends AppCompatActivity
{
    private static String TAG = OfflineActivationActivity.class.getSimpleName();

    public static String EXTRA_LICENSE_KEY = "$LICENSEKEY";
    public static String EXTRA_DEVICE_ID = "$DEVICEID";
    public static String EXTRA_ACTIVATION_CODE = "$ACTIVATIONCODE";

    private String _licenseKey = null;
    private String _deviceId = null;
    private Bitmap _bm = null;
    private String _activationCode = null;
    private Intent _resultIntent = new Intent();

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_offline_activation);

        ActionBar ab = getSupportActionBar();
        if(ab != null)
        {
            ab.setDisplayHomeAsUpEnabled(true);
        }

        Intent intent = getIntent();
        if(intent != null)
        {
            _licenseKey = intent.getStringExtra(EXTRA_LICENSE_KEY);
            _deviceId = intent.getStringExtra(EXTRA_DEVICE_ID);
        }

        buildBitmap();
        setBitmap();

        AlphaAnimation animS1 = new AlphaAnimation(0.0f, 1.0f);
        animS1.setDuration(5000);
        animS1.setRepeatCount(0);
        findViewById(R.id.tvStep1ExtraDescription).startAnimation(animS1);

        AlphaAnimation animS2 = new AlphaAnimation(0.0f, 1.0f);
        animS2.setDuration(10000);
        animS2.setRepeatCount(0);
        findViewById(R.id.tvStepTitle2).startAnimation(animS2);
        findViewById(R.id.tvStep2Description).startAnimation(animS2);
        findViewById(R.id.ivScanOfflineActivationQrCode).startAnimation(animS2);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item)
    {
        switch (item.getItemId())
        {
            case android.R.id.home:
                finish();
                return true;
        }

        return super.onOptionsItemSelected(item);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent intent)
    {
        if(requestCode == IntentIntegrator.REQUEST_CODE)
        {
            IntentResult result = IntentIntegrator.parseActivityResult(requestCode, resultCode, intent);
            if(result != null)
            {
                _activationCode = result.getContents();

                _resultIntent.putExtra(EXTRA_ACTIVATION_CODE, _activationCode);
                setResult(RESULT_OK, _resultIntent);
                finish();
            }
        }
    }

    private void setBitmap()
    {
        ImageView iv = findViewById(R.id.ivQrCode);

        if(_bm != null)
        {
            iv.setImageBitmap(_bm);
            iv.setVisibility(View.VISIBLE);
        }
        else
        {
            iv.setVisibility(View.INVISIBLE);
        }
    }

    private void buildBitmap()
    {
        if(_bm == null && !Utils.isEmptyString(_licenseKey) && !Utils.isEmptyString(_deviceId))
        {
            String entitlementKey = getString(R.string.licensing_entitlement);
            String stringToHash = _licenseKey + _deviceId + entitlementKey;
            String hValue = Utils.md5HashOfString(stringToHash);

            StringBuilder sb = new StringBuilder();
            sb.append(getString(R.string.offline_licensing_activation_url));
            sb.append("?");
            sb.append("licenseId=");
            sb.append(_licenseKey);
            sb.append("&");
            sb.append("deviceSerialNumber=");
            sb.append(_deviceId);
            sb.append("&");
            sb.append("h=");
            sb.append(hValue);

            String url = sb.toString();

            _bm = Utils.stringToQrCodeBitmap(url, Constants.QR_CODE_WIDTH, Constants.QR_CODE_HEIGHT);
        }
    }

    public void onClickScanOfflineActivationQrCode(View view)
    {
        IntentIntegrator ii = new IntentIntegrator(this);

        ii.setCaptureActivity(OrientationIndependentQrCodeScanActivity.class);
        ii.setPrompt("Scan the activation QR code");
        ii.setBeepEnabled(true);
        ii.setOrientationLocked(false);
        ii.setDesiredBarcodeFormats(IntentIntegrator.QR_CODE_TYPES);
        ii.setBarcodeImageEnabled(true);
        ii.setTimeout(10000);
        ii.initiateScan();
    }
}
