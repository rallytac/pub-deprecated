//
//  Copyright (c) 2019 Rally Tactical Systems, Inc.
//  All rights reserved.
//

package com.rallytac.engagereference.core;

import android.Manifest;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.provider.Settings;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.Window;
import android.view.WindowManager;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;

import com.google.android.gms.common.ConnectionResult;
import com.google.android.gms.common.GoogleApiAvailability;

import java.util.ArrayList;
import java.util.List;
import java.util.Timer;
import java.util.TimerTask;

import static com.rallytac.engagereference.core.PowerSaverHelper.prepareIntentForWhiteListingOfBatteryOptimization;

public class LauncherActivity extends AppCompatActivity
{
    private static String TAG = LauncherActivity.class.getSimpleName();

    private static int ACTION_MANAGE_OVERLAY_PERMISSION_REQUEST_CODE= 5469;
    private static int ACTION_MANAGE_BATTERY_OPTIMIZATION_PERMISSION_REQUEST_CODE= 4242;
    private static int ACTION_MANAGE_ANDROID_PERMISSIONS_REQUEST_CODE= 4243;
    private static int ACTION_MANAGE_ANDROID_BATTERY_STANDARD_OPT_ACTIVITY_REQUEST_CODE= 4244;
    private static String TO_ENSURE_BEST_PERFORMANCE = "To ensure the best possible performance, please disable battery optimizations for this application.";

    private static final int PERM_STEP_CHECK_SCREEN_OVERLAY = 0;
    private static final int PERM_STEP_CHECK_DOZE = 1;
    private static final int PERM_STEP_CHECK_ANDROID_PERMISSIONS = 2;
    private static final int PERM_STEP_CHECK_DONE = 3;

