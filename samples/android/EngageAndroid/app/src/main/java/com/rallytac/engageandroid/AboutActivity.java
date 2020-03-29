//
//  Copyright (c) 2019 Rally Tactical Systems, Inc.
//  All rights reserved.
//

package com.rallytac.engageandroid;

import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.provider.Settings;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;
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
                                LicenseActivationTask.ITaskCompletionNotification,
                                LicenseDeactivationTask.ITaskCompletionNotification

{
    private static String TAG = AboutActivity.class.getSimpleName();

    private int OFFLINE_ACTIVATION_REQUEST_CODE = 771;


    //private enum KeyType {ktUnknown, ktPerpetual, ktExpires};
    private enum ScanType {stUnknown, stLicenseKey, stActivationCode}

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
    private InternalDescriptor _activeLd = null;
    private InternalDescriptor _newLd = null;


    private int _numberOfClicksOfAppLogo = 0;

    private class InternalDescriptor
    {
        public LicenseDescriptor _ld;
        public boolean _needsSaving = false;

        InternalDescriptor()
        {
            _needsSaving = false;
        }

        InternalDescriptor(InternalDescriptor x)
        {
            this._ld = x._ld;
            this._needsSaving = x._needsSaving;
        }

        public boolean isValid()
        {
            return _ld.isValid();
        }

        public boolean equals(InternalDescriptor x)
        {
            return this._ld.equals(x);
        }
    }

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

        String key = Globals.getSharedPreferences().getString(PreferenceKeys.USER_LICENSING_KEY, "");
        String ac = Globals.getSharedPreferences().getString(PreferenceKeys.USER_LICENSING_ACTIVATION_CODE, "");

        // Get the license descriptor using the existing license key and activation code (if any)
        _activeLd = parseIntoInternalDescriptor(Globals.getEngageApplication()
                                                .getEngine()
                                                .engageGetLicenseDescriptor(getString(R.string.licensing_entitlement), key, ac, getString(R.string.manufacturer_id)));

        // At this point, the new one is the same as the active one
        _newLd = new InternalDescriptor(_activeLd);
        _newLd._needsSaving = false;

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

        String s = _activeLd._ld._deviceId;
        if(Utils.isEmptyString(s))
        {
            s = "unknown";
        }

        _etDeviceId.setText(s);
        _etLicenseKey.setText(_activeLd._ld._key);
        _etActivationCode.setText(_activeLd._ld._activationCode);

        String versionInfo;

        versionInfo = BuildConfig.VERSION_NAME + " (Engage Engine " + Globals.getEngageApplication().getEngine().engageGetVersion() + ")";

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
        super.onStop();
    }

    private void saveLicenseData()
    {
        if(_newLd.isValid() && _newLd._needsSaving)
        {
            _newLd._needsSaving = false;

            String key = Utils.emptyAs(_newLd._ld._key, "");
            String ac = Utils.emptyAs(_newLd._ld._activationCode, "");

            Log.i(TAG, "saving licensing [" + getString(R.string.licensing_entitlement) + "] [" + key + "] [" + ac + "]");

            Globals.getSharedPreferencesEditor().putString(PreferenceKeys.USER_LICENSING_KEY, key);
            Globals.getSharedPreferencesEditor().putString(PreferenceKeys.USER_LICENSING_ACTIVATION_CODE, ac);
            Globals.getSharedPreferencesEditor().apply();

            // Put the new license into effect
            Globals.getEngageApplication().getEngine().engageUpdateLicense(getString(R.string.licensing_entitlement), key, ac, getString(R.string.manufacturer_id));

            Globals.getEngageApplication().logEvent(Analytics.NEW_LICENSE_FROM_USER);
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
        rc._ld = LicenseDescriptor.fromJson(jsonData);
        return rc;
    }

    private void updateNewLdFromEnteredData()
    {
        String key = _etLicenseKey.getText().toString();
        String ac = _etActivationCode.getText().toString();

        _newLd = parseIntoInternalDescriptor(Globals.getEngageApplication()
                                                    .getEngine()
                                                    .engageGetLicenseDescriptor(getString(R.string.licensing_entitlement), key, ac, getString(R.string.manufacturer_id)));

        _newLd._needsSaving = true;
    }

    private void updateUi()
    {
        final String limitedTxMsg = "You can continue to use the application but won't be able to transmit for more than 3 seconds at a time.";

        StringBuilder sb = new StringBuilder();

        if(_activeLd.equals(_newLd))
        {
            // If active and new are the same, we'll build from the active
            if(_activeLd.isValid())
            {
                if(_activeLd._ld._theType == Engine.LicenseType.expires)
                {
                    Date dt = new Date();
                    if(_activeLd._ld._expires.after(dt))
                    {
                        sb.append("Your existing license expires " + Utils.formattedTimespan(dt.getTime(), _activeLd._ld._expires.getTime()) + ".");
                    }
                    else
                    {
                        sb.append("Your existing license expired on " + _activeLd._ld._expiresFormatted + ".  " + limitedTxMsg);
                    }
                }
                else
                {
                    if(Utils.isEmptyString(_activeLd._ld._activationCode))
                    {
                        sb.append("Your existing license never expires but requires an activation code. " + limitedTxMsg);
                    }
                    else
                    {
                        sb.append("Your license is active.");
                    }
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
                if(_newLd._ld._theType == Engine.LicenseType.expires)
                {
                    Date dt = new Date();
                    if(_newLd._ld._expires.after(dt))
                    {
                        sb.append("This license expires " + Utils.formattedTimespan(dt.getTime(), _newLd._ld._expires.getTime()) + ".");
                    }
                    else
                    {
                        sb.append("This license expired on " + _newLd._ld._expiresFormatted + ".  " + limitedTxMsg);
                    }
                }
                else
                {
                    if(Utils.isEmptyString(_newLd._ld._activationCode))
                    {
                        sb.append("This license never expires but requires an activation code. " + limitedTxMsg);
                    }
                    else
                    {
                        sb.append("Your license is active.");
                    }
                    //tryAutoActivate = true;
                }
            }
            else
            {
                sb.append("This is not a valid license.  " + limitedTxMsg);
            }
        }

        _tvLicensingMessage.setText(sb.toString());

        if(Utils.isEmptyString(_activeLd._ld._activationCode) || Utils.isEmptyString(_etActivationCode.getText().toString()))
        {
            findViewById(R.id.btnDeactivate).setVisibility(View.INVISIBLE);
        }
        else
        {
            findViewById(R.id.btnDeactivate).setVisibility(View.VISIBLE);
        }
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
        if (item.getItemId() == android.R.id.home)
        {
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
                        updateUi();
                    }
                    else if(_scanType == ScanType.stActivationCode)
                    {
                        _etActivationCode.setText(scannedString);
                        updateUi();
                    }
                }
            }
        }
        else if(requestCode == OFFLINE_ACTIVATION_REQUEST_CODE)
        {
            if(intent != null)
            {
                String activationCode = intent.getStringExtra(OfflineActivationActivity.EXTRA_ACTIVATION_CODE);
                if (!Utils.isEmptyString(activationCode))
                {
                    _etActivationCode.setText(activationCode);
                    updateUi();
                }
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

    public void onClickDeactivate(View view)
    {
        AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(this);

        alertDialogBuilder.setTitle(R.string.about_deactivate_title);

        alertDialogBuilder.setMessage(R.string.about_deactivate_really_sure);
        alertDialogBuilder.setCancelable(true);
        alertDialogBuilder.setNegativeButton("No", new DialogInterface.OnClickListener()
        {
            @Override
            public void onClick(DialogInterface dialog, int which)
            {
                dialog.cancel();
            }
        });
        alertDialogBuilder.setPositiveButton("Yes", new DialogInterface.OnClickListener()
        {
            @Override
            public void onClick(DialogInterface dialog, int which)
            {
                attemptOnlineDeactivation();
            }
        });

        AlertDialog dlg = alertDialogBuilder.create();
        dlg.show();
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
            promptForOfflineActivation();
        }
    }

    private void attemptOnlineActivation()
    {
        String url;
        if(Globals.getSharedPreferences().getBoolean(PreferenceKeys.DEVELOPER_USE_DEV_LICENSING_SYSTEM, false))
        {
            url = getString(R.string.online_licensing_activation_url_dev);
        }
        else
        {
            url = getString(R.string.online_licensing_activation_url_prod);
        }

        String key = _etLicenseKey.getText().toString();
        String ac = _etActivationCode.getText().toString();
        String entitlementKey = getString(R.string.licensing_entitlement);

        if(Utils.isEmptyString(key))
        {
            Utils.showErrorMsg(this, getString(R.string.about_invalid_license_key));
            return;
        }

        if(Utils.isEmptyString(entitlementKey))
        {
            Utils.showErrorMsg(this, getString(R.string.about_invalid_entitlement_key));
            return;
        }

        try
        {
            LicenseDescriptor testDescriptor = LicenseDescriptor.fromJson(Globals.getEngageApplication()
                    .getEngine()
                    .engageGetLicenseDescriptor(Globals.getEngageApplication().getString(R.string.licensing_entitlement),
                            key,
                            ac,
                            Globals.getEngageApplication().getString(R.string.manufacturer_id)));

            if (testDescriptor == null)
            {
                Utils.showErrorMsg(this, getString(R.string.failed_to_validate_license_info));
                throw new Exception("");
            }

            if (testDescriptor._status != Engine.LicensingStatusCode.requiresActivation)
            {
                Utils.showErrorMsg(this, getString(R.string.about_invalid_license_key));
                throw new Exception("");
            }

            String stringToHash = key + _activeLd._ld._deviceId + entitlementKey;
            String hValue = Utils.md5HashOfString(stringToHash);

            LicenseActivationTask lat = new LicenseActivationTask(this, url, getString(R.string.licensing_entitlement), key, ac, _activeLd._ld._deviceId, hValue, this);

            _progressDialog = Utils.showProgressMessage(this, getString(R.string.obtaining_activation_code), _progressDialog);
            lat.execute();
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    @Override
    public void onLicenseDeactivationTaskComplete(int result, String resultMessage)
    {
        _progressDialog = Utils.hideProgressMessage(_progressDialog);

        if(result == 0)
        {
            runOnUiThread(new Runnable()
            {
                @Override
                public void run()
                {
                    _etActivationCode.setText(null);
                    userChangedLicensedData();
                    updateUi();
                    Toast.makeText(AboutActivity.this, R.string.deactivated, Toast.LENGTH_LONG).show();
                }
            });
        }
        else
        {
            promptForOfflineDeactivation();
        }
    }

    private void attemptOnlineDeactivation()
    {
        String url;
        if(Globals.getSharedPreferences().getBoolean(PreferenceKeys.DEVELOPER_USE_DEV_LICENSING_SYSTEM, false))
        {
            url = getString(R.string.online_licensing_deactivation_url_dev);
        }
        else
        {
            url = getString(R.string.online_licensing_deactivation_url_prod);
        }

        String key = _etLicenseKey.getText().toString();

        String entitlementKey = getString(R.string.licensing_entitlement);

        String stringToHash = key + _activeLd._ld._deviceId + entitlementKey;
        String hValue = Utils.md5HashOfString(stringToHash);

        LicenseDeactivationTask ldt = new LicenseDeactivationTask(this, url, key, _activeLd._ld._deviceId, hValue, this);

        _progressDialog = Utils.showProgressMessage(this, getString(R.string.deactivating), _progressDialog);
        ldt.execute();
    }

    private void promptForOfflineDeactivation()
    {
        runOnUiThread(new Runnable()
        {
            @Override
            public void run()
            {
                AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(AboutActivity.this);

                alertDialogBuilder.setTitle(R.string.about_deactivate_title);

                alertDialogBuilder.setMessage(R.string.deactivation_failed_try_offline);
                alertDialogBuilder.setCancelable(true);
                alertDialogBuilder.setNegativeButton("No", new DialogInterface.OnClickListener()
                {
                    @Override
                    public void onClick(DialogInterface dialog, int which)
                    {
                        dialog.cancel();
                    }
                });
                alertDialogBuilder.setPositiveButton("Yes", new DialogInterface.OnClickListener()
                {
                    @Override
                    public void onClick(DialogInterface dialog, int which)
                    {
                        _etActivationCode.setText(null);
                        userChangedLicensedData();
                        updateUi();
                        Toast.makeText(AboutActivity.this, R.string.deactivated, Toast.LENGTH_LONG).show();

                        Intent intent = new Intent(AboutActivity.this, OfflineDeactivationActivity.class);
                        intent.putExtra(OfflineActivationActivity.EXTRA_DEVICE_ID, _activeLd._ld._deviceId);
                        intent.putExtra(OfflineActivationActivity.EXTRA_LICENSE_KEY, _etLicenseKey.getText().toString());
                        startActivity(intent);
                    }
                });

                AlertDialog dlg = alertDialogBuilder.create();
                dlg.show();
            }
        });
    }

    private void promptForOfflineActivation()
    {
        runOnUiThread(new Runnable()
        {
            @Override
            public void run()
            {
                AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(AboutActivity.this);

                alertDialogBuilder.setTitle(R.string.about_activate_title);

                alertDialogBuilder.setMessage(R.string.activation_failed_try_offline);
                alertDialogBuilder.setCancelable(true);
                alertDialogBuilder.setNegativeButton("No", new DialogInterface.OnClickListener()
                {
                    @Override
                    public void onClick(DialogInterface dialog, int which)
                    {
                        dialog.cancel();
                    }
                });
                alertDialogBuilder.setPositiveButton("Yes", new DialogInterface.OnClickListener()
                {
                    @Override
                    public void onClick(DialogInterface dialog, int which)
                    {
                        String key = _etLicenseKey.getText().toString();
                        if(Utils.isEmptyString(key))
                        {
                            Utils.showErrorMsg(AboutActivity.this, getString(R.string.about_invalid_license_key));
                            return;
                        }

                        Intent intent = new Intent(AboutActivity.this, OfflineActivationActivity.class);
                        intent.putExtra(OfflineActivationActivity.EXTRA_DEVICE_ID, _activeLd._ld._deviceId);
                        intent.putExtra(OfflineActivationActivity.EXTRA_LICENSE_KEY, _etLicenseKey.getText().toString());
                        startActivityForResult(intent, OFFLINE_ACTIVATION_REQUEST_CODE);
                    }
                });

                AlertDialog dlg = alertDialogBuilder.create();
                dlg.show();
            }
        });
    }
}
