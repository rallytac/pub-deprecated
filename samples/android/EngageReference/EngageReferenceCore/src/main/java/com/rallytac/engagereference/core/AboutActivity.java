//
//  Copyright (c) 2019 Rally Tactical Systems, Inc.
//  All rights reserved.
//

package com.rallytac.engagereference.core;

import android.app.ProgressDialog;
import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.provider.Settings;
import android.support.v7.app.ActionBar;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.text.Editable;
import android.text.SpannableString;
import android.text.TextWatcher;
import android.text.method.LinkMovementMethod;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.MenuItem;
import android.view.View;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import com.google.zxing.integration.android.IntentIntegrator;
import com.google.zxing.integration.android.IntentResult;
import com.rallytac.engage.engine.Engine;

import org.json.JSONObject;

import java.util.Date;

public class AboutActivity extends
                                AppCompatActivity
                           implements
                                LicenseActivationTask.ITaskCompletionNotification

{
    private static String TAG = AboutActivity.class.getSimpleName();

    private int OFFLINE_ACTIVATION_REQUEST_CODE = 771;

    private enum KeyType {ktUnknown, ktPerpetual, ktExpires};
    private enum ScanType {stUnknown, stLicenseKey, stActivationCode};

    private ImageView _ivAppLogo;
    private TextView _tvLicenseHeader;
    private TextView _tvLicensingMessage;
    private EditText _etDeviceId;
    private EditText _etLicenseKey;
    private EditText _etActivationCode;
    private boolean _creating;
    private ScanType _scanType;
    private ProgressDialog _progressDialog = null;
    private boolean _scanning = false;
    private String _lastSavedKey = "";
    private String _lastSavedActivationCode = "";


    private int _numberOfClicksOfAppLogo = 0;

    private class InternalDescriptor
    {
        public KeyType _type;
        public String _deviceId;
        public String _key;
        public String _activationCode;
        public Date _expires;
        public String _expiresFormatted = null;

        public boolean isValid()
        {
            return _type != KeyType.ktUnknown;
        }

        public boolean equals(InternalDescriptor ld)
        {
            return Utils.stringsMatch(_deviceId, ld._deviceId) &&
                    Utils.stringsMatch(_key, ld._key) &&
                    Utils.stringsMatch(_activationCode, ld._activationCode);
        }
    }

    private InternalDescriptor _activeLd = null;
    private InternalDescriptor _newLd = null;

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        Globals.getEngageApplication().pauseLicenseActivation();

        _creating = true;
        setContentView(R.layout.activity_about);

        ActionBar ab = getSupportActionBar();
        if(ab != null)
        {
            ab.setDisplayHomeAsUpEnabled(true);
        }

        // This is for developer-related testing.  How it works is that if you
        // click the app logo 7 times, the app toggles developer mode
        {
            _ivAppLogo = findViewById(R.id.ivAppLogo);
            _ivAppLogo.setOnClickListener(new View.OnClickListener()
            {
                @Override
                public void onClick(View v)
                {
                    _numberOfClicksOfAppLogo++;
                    if(_numberOfClicksOfAppLogo >= 7)
                    {
                        _numberOfClicksOfAppLogo = 0;
                        boolean devModeActive = Globals.getSharedPreferences().getBoolean(PreferenceKeys.DEVELOPER_MODE_ACTIVE, false);
                        devModeActive = !devModeActive;
                        Globals.getSharedPreferencesEditor().putBoolean(PreferenceKeys.DEVELOPER_MODE_ACTIVE, devModeActive);
                        Globals.getSharedPreferencesEditor().apply();

                        if(devModeActive)
                        {
                            Toast.makeText(AboutActivity.this, "Developer mode activated", Toast.LENGTH_LONG).show();
                        }
                        else
                        {
                            Toast.makeText(AboutActivity.this, "Developer mode deactivated", Toast.LENGTH_LONG).show();
                        }
                    }
                }
            });
        }

        // Get the active license descriptor
        _activeLd = parseIntoInternalDescriptor(Globals.getEngageApplication()
                                                .getEngine()
                                                .engageGetActiveLicenseDescriptor());

        // At this point, the new one is the same as the active one
        _newLd = _activeLd;

        _tvLicenseHeader = findViewById(R.id.tvLicenseHeader);
        _tvLicensingMessage = findViewById(R.id.tvLicensingMessage);
        _etDeviceId = findViewById(R.id.etDeviceId);
        _etLicenseKey = findViewById(R.id.etLicenseKey);
        _etLicenseKey.addTextChangedListener(new TextWatcher()
        {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after)
            {
            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count)
            {
            }

            @Override
            public void afterTextChanged(Editable s)
            {
                _etActivationCode.setText(null);
                userChangedLicensedData();
            }
        });
        _etActivationCode = findViewById(R.id.etActivationCode);
        _etActivationCode.addTextChangedListener(new TextWatcher()
        {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after)
            {
            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count)
            {
            }

            @Override
            public void afterTextChanged(Editable s)
            {
                userChangedLicensedData();
            }
        });

        String s = _activeLd._deviceId;
        if(Utils.isEmptyString(s))
        {
            s = "unknown";
        }

        _lastSavedKey = Globals.getSharedPreferences().getString(PreferenceKeys.USER_LICENSING_KEY, "");
        _lastSavedActivationCode = Globals.getSharedPreferences().getString(PreferenceKeys.USER_LICENSING_ACTIVATION_CODE, "");

        _etDeviceId.setText(s);
        _etLicenseKey.setText(_lastSavedKey);
        _etActivationCode.setText(_lastSavedActivationCode);

        String versionInfo;

        versionInfo = BuildConfig.VERSION_NAME + " (Engine version " + Globals.getEngageApplication().getEngine().engageGetVersion() + ")";

        ((TextView)findViewById(R.id.tvVersion)).setText(versionInfo);

        setTitle(R.string.title_about);

        updateUi();

        _creating = false;
    }

    @Override
    protected void onResume()
    {
        Globals.getEngageApplication().pauseLicenseActivation();
        super.onResume();
    }

    @Override
    protected void onStop()
    {
        /*
        if(!_scanning)
        {
            saveLicenseData();
        }

        Globals.getEngageApplication().resumeLicenseActivation();
        */

        super.onStop();
    }

    private void saveLicenseData()
    {
        if(_newLd.isValid())
        {
            String key = _etLicenseKey.getText().toString();
            String ac = _etActivationCode.getText().toString();

            if (!Utils.isEmptyString(key))
            {
                if (Utils.isEmptyString(ac))
                {
                    ac = "";
                }

                if (key.compareTo(_lastSavedKey) != 0 || ac.compareTo(_lastSavedActivationCode) != 0)
                {
                    _lastSavedKey = key;
                    _lastSavedActivationCode = ac;

                    Log.i(TAG, "saving licensing [" + getString(R.string.licensing_entitlement) + "] [" + key + "] [" + ac + "]");

                    Globals.getSharedPreferencesEditor().putString(PreferenceKeys.USER_LICENSING_KEY, key);
                    Globals.getSharedPreferencesEditor().putString(PreferenceKeys.USER_LICENSING_ACTIVATION_CODE, ac);
                    Globals.getSharedPreferencesEditor().apply();

                    // Put the new license into effect
                    Globals.getEngageApplication().getEngine().engageUpdateLicense(getString(R.string.licensing_entitlement), key, ac);
                }
            }
        }
    }

    private void userChangedLicensedData()
    {
        if(!_creating)
        {
            updateNewLdFromEnteredData();
            updateUi();
        }
    }

    private InternalDescriptor parseIntoInternalDescriptor(String jsonData)
    {
        InternalDescriptor rc = new InternalDescriptor();
        rc._type = KeyType.ktUnknown;

        try
        {
            JSONObject obj = new JSONObject(jsonData);

            int type = obj.optInt(Engine.JsonFields.License.type, 0);
            long unixSeconds = obj.optInt(Engine.JsonFields.License.expires, 0);
            String expiresFormatted = obj.optString(Engine.JsonFields.License.expiresFormatted, "");

            rc._type = KeyType.values()[type];
            rc._deviceId = obj.optString(Engine.JsonFields.License.deviceId, "");
            if(Utils.isEmptyString(rc._deviceId))
            {
                throw new Exception("invalid device id");
            }

            rc._key = obj.optString(Engine.JsonFields.License.key, "");
            rc._activationCode = obj.optString(Engine.JsonFields.License.activationCode, "");
            if(unixSeconds > 0)
            {
                rc._expires = Utils.javaDateFromUnixSeconds(unixSeconds);
                rc._expiresFormatted = Utils.formatDateUtc(rc._expires);
            }
            else
            {
                if(rc._type == KeyType.ktExpires)
                {
                    rc._type = KeyType.ktUnknown;
                    rc._expires = null;
                    rc._expiresFormatted = null;
                }
            }
        }
        catch (Exception e)
        {
            rc = new InternalDescriptor();
            rc._type = KeyType.ktUnknown;
        }

        return rc;
    }

    private boolean isEnteredDataValid()
    {
        String key = _etLicenseKey.getText().toString();
        String ac = _etActivationCode.getText().toString();
        InternalDescriptor tmp = parseIntoInternalDescriptor(Globals.getEngageApplication()
                                .getEngine()
                                .engageGetLicenseDescriptor(getString(R.string.licensing_entitlement), key, ac));

        return (tmp != null && tmp._type != KeyType.ktUnknown);
    }

    private void updateNewLdFromEnteredData()
    {
        String key = _etLicenseKey.getText().toString();
        String ac = _etActivationCode.getText().toString();

        _newLd = parseIntoInternalDescriptor(Globals.getEngageApplication()
                                                    .getEngine()
                                                    .engageGetLicenseDescriptor(getString(R.string.licensing_entitlement), key, ac));
    }

    private void updateUi()
    { final String limitedTxMsg = "You can continue to use the application but won't be able to transmit for more than 3 seconds at a time.";

        StringBuilder sb = new StringBuilder();

        if(_activeLd.equals(_newLd))
        {
            // If active and new are the same, we'll build from the active
            if(_activeLd.isValid())
            {
                if(_activeLd._type == KeyType.ktExpires)
                {
                    Date dt = new Date();
                    if(_activeLd._expires.after(dt))
                    {
                        sb.append("Your existing license expires " + Utils.formattedTimespan(dt.getTime(), _activeLd._expires.getTime()) + ".");
                    }
                    else
                    {
                        sb.append("Your existing license expired on " + _activeLd._expiresFormatted + ".  " + limitedTxMsg);
                    }
                }
                else
                {
                    sb.append("Your existing license never expires and requires an activation code.");
                    //tryAutoActivate = true;
                }
            }
            else
            {
                sb.append("You don't yet have a valid license.  " + limitedTxMsg);
            }
        }
        else
        {
            if(_newLd.isValid())
            {
                if(_newLd._type == KeyType.ktExpires)
                {
                    Date dt = new Date();
                    if(_newLd._expires.after(dt))
                    {
                        sb.append("This license expires " + Utils.formattedTimespan(dt.getTime(), _newLd._expires.getTime()) + ".");
                    }
                    else
                    {
                        sb.append("This license expired on " + _newLd._expiresFormatted + ".  " + limitedTxMsg);
                    }
                }
                else
                {
                    sb.append("This license never expires but requires an activation code.");
                    //tryAutoActivate = true;
                }
            }
            else
            {
                sb.append("This is not a valid license.  " + limitedTxMsg);
            }
        }

        _tvLicensingMessage.setText(sb.toString());

        /*
        // Should we try auto-activation?
        if(tryAutoActivate)
        {
            String ac = _etActivationCode.getText().toString();
            if(Utils.isEmptyString(ac))
            {
                attemptOnlineActivation();
            }
        }
        */
    }

    @Override
    public void finish()
    {
        saveLicenseData();
        Globals.getEngageApplication().resumeLicenseActivation();
        super.finish();
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
            _scanning = false;

            IntentResult result = IntentIntegrator.parseActivityResult(requestCode, resultCode, intent);
            if(result != null)
            {
                String scannedString = result.getContents();

                if (!Utils.isEmptyString(scannedString))
                {
                    if(_scanType == ScanType.stLicenseKey)
                    {
                        _etLicenseKey.setText(scannedString);
                    }
                    else if(_scanType == ScanType.stActivationCode)
                    {
                        _etActivationCode.setText(scannedString);
                    }
                }
            }
        }
        else if(requestCode == OFFLINE_ACTIVATION_REQUEST_CODE)
        {
            String activationCode = intent.getStringExtra(OfflineActivationActivity.EXTRA_ACTIVATION_CODE);
            if(!Utils.isEmptyString(activationCode))
            {
                _etActivationCode.setText(activationCode);
            }
        }
    }

    private String machineInfo()
    {
        DisplayMetrics dm = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(dm);
        int densityDpi = (int) (dm.density * 160f);
        //double x = Math.pow(mWidthPixels / dm.xdpi, 2);
        //double y = Math.pow(mHeightPixels / dm.ydpi, 2);
        //int screenInches = Math.sqrt(x + y);
        //int rounded = df2.format(screenInches);


        StringBuilder sb = new StringBuilder();

        sb.append("\n\nSYSTEM:");
        sb.append("\nID: " + Settings.Secure.getString(getContentResolver(), Settings.Secure.ANDROID_ID));

        sb.append("\n\nDEVICE:");
        sb.append("\nMANUFACTURER: " + Build.MANUFACTURER);
        sb.append("\nID: " + Build.ID);
        sb.append("\nBRAND: " + Build.BRAND);
        sb.append("\nMODEL: " + Build.MODEL);
        sb.append("\nBOARD: " + Build.BOARD);
        sb.append("\nHARDWARE: " + Build.HARDWARE);
        sb.append("\nSERIAL: " + Build.SERIAL);
        sb.append("\nBOOTLOADER: " + Build.BOOTLOADER);
        sb.append("\nUSER: " + Build.USER);
        sb.append("\nHOST: " + Build.HOST);
        sb.append("\nBUILD.TIME: " + Build.TIME);
        sb.append("\nVERSION.RELEASE: " + Build.VERSION.RELEASE);
        sb.append("\nVERSION.SDK_INT: " + Build.VERSION.SDK_INT);

        return sb.toString();
    }

    private void scanData(String prompt, ScanType st)
    {
        _scanType = st;
        _scanning = true;

        IntentIntegrator ii = new IntentIntegrator(this);

        ii.setCaptureActivity(OrientationIndependentQrCodeScanActivity.class);
        ii.setPrompt(prompt);
        ii.setBeepEnabled(true);
        ii.setOrientationLocked(false);
        ii.setDesiredBarcodeFormats(IntentIntegrator.QR_CODE_TYPES);
        ii.setBarcodeImageEnabled(true);
        ii.setTimeout(10000);
        ii.initiateScan();
    }


    public void onClickContact(View view)
    {
        Intent intent = new Intent(this, ContactActivity.class);
        startActivity(intent);
    }

    public void onClickSystemInfo(View view)
    {
        String s;

        s = machineInfo();

        final TextView message = new TextView(this);
        final SpannableString ss = new SpannableString(s);

        message.setText(ss);
        message.setMovementMethod(LinkMovementMethod.getInstance());
        message.setPadding(32, 32, 32, 32);

        AlertDialog dlg = new AlertDialog.Builder(this)
                .setTitle("System Information")
                .setCancelable(true)
                .setNegativeButton(R.string.button_close, null)
                .setView(message).create();

        dlg.show();
    }

    public void onClickScanLicenseKey(View view)
    {
        scanData(getString(R.string.scan_license_key), ScanType.stLicenseKey);
    }

    public void onClickScanActivationCode(View view)
    {
        scanData(getString(R.string.scan_activation_code), ScanType.stActivationCode);
    }

    public void onClickGetActivationCodeOnline(View view)
    {
        attemptOnlineActivation();
    }

    public void onClickGetActivationCodeOffline(View view)
    {
        Intent intent = new Intent(this, OfflineActivationActivity.class);
        intent.putExtra(OfflineActivationActivity.EXTRA_DEVICE_ID, _activeLd._deviceId);
        intent.putExtra(OfflineActivationActivity.EXTRA_LICENSE_KEY, _etLicenseKey.getText().toString());
        startActivityForResult(intent, OFFLINE_ACTIVATION_REQUEST_CODE);
    }

    @Override
    public void onLicenseActivationTaskComplete(int result, String activationCode, String resultMessage)
    {
        _progressDialog = Utils.hideProgressMessage(_progressDialog);

        if(result == 0 && !Utils.isEmptyString(activationCode))
        {
            _etActivationCode.setText(activationCode);
        }
        else
        {
            Toast.makeText(this, resultMessage, Toast.LENGTH_LONG).show();
        }
    }

    private void attemptOnlineActivation()
    {
        String url = getString(R.string.online_licensing_activation_url);
        String key = _etLicenseKey.getText().toString();
        String ac = _etActivationCode.getText().toString();

        String entitlementKey = getString(R.string.licensing_entitlement);

        String stringToHash = key + _activeLd._deviceId + entitlementKey;
        String hValue = Utils.md5HashOfString(stringToHash);

        LicenseActivationTask lat = new LicenseActivationTask(this, url, getString(R.string.licensing_entitlement), key, ac, _activeLd._deviceId, hValue, this);

        _progressDialog = Utils.showProgressMessage(this, getString(R.string.obtaining_activation_code), _progressDialog);
        lat.execute();
    }
}