    private long _waitStartedAt = 0;
    private Timer _waitForEngageOnlineTimer;
    private int _permissionStateMachineStep = 0;


    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);

        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_launcher);

        Animation fadeInAnimation = AnimationUtils.loadAnimation(this, R.anim.launcher_logo_fade);

        findViewById(R.id.ivAppLogo).startAnimation(fadeInAnimation);

        _permissionStateMachineStep = PERM_STEP_CHECK_DOZE;
        permissionStateTransition();
    }

    private void showIssueAndFinish(final String title, final String message)
    {
        runOnUiThread(new Runnable()
        {
            @Override
            public void run()
            {
                AlertDialog dlg = new AlertDialog.Builder(LauncherActivity.this)
                        .setTitle(title)
                        .setMessage(message)
                        .setCancelable(false)
                        .setPositiveButton(getString(R.string.button_ok), new DialogInterface.OnClickListener()
                        {
                            @Override
                            public void onClick(DialogInterface dialogInterface, int i)
                            {
                                Globals.getEngageApplication().stop();
                                LauncherActivity.this.finish();
                            }
                        }).create();

                dlg.show();
            }
        });
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent intent)
    {
        if (requestCode == ACTION_MANAGE_OVERLAY_PERMISSION_REQUEST_CODE)
        {
            permissionStateTransition();
        }
        else if (requestCode == ACTION_MANAGE_BATTERY_OPTIMIZATION_PERMISSION_REQUEST_CODE)
        {
            PowerSaverHelper.WhiteListedInBatteryOptimizations setting = PowerSaverHelper.getIfAppIsWhiteListedFromBatteryOptimizations(getApplication(), getPackageName());
            if(setting == PowerSaverHelper.WhiteListedInBatteryOptimizations.WHITE_LISTED)
            {
                permissionStateTransition();
            }
            else
            {
                showDozeRelatedIssueAndMoveOn(getString(R.string.startup_note_about_optimization_that_performance_may_vary) + getString(R.string.to_ensure_best_performance_disable_battery_optimizations));
            }
        }
        else if (requestCode == ACTION_MANAGE_ANDROID_PERMISSIONS_REQUEST_CODE)
        {
            permissionStateTransition();
        }
        else if (requestCode == ACTION_MANAGE_ANDROID_BATTERY_STANDARD_OPT_ACTIVITY_REQUEST_CODE)
        {
            permissionStateTransition();
        }
    }

    private void permissionStateTransition()
    {
        switch(_permissionStateMachineStep)
        {
            case PERM_STEP_CHECK_SCREEN_OVERLAY:
                _permissionStateMachineStep++;
                checkPopUpPermission();
                break;

            case PERM_STEP_CHECK_DOZE:
                _permissionStateMachineStep++;
                checkDozePermission();
                break;

            case PERM_STEP_CHECK_ANDROID_PERMISSIONS:
                _permissionStateMachineStep++;
                checkForPermissions();
                break;

            case PERM_STEP_CHECK_DONE:
                startEngineWhenServiceIsOnline();
                break;
        }
    }

    private void checkPopUpPermission()
    {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M)
        {
            if (!Settings.canDrawOverlays(this))
            {
                Intent intent = new Intent(Settings.ACTION_MANAGE_OVERLAY_PERMISSION, Uri.parse("package:" + getPackageName()));
                startActivityForResult(intent, ACTION_MANAGE_OVERLAY_PERMISSION_REQUEST_CODE);
            }
            else
            {
                permissionStateTransition();
            }
        }
        else
        {
            permissionStateTransition();
        }
    }

    private void showDozeRelatedIssueAndMoveOn(String msg)
    {
        final Intent intentToRequestIgnore = Utils.intentToIgnoreBatteryOptimization(Globals.getEngageApplication());
        String finalMessage = msg;

        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(R.string.title_battery_optimization_issue);
        builder.setCancelable(false);
        builder.setIcon(R.drawable.ic_error);
        builder.setPositiveButton(R.string.button_ok, new DialogInterface.OnClickListener()
        {
            @Override
            public void onClick(DialogInterface dialogInterface, int i)
            {
                permissionStateTransition();
            }
        });

        if(intentToRequestIgnore != null)
        {
            finalMessage += getString(R.string.tap_to_open_battery_optimizations);

            builder.setNeutralButton(R.string.button_battery_optimizations, new DialogInterface.OnClickListener()
            {
                @Override
                public void onClick(DialogInterface dialogInterface, int i)
                {
                    startActivityForResult(intentToRequestIgnore, ACTION_MANAGE_ANDROID_BATTERY_STANDARD_OPT_ACTIVITY_REQUEST_CODE);
                }
            });
        }

        builder.setMessage(finalMessage);


        AlertDialog dlg = builder.create();
        dlg.show();
    }

    private void checkDozePermission()
    {
        // See if we're already whitelisted
        PowerSaverHelper.WhiteListedInBatteryOptimizations setting = PowerSaverHelper.getIfAppIsWhiteListedFromBatteryOptimizations(getApplication(), getPackageName());
        if(setting == PowerSaverHelper.WhiteListedInBatteryOptimizations.WHITE_LISTED)
        {
            permissionStateTransition();
            return;
        }

        if(setting == PowerSaverHelper.WhiteListedInBatteryOptimizations.ERROR_GETTING_STATE)
        {
            showDozeRelatedIssueAndMoveOn(getString(R.string.startup_error_cannot_determine_battery_optimization_state) + getString(R.string.to_ensure_best_performance_disable_battery_optimizations));
            return;
        }
        else if(setting == PowerSaverHelper.WhiteListedInBatteryOptimizations.UNKNOWN_TOO_OLD_ANDROID_API_FOR_CHECKING)
        {
            showDozeRelatedIssueAndMoveOn(getString(R.string.startup_error_older_android_version) + getString(R.string.to_ensure_best_performance_disable_battery_optimizations));
            return;
        }
        else if(setting == PowerSaverHelper.WhiteListedInBatteryOptimizations.IRRELEVANT_OLD_ANDROID_API)
        {
            showDozeRelatedIssueAndMoveOn(getString(R.string.startup_error_older_android_version) + getString(R.string.to_ensure_best_performance_disable_battery_optimizations));
            return;
        }
        else
        {
            // Only do this if we even have the permission in the manifest
            if (Utils.isManifestPermissionPresent(Manifest.permission.REQUEST_IGNORE_BATTERY_OPTIMIZATIONS))
            {
                @SuppressLint("MissingPermission")
                Intent intent = prepareIntentForWhiteListingOfBatteryOptimization(getApplication(), getPackageName(), false);
                if (intent != null)
                {
                    startActivityForResult(intent, ACTION_MANAGE_BATTERY_OPTIMIZATION_PERMISSION_REQUEST_CODE);
                }
                else
                {
                    showDozeRelatedIssueAndMoveOn(getString(R.string.startup_error_looks_like_version_of_application_cannot_ask_for_battery_optimization) + getString(R.string.to_ensure_best_performance_disable_battery_optimizations));
                }
            }
            else
            {
                // There's nothing much we can do so just let our user know and move on
                showDozeRelatedIssueAndMoveOn(getString(R.string.startup_error_looks_like_we_cannot_ask_for_battery_optimization) + getString(R.string.to_ensure_best_performance_disable_battery_optimizations));
            }
        }
    }


    public boolean checkPlayServices(Activity activity)
    {
        // Do we even have Google Play services here?
        final int PLAY_SERVICES_RESOLUTION_REQUEST = 9000;
        GoogleApiAvailability apiAvailability = GoogleApiAvailability.getInstance();
        int resultCode = apiAvailability.isGooglePlayServicesAvailable(activity);

        if (resultCode != ConnectionResult.SUCCESS)
        {
            if (apiAvailability.isUserResolvableError(resultCode))
            {
                //apiAvailability.getErrorDialog(activity, resultCode, PLAY_SERVICES_RESOLUTION_REQUEST).show();
            }
            else
            {
                Log.e(TAG, "This device is not supported.");
                showIssueAndFinish(getString(R.string.title_google_play_services_error), getString(R.string.google_play_services_not_installed_or_out_of_date));
            }

            return false;
        }

        return true;
    }


    private void checkForPermissions()
    {
        String[] required = new String[] {
                Manifest.permission.RECORD_AUDIO,
                Manifest.permission.READ_EXTERNAL_STORAGE,
                Manifest.permission.WRITE_EXTERNAL_STORAGE,
                Manifest.permission.ACCESS_NETWORK_STATE,
                Manifest.permission.ACCESS_WIFI_STATE,
                Manifest.permission.WAKE_LOCK,
                Manifest.permission.CHANGE_WIFI_MULTICAST_STATE,
                Manifest.permission.INTERNET,
                Manifest.permission.MODIFY_AUDIO_SETTINGS,
                Manifest.permission.VIBRATE,
                Manifest.permission.ACCESS_FINE_LOCATION,
                Manifest.permission.ACCESS_COARSE_LOCATION,
                Manifest.permission.CAMERA,
                //android.Manifest.permission.REQUEST_IGNORE_BATTERY_OPTIMIZATIONS,
        };

        List<String> askFor = null;

        for(String s : required)
        {
            int permission = ActivityCompat.checkSelfPermission(this, s);
            if(permission != PackageManager.PERMISSION_GRANTED)
            {
                if(askFor == null)
                {
                    askFor = new ArrayList<>();
                }
                askFor.add(s);
            }
        }

        if(askFor != null)
        {
            String[] requested = new String[askFor.size()];
            int x = 0;

            for(String s : askFor)
            {
                requested[x] = s;
                x++;
            }

            ActivityCompat.requestPermissions(this, requested, 1);
        }
        else
        {
            permissionStateTransition();
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String permissions[], @NonNull int[] grantResults)
    {
        boolean allAllowed = true;

        for (int grantResult : grantResults)
        {
            if (grantResult != PackageManager.PERMISSION_GRANTED)
            {
                allAllowed = false;
                break;
            }
        }

        if(allAllowed)
        {
            permissionStateTransition();
        }
        else
        {
            showIssueAndFinish(getString(R.string.title_permissions_required), getString(R.string.startup_one_or_more_permissions_denied));
        }
    }

    private void startEngineWhenServiceIsOnline()
    {
        Log.i(TAG, "engaging ...");

        _waitStartedAt = Utils.nowMs();
        _waitForEngageOnlineTimer = new Timer();
        _waitForEngageOnlineTimer.scheduleAtFixedRate(new TimerTask()
        {
            @Override
            public void run()
            {
                if(Globals.getEngageApplication().isServiceOnline())
                {
                    _waitForEngageOnlineTimer.cancel();
                    Globals.getEngageApplication().startEngine();

                    String launchActivityName = Utils.getMetaData("com.rallytac.engagereference.core.LAUNCH_ACTIVITY");
                    if(!Utils.isEmptyString(launchActivityName))
                    {
                        try
                        {
                            Class<?> cls = getClassLoader().loadClass(launchActivityName);
                            startActivity(new Intent(getApplicationContext(), cls));
                            finish();
                        }
                        catch (Exception e)
                        {
                            showIssueAndFinish(getString(R.string.title_ui_launch_error), "Cannot launch the UI named : '" + launchActivityName + "'.\n\nPlease check your Android manifest.");
                        }
                    }
                    else
                    {
                        showIssueAndFinish(getString(R.string.title_ui_launch_error), "No launch UI named.\n\nPlease check your Android manifest.");
                    }
                }
                else
                {
                    if(Utils.nowMs() - _waitStartedAt > Constants.LAUNCH_TIMEOUT_MS)
                    {
                        _waitForEngageOnlineTimer.cancel();
                        showIssueAndFinish(getString(R.string.title_startup_error), getString(R.string.startup_cannot_connect_to_engine));
                    }

                    Log.i(TAG, "waiting for engage service to come online");
                }
            }
        }, 500, 500);
    }
}
