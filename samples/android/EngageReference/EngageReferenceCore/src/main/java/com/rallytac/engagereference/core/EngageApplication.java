//
//  Copyright (c) 2019 Rally Tactical Systems, Inc.
//  All rights reserved.
//

package com.rallytac.engagereference.core;

import android.app.Activity;
import android.app.Application;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.location.Location;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.BatteryManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.VibrationEffect;
import android.os.Vibrator;
import android.preference.PreferenceManager;
import android.support.v7.app.AlertDialog;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.EditText;
import android.widget.Toast;

import com.google.zxing.integration.android.IntentIntegrator;
import com.google.zxing.integration.android.IntentResult;
import com.rallytac.engage.engine.Engine;
import com.rallytac.engagereference.core.Biometrics.DataSeries;
import com.rallytac.engagereference.core.Biometrics.RandomHumanBiometricGenerator;

import org.json.JSONArray;
import org.json.JSONObject;

import java.io.ByteArrayOutputStream;
import java.net.NetworkInterface;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Timer;
import java.util.TimerTask;

public class EngageApplication
                                extends
                                    Application

                                implements
                                    Application.ActivityLifecycleCallbacks,
                                    ServiceConnection,
                                    Engine.IEngineListener,
                                    Engine.IRallypointListener,
                                    Engine.IGroupListener,
                                    Engine.ILicenseListener,
                                    LocationManager.ILocationUpdateNotifications,
                                    IPushToTalkRequestHandler,
                                    BluetoothManager.IBtNotification,
                                    LicenseActivationTask.ITaskCompletionNotification
{
    private static String TAG = EngageApplication.class.getSimpleName();

    public interface IPresenceChangeListener
    {
        void onPresenceAdded(PresenceDescriptor pd);
        void onPresenceChange(PresenceDescriptor pd);
        void onPresenceRemoved(PresenceDescriptor pd);
    }

    public interface IUiUpdateListener
    {
        void onAnyTxPending();
        void onAnyTxActive();
        void onAnyTxEnding();
        void onAllTxEnded();
        void onGroupUiRefreshNeeded(GroupDescriptor gd);
        void onGroupTxUsurped(GroupDescriptor gd);
        void onGroupMaxTxTimeExceeded(GroupDescriptor gd);
    }

    public interface IAssetChangeListener
    {
        void onAssetDiscovered(String id, String json);
        void onAssetRediscovered(String id, String json);
        void onAssetUndiscovered(String id, String json);
    }

    public interface IConfigurationChangeListener
    {
        void onMissionChanged();
        void onCriticalConfigurationChange();
    }

    public interface ILicenseChangeListener
    {
        void onLicenseChanged();
        void onLicenseExpired();
        void onLicenseExpiring(long secondsLeft);
    }

    public interface IGroupTimelineListener
    {
        void onGroupTimelineEventStarted(GroupDescriptor gd, String eventJson);
        void onGroupTimelineEventUpdated(GroupDescriptor gd, String eventJson);
        void onGroupTimelineEventEnded(GroupDescriptor gd, String eventJson);
        void onGroupTimelineReport(GroupDescriptor gd, String reportJson);
        void onGroupTimelineReportFailed(GroupDescriptor gd);
    }

    private EngageService _svc = null;
    private boolean _engineRunning = false;
    private ActiveConfiguration _activeConfiguration = null;
    private boolean _missionChangedStatus = false;
    private LocationManager _locationManager = null;

    private HashSet<IPresenceChangeListener> _presenceChangeListeners = new HashSet<>();
    private HashSet<IUiUpdateListener> _uiUpdateListeners = new HashSet<>();
    private HashSet<IAssetChangeListener> _assetChangeListeners = new HashSet<>();
    private HashSet<IConfigurationChangeListener> _configurationChangeListeners = new HashSet<>();
    private HashSet<ILicenseChangeListener> _licenseChangeListeners = new HashSet<>();
    private HashSet<IGroupTimelineListener> _groupTimelineListeners = new HashSet<>();

    private long _lastAudioActivity = 0;
    private long _lastTxActivity = 0;
    private Timer _groupHealthCheckTimer = null;
    private long _lastNetworkErrorNotificationPlayed = 0;
    private HashMap<String, GroupDescriptor> _dynamicGroups = new HashMap<>();
    private HardwareButtonManager _hardwareButtonManager = null;
    private boolean _licenseExpired = false;
    private long _licenseSecondsLeft = 0;
    private Timer _licenseActivationTimer = null;
    private boolean _licenseActivationPaused = false;

    private Timer _humanBiometricsReportingTimer = null;
    private int _hbmTicksSoFar = 0;
    private int _hbmTicksBeforeReport = 5;

    private DataSeries _hbmHeartRate = null;
    private DataSeries _hbmSkinTemp = null;
    private DataSeries _hbmCoreTemp = null;
    private DataSeries _hbmHydration = null;
    private DataSeries _hbmBloodOxygenation = null;
    private DataSeries _hbmFatigueLevel = null;
    private DataSeries _hbmTaskEffectiveness = null;

    private RandomHumanBiometricGenerator _rhbmgHeart = null;
    private RandomHumanBiometricGenerator _rhbmgSkinTemp = null;
    private RandomHumanBiometricGenerator _rhbmgCoreTemp = null;
    private RandomHumanBiometricGenerator _rhbmgHydration = null;
    private RandomHumanBiometricGenerator _rhbmgOxygenation = null;
    private RandomHumanBiometricGenerator _rhbmgFatigueLevel = null;
    private RandomHumanBiometricGenerator _rhbmgTaskEffectiveness = null;

    private JSONObject _cachedPdLocation = null;
    private JSONObject _cachedPdConnectivityInfo = null;
    private JSONObject _cachedPdPowerInfo = null;

    private MyDeviceMonitor _deviceMonitor = null;
    private boolean _enableDevicePowerMonitor = false;
    private boolean _enableDeviceConnectivityMonitor = false;

    private class MyDeviceMonitor extends BroadcastReceiver
    {
        private final int NUM_SIGNAL_LEVELS = 5;

        public MyDeviceMonitor()
        {
        }

        public void start()
        {
            IntentFilter intentFilter = new IntentFilter();
            intentFilter.addAction(WifiManager.RSSI_CHANGED_ACTION);
            intentFilter.addAction(ConnectivityManager.CONNECTIVITY_ACTION);

            intentFilter.addAction(Intent.ACTION_BATTERY_CHANGED);
            intentFilter.addAction("android.intent.action.BATTERY_LEVEL_CHANGED");
            intentFilter.addAction(Intent.ACTION_BATTERY_LOW);
            intentFilter.addAction(Intent.ACTION_BATTERY_OKAY);
            intentFilter.addAction(Intent.ACTION_POWER_CONNECTED);
            intentFilter.addAction(Intent.ACTION_POWER_DISCONNECTED);

            registerReceiver(this, intentFilter);
        }

        public void stop()
        {
            unregisterReceiver(this);
        }

        @Override
        public void onReceive(Context context, Intent intent)
        {
            Log.i(TAG, "{DBG}: " + intent.toString());

            String action = intent.getAction();
            if(action == null)
            {
                return;
            }

            Log.i(TAG, "{DBG}: action=" + action);

            if(action.compareTo(WifiManager.RSSI_CHANGED_ACTION) == 0)
            {
                try
                {
                    Bundle bundle = intent.getExtras();
                    if(bundle != null)
                    {
                        int newRssiDbm = bundle.getInt(WifiManager.EXTRA_NEW_RSSI);
                        int level = WifiManager.calculateSignalLevel(newRssiDbm, NUM_SIGNAL_LEVELS);

                        onConnectivityChange(true, ConnectivityType.wirelessWifi, newRssiDbm, level);
                    }
                }
                catch(Exception e)
                {
                    e.printStackTrace();
                }
            }
            else if(action.compareTo(ConnectivityManager.CONNECTIVITY_ACTION) == 0)
            {
                try
                {
                    NetworkInfo info1 = intent.getParcelableExtra(ConnectivityManager.EXTRA_NETWORK_INFO);
                    NetworkInfo info2 = intent.getParcelableExtra(ConnectivityManager.EXTRA_OTHER_NETWORK_INFO);

                    Log.i(TAG, "{DBG}: info1=" + info1);

                    if (info1 != null)
                    {
                        if (info1.isConnected())
                        {
                            if (info1.getType() == ConnectivityManager.TYPE_MOBILE)
                            {
                                // TODO: RSSI for cellular
                                onConnectivityChange(true, ConnectivityType.wirelessCellular, 0, NUM_SIGNAL_LEVELS);
                            }
                            else if (info1.getType() == ConnectivityManager.TYPE_WIFI)
                            {
                                WifiManager wifiManager = (WifiManager) getApplicationContext().getSystemService(Context.WIFI_SERVICE);
                                WifiInfo wifiInfo = wifiManager.getConnectionInfo();
                                int level = WifiManager.calculateSignalLevel(wifiInfo.getRssi(), NUM_SIGNAL_LEVELS);

                                onConnectivityChange(true, ConnectivityType.wirelessWifi, wifiInfo.getRssi(), level);
                            }
                            else if (info1.getType() == ConnectivityManager.TYPE_ETHERNET)
                            {
                                onConnectivityChange(true, ConnectivityType.wired, 0, NUM_SIGNAL_LEVELS);
                            }
                        }
                    }
                }
                catch (Exception e)
                {
                    e.printStackTrace();
                }
            }
            else if(action.compareTo(Intent.ACTION_BATTERY_CHANGED) == 0)
            {
                try
                {
                    int level = intent.getIntExtra(BatteryManager.EXTRA_LEVEL,0);
                    int scale = intent.getIntExtra(BatteryManager.EXTRA_SCALE,0);
                    int levelPercent = (int)(((float)level / scale) * 100);

                    PowerSourceType pst;
                    int pluggedIn = intent.getIntExtra(BatteryManager.EXTRA_PLUGGED, 0);
                    if(pluggedIn == 0)
                    {
                        pst = PowerSourceType.battery;
                    }
                    else
                    {
                        pst = PowerSourceType.wired;
                    }

                    PowerSourceState pss;
                    int chargingExtra = intent.getIntExtra(BatteryManager.EXTRA_STATUS, BatteryManager.BATTERY_STATUS_UNKNOWN);
                    switch(chargingExtra)
                    {
                        case BatteryManager.BATTERY_STATUS_UNKNOWN:
                            pss = PowerSourceState.unknown;
                            break;

                        case BatteryManager.BATTERY_STATUS_CHARGING:
                            pss = PowerSourceState.charging;
                            break;

                        case BatteryManager.BATTERY_STATUS_DISCHARGING:
                            pss = PowerSourceState.discharging;
                            break;

                        case BatteryManager.BATTERY_STATUS_NOT_CHARGING:
                            pss = PowerSourceState.notCharging;
                            break;

                        case BatteryManager.BATTERY_STATUS_FULL:
                            pss = PowerSourceState.full;
                            break;

                        default:
                            pss = PowerSourceState.unknown;
                            break;
                    }

                    onPowerChange(pst, pss, levelPercent);
                }
                catch (Exception e)
                {
                    e.printStackTrace();
                }
            }
            else
            {
                Log.w(TAG, "{DBG}: unhandled action '" + action + "'");
            }
        }
    }

    @Override
    public void onCreate()
    {
        Log.d(TAG, "onCreate");

        super.onCreate();

        // Its important to set this as soon as possible!
        com.rallytac.engage.engine.Engine.setApplicationContext(this.getApplicationContext());

        // Note: This is for developer gtesting only!!
        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP)
        {
            try
            {
                //Os.setenv("ENGAGE_OVERRIDE_DEVICE_ID", "$RTS$DELETEME01", true);
                //Os.setenv("ENGAGE_LICENSE_CHECK_INTERVAL_MS", "5000", true);

            }
            catch (Exception e)
            {
                Log.e(TAG, "cannot set 'ENGAGE_OVERRIDE_DEVICE_ID' environment variable");
            }
        }

        Globals.setEngageApplication(this);
        Globals.setContext(getApplicationContext());
        Globals.setSharedPreferences(PreferenceManager.getDefaultSharedPreferences(this));
        Globals.setAudioPlayerManager(new AudioPlayerManager(this));

        runPreflightCheck();

        registerActivityLifecycleCallbacks(this);

        startService(new Intent(this, EngageService.class));

        int bindingFlags = (Context.BIND_AUTO_CREATE | Context.BIND_IMPORTANT);
        Intent intent= new Intent(this, EngageService.class);
        bindService(intent, this, bindingFlags);
    }

    @Override
    public void onTerminate()
    {
        Log.d(TAG, "onTerminate");
        stop();

        super.onTerminate();
    }

    @Override
    public void onActivityCreated(Activity activity, Bundle bundle)
    {
        Log.d(TAG, "onActivityCreated: " + activity.toString());
    }

    @Override
    public void onActivityStarted(Activity activity)
    {
        Log.d(TAG, "onActivityStarted: " + activity.toString());
    }

    @Override
    public void onActivityResumed(Activity activity)
    {
        Log.d(TAG, "onActivityResumed: " + activity.toString());
    }

    @Override
    public void onActivityPaused(Activity activity)
    {
        Log.d(TAG, "onActivityPaused: " + activity.toString());
    }

    @Override
    public void onActivityStopped(Activity activity)
    {
        Log.d(TAG, "onActivityStopped: " + activity.toString());
    }

    @Override
    public void onActivitySaveInstanceState(Activity activity, Bundle bundle)
    {
        Log.d(TAG, "onActivitySaveInstanceState: " + activity.toString());
    }

    @Override
    public void onActivityDestroyed(Activity activity)
    {
        Log.d(TAG, "onActivityDestroyed: " + activity.toString());
    }

    @Override
    public void onServiceConnected(ComponentName name, IBinder binder)
    {
        Log.d(TAG, "onServiceConnected: " + name.toString() + ", " + binder.toString());
        _svc = ((EngageService.EngageServiceBinder)binder).getService();

        getEngine().addEngineListener(this);
        getEngine().addRallypointListener(this);
        getEngine().addGroupListener(this);
        getEngine().addLicenseListener(this);

        startDeviceMonitor();
    }

    @Override
    public void onServiceDisconnected(ComponentName name)
    {
        Log.d(TAG, "onServiceDisconnected: " + name.toString());
        cleanupServiceConnection();
        _svc = null;
    }

    @Override
    public void onBindingDied(ComponentName name)
    {
        Log.d(TAG, "onBindingDied: " + name.toString());
        cleanupServiceConnection();
        _svc = null;
    }

    @Override
    public void onNullBinding(ComponentName name)
    {
        Log.d(TAG, "onNullBinding: " + name.toString());
        cleanupServiceConnection();
        _svc = null;
    }

    private void updateCachedPdLocation(Location location)
    {
        try
        {
            if(location != null)
            {
                JSONObject obj = new JSONObject();

                obj.put(Engine.JsonFields.Location.longitude, location.getLongitude());
                obj.put(Engine.JsonFields.Location.latitude, location.getLatitude());
                if(location.hasAltitude())
                {
                    obj.put(Engine.JsonFields.Location.altitude, location.getAltitude());
                }
                if(location.hasBearing())
                {
                    obj.put(Engine.JsonFields.Location.direction, location.getBearing());
                }
                if(location.hasSpeed())
                {
                    obj.put(Engine.JsonFields.Location.speed, location.getSpeed());
                }

                _cachedPdLocation = obj;
            }
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    private void updateCachedPdConnectivityInfo(ConnectivityType type, int rssi, int qualityRating)
    {
        if(_enableDeviceConnectivityMonitor)
        {
            try
            {
                JSONObject obj = new JSONObject();
                obj.put(Engine.JsonFields.Connectivity.type, type.ordinal());
                obj.put(Engine.JsonFields.Connectivity.strength, rssi);
                obj.put(Engine.JsonFields.Connectivity.rating, qualityRating);
                _cachedPdConnectivityInfo = obj;
            }
            catch (Exception e)
            {
                e.printStackTrace();
            }
        }
        else
        {
            _cachedPdConnectivityInfo = null;
        }
    }

    private void updateCachedPdPowerInfo(PowerSourceType source, PowerSourceState state, int level)
    {
        if(_enableDevicePowerMonitor)
        {
            try
            {
                JSONObject obj = new JSONObject();
                obj.put(Engine.JsonFields.Power.source, source.ordinal());
                obj.put(Engine.JsonFields.Power.state, state.ordinal());
                obj.put(Engine.JsonFields.Power.level, level);
                _cachedPdPowerInfo = obj;
            }
            catch (Exception e)
            {
                e.printStackTrace();
            }
        }
        else
        {
            _cachedPdPowerInfo = null;
        }
    }

    public enum ConnectivityType {unknown, wired, wirelessWifi, wirelessCellular}
    public enum PowerSourceType {unknown, battery, wired}
    public enum PowerSourceState {unknown, charging, discharging, notCharging, full}

    public void onConnectivityChange(boolean connected, ConnectivityType type, int rssi, int qualityRating)
    {
        updateCachedPdConnectivityInfo(type, rssi, qualityRating);
        sendUpdatedPd(buildPd());
    }

    public void onPowerChange(PowerSourceType source, PowerSourceState state, int level)
    {
        updateCachedPdPowerInfo(source, state, level);
        sendUpdatedPd(buildPd());
    }

    private JSONObject buildPd()
    {
        JSONObject pd = null;

        try
        {
            if(getActiveConfiguration() != null)
            {
                pd = new JSONObject();

                pd.put(Engine.JsonFields.Identity.objectName, getActiveConfiguration().makeIdentityObject());
                if(_cachedPdLocation != null)
                {
                    pd.put(Engine.JsonFields.Location.objectName, _cachedPdLocation);
                }

                if(_cachedPdPowerInfo != null)
                {
                    pd.put(Engine.JsonFields.Power.objectName, _cachedPdPowerInfo);
                }

                if(_cachedPdConnectivityInfo != null)
                {
                    pd.put(Engine.JsonFields.Connectivity.objectName, _cachedPdConnectivityInfo);
                }
            }
        }
        catch(Exception e)
        {
            e.printStackTrace();
            pd = null;
        }

        return pd;
    }

    public void restartDeviceMonitoring()
    {
        startDeviceMonitor();
        stopDeviceMonitor();
    }

    private void startDeviceMonitor()
    {
        if(_deviceMonitor == null)
        {
            _enableDevicePowerMonitor = Globals.getSharedPreferences().getBoolean(PreferenceKeys.USER_EXPERIMENT_ENABLE_DEVICE_REPORT_POWER, false);
            _enableDeviceConnectivityMonitor = Globals.getSharedPreferences().getBoolean(PreferenceKeys.USER_EXPERIMENT_ENABLE_DEVICE_REPORT_CONNECTIVITY, false);

            if(_enableDevicePowerMonitor || _enableDeviceConnectivityMonitor)
            {
                _deviceMonitor = new MyDeviceMonitor();
                _deviceMonitor.start();
            }
        }
    }

    private void stopDeviceMonitor()
    {
        if(_deviceMonitor != null)
        {
            _deviceMonitor.stop();
            _deviceMonitor = null;
        }
    }

    private void cleanupServiceConnection()
    {
        Log.d(TAG, "cleanupServiceConnection");

        stopDeviceMonitor();

        if(getEngine() != null)
        {
            getEngine().removeEngineListener(this);
            getEngine().removeRallypointListener(this);
            getEngine().removeGroupListener(this);
        }
    }

    public void stop()
    {
        Log.d(TAG, "stop");

        stopDeviceMonitor();

        if(getEngine() != null)
        {
            getEngine().engageStop();
            getEngine().engageShutdown();
        }

        unbindService(this);
        stopService(new Intent(this, EngageService.class));

        unregisterActivityLifecycleCallbacks(this);

        goIdle();

        try
        {
            Thread.sleep(500);
        }
        catch (Exception e)
        {
        }
    }

    public void terminateApplicationAndReturnToAndroid(Activity callingActivity)
    {
        stop();

        callingActivity.moveTaskToBack(true);
        callingActivity.finishAndRemoveTask();

        android.os.Process.killProcess(android.os.Process.myPid());
        System.exit(0);
    }

    public void addPresenceChangeListener(IPresenceChangeListener listener)
    {
        synchronized (_presenceChangeListeners)
        {
            _presenceChangeListeners.add(listener);
        }
    }

    public void removePresenceChangeListener(IPresenceChangeListener listener)
    {
        synchronized (_presenceChangeListeners)
        {
            _presenceChangeListeners.remove(listener);
        }
    }

    public void addUiUpdateListener(IUiUpdateListener listener)
    {
        synchronized (_uiUpdateListeners)
        {
            _uiUpdateListeners.add(listener);
        }
    }

    public void removeUiUpdateListener(IUiUpdateListener listener)
    {
        synchronized (_uiUpdateListeners)
        {
            _uiUpdateListeners.remove(listener);
        }
    }

    public void addAssetChangeListener(IAssetChangeListener listener)
    {
        synchronized (_assetChangeListeners)
        {
            _assetChangeListeners.add(listener);
        }
    }

    public void removeAssetChangeListener(IAssetChangeListener listener)
    {
        synchronized (_assetChangeListeners)
        {
            _assetChangeListeners.remove(listener);
        }
    }

    public void addConfigurationChangeListener(IConfigurationChangeListener listener)
    {
        synchronized (_configurationChangeListeners)
        {
            _configurationChangeListeners.add(listener);
        }
    }

    public void removeConfigurationChangeListener(IConfigurationChangeListener listener)
    {
        synchronized (_configurationChangeListeners)
        {
            _configurationChangeListeners.remove(listener);
        }
    }

    public void addLicenseChangeListener(ILicenseChangeListener listener)
    {
        synchronized (_licenseChangeListeners)
        {
            _licenseChangeListeners.add(listener);
        }
    }

    public void removeLicenseChangeListener(ILicenseChangeListener listener)
    {
        synchronized (_licenseChangeListeners)
        {
            _licenseChangeListeners.remove(listener);
        }
    }

    public void addGroupTimelineListener(IGroupTimelineListener listener)
    {
        synchronized (_groupTimelineListeners)
        {
            _groupTimelineListeners.add(listener);
        }
    }

    public void removeGroupTimelineListener(IGroupTimelineListener listener)
    {
        synchronized (_groupTimelineListeners)
        {
            _groupTimelineListeners.remove(listener);
        }
    }

    public void startHardwareButtonManager()
    {
        Log.d(TAG, "startHardwareButtonManager");
        _hardwareButtonManager = new HardwareButtonManager(this, this, this);
        _hardwareButtonManager.start();
    }

    public void stopHardwareButtonManager()
    {
        if(_hardwareButtonManager != null)
        {
            _hardwareButtonManager.stop();
            _hardwareButtonManager = null;
        }
    }

    public void startLocationUpdates()
    {
        Log.d(TAG, "startLocationUpdates");
        stopLocationUpdates();

        ActiveConfiguration.LocationConfiguration lc = getActiveConfiguration().getLocationConfiguration();
        if(lc != null && lc.enabled)
        {
            _locationManager = new LocationManager(this,
                    this,
                    lc.accuracy,
                    lc.intervalMs,
                    lc.minIntervalMs,
                    lc.minDisplacement);

            _locationManager.start();
        }
    }

    public void stopLocationUpdates()
    {
        Log.d(TAG, "stopLocationUpdates");
        if(_locationManager != null)
        {
            _locationManager.stop();
            _locationManager = null;
        }
    }

    public void setMissionChangedStatus(boolean s)
    {
        Log.d(TAG, "setMissionChangedStatus: " + s);
        _missionChangedStatus = s;
    }

    public boolean getMissionChangedStatus()
    {
        return _missionChangedStatus;
    }

    private void sendUpdatedPd(JSONObject pd)
    {
        if(pd == null)
        {
            Log.w(TAG, "sendUpdatedPd with null PD");
            return;
        }

        try
        {
            if(getActiveConfiguration() != null)
            {
                Log.d(TAG, "sendUpdatedPd pd=" + pd.toString());

                if(getActiveConfiguration().getMissionGroups() != null)
                {
                    boolean anyPresenceGroups = false;
                    String pdString = pd.toString();

                    for(GroupDescriptor gd : getActiveConfiguration().getMissionGroups())
                    {
                        if(gd.type == GroupDescriptor.Type.gtPresence)
                        {
                            anyPresenceGroups = true;
                            getEngine().engageUpdatePresenceDescriptor(gd.id, pdString, 1);
                        }
                    }

                    if(!anyPresenceGroups)
                    {
                        Log.w(TAG, "sendUpdatedPd but no presence groups");
                    }
                    else
                    {
                        Log.i(TAG, "sendUpdatedPd sent updated PD: " + pdString);
                    }
                }
                else
                {
                    Log.w(TAG, "sendUpdatedPd but no mission groups");
                }
            }
            else
            {
                Log.w(TAG, "sendUpdatedPd but no active configuration");
            }
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }


    @Override
    public void onLocationUpdated(Location loc)
    {
        Log.d(TAG, "onLocationUpdated: " + loc.toString());
        updateCachedPdLocation(loc);
        sendUpdatedPd(buildPd());
    }

    public VolumeLevels loadVolumeLevels(String groupId)
    {
        VolumeLevels vl = new VolumeLevels();

        vl.left = Globals.getSharedPreferences().getInt(PreferenceKeys.VOLUME_LEFT_FOR_GROUP_BASE_NAME + groupId, 100);
        if(vl.left < 0)
        {
            vl.left = 0;
        }

        vl.right = Globals.getSharedPreferences().getInt(PreferenceKeys.VOLUME_RIGHT_FOR_GROUP_BASE_NAME + groupId, 100);
        if(vl.right < 0)
        {
            vl.right = 0;
        }

        return vl;
    }

    public void saveVolumeLevels(String groupId, VolumeLevels vl)
    {
        Globals.getSharedPreferencesEditor().putInt(PreferenceKeys.VOLUME_LEFT_FOR_GROUP_BASE_NAME + groupId, vl.left);
        Globals.getSharedPreferencesEditor().putInt(PreferenceKeys.VOLUME_RIGHT_FOR_GROUP_BASE_NAME + groupId, vl.right);
        Globals.getSharedPreferencesEditor().apply();
    }

    private GroupDescriptor getGroup(String id)
    {
        if(getActiveConfiguration().getMissionGroups() != null)
        {
            for(GroupDescriptor gd : getActiveConfiguration().getMissionGroups())
            {
                if(gd.id.compareTo(id) == 0)
                {
                    return gd;
                }
            }
        }

        return null;
    }

    private String buildAdvancedTxJson(int flags, int priority, int subchannelTag, boolean includeNodeId, String alias)
    {
        String rc;

        try
        {
            JSONObject obj = new JSONObject();

            obj.put("flags", flags);
            obj.put("priority", priority);
            obj.put("subchannelTag", subchannelTag);
            obj.put("includeNodeId", includeNodeId);

            if(!Utils.isEmptyString(alias))
            {
                obj.put("alias", alias);
            }

            rc = obj.toString();

            //Log.e(TAG, rc);
        }
        catch (Exception e)
        {
            e.printStackTrace();
            rc = null;
        }

        return rc;
    }

    private String buildFinalGroupJsonConfiguration(String groupJson)
    {
        String rc;

        try
        {
            JSONObject group = new JSONObject(groupJson);

            if(!Utils.isEmptyString(_activeConfiguration.getNetworkInterfaceName()))
            {
                group.put("interfaceName", _activeConfiguration.getNetworkInterfaceName());
            }

            if(!Utils.isEmptyString(_activeConfiguration.getUserAlias()))
            {
                group.put("alias", _activeConfiguration.getUserAlias());
            }

            if(group.optInt(Engine.JsonFields.Group.type, 0) == 1)
            {
                JSONObject audio = new JSONObject();

                //audio.put("inputId", 1);
                //audio.put("outputId", 4);
                audio.put("outputGain", (_activeConfiguration.getSpeakerOutputBoostFactor() * 100));

                group.put("audio", audio);
            }

            if(_activeConfiguration.getUseRp())
            {
                JSONObject rallypoint = new JSONObject();

                JSONObject host = new JSONObject();
                host.put("address", _activeConfiguration.getRpAddress());
                host.put("port", _activeConfiguration.getRpPort());

                rallypoint.put("host", host);
                rallypoint.put("certificate", Utils.getStringResource(this, R.raw.android_rts_factory_default_engage_certificate));
                rallypoint.put("certificateKey", Utils.getStringResource(this, R.raw.android_rts_factory_default_engage_private_key));


                JSONArray rallypoints = new JSONArray();
                rallypoints.put(rallypoint);

                group.put("rallypoints", rallypoints);
            }

            rc = group.toString();
        }
        catch (Exception e)
        {
            e.printStackTrace();
            rc = null;
        }

        return rc;
    }

    public void createAllGroupObjects()
    {
        Log.d(TAG, "createAllGroupObjects");
        try
        {
            for(GroupDescriptor gd : _activeConfiguration.getMissionGroups())
            {
                getEngine().engageCreateGroup(buildFinalGroupJsonConfiguration(gd.jsonConfiguration));
                if(gd.type == GroupDescriptor.Type.gtAudio)
                {
                    VolumeLevels vl = loadVolumeLevels(gd.id);
                    getEngine().engageSetGroupRxVolume(gd.id, vl.left, vl.right);
                }
            }
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    public void joinSelectedGroups()
    {
        Log.d(TAG, "joinSelectedGroups");

        // TODO: leaving everything before joining is terrible - needs to be optimized!
        leaveAllGroups();

        try
        {
            for(GroupDescriptor gd : _activeConfiguration.getMissionGroups())
            {
                if(gd.type == GroupDescriptor.Type.gtPresence)
                {
                    // All presence groups get joined
                    getEngine().engageJoinGroup(gd.id);
                }
                if(gd.type == GroupDescriptor.Type.gtRaw)
                {
                    // All raw groups get joined
                    getEngine().engageJoinGroup(gd.id);
                }
                else if(gd.type == GroupDescriptor.Type.gtAudio)
                {
                    // Maybe just the one that's the single view
                    if(_activeConfiguration.getUiMode() == Constants.UiMode.vSingle)
                    {
                        if(gd.selectedForSingleView)
                        {
                            getEngine().engageJoinGroup(gd.id);
                        }
                    }
                    // ... or the multi-view group(s)?
                    else if(_activeConfiguration.getUiMode() == Constants.UiMode.vMulti)
                    {
                        if(gd.selectedForMultiView)
                        {
                            getEngine().engageJoinGroup(gd.id);
                        }
                    }
                }
            }

            stopGroupHealthCheckTimer();
            startGroupHealthCheckerTimer();
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    public void leaveAllGroups()
    {
        Log.d(TAG, "leaveAllGroups");
        try
        {
            stopGroupHealthCheckTimer();
            for(GroupDescriptor gd : _activeConfiguration.getMissionGroups())
            {
                getEngine().engageLeaveGroup(gd.id);
            }
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    private void startGroupHealthCheckerTimer()
    {
        Log.d(TAG, "startGroupHealthCheckerTimer");
        if(_groupHealthCheckTimer == null)
        {
            _groupHealthCheckTimer = new Timer();
            _groupHealthCheckTimer.scheduleAtFixedRate(new TimerTask()
            {
                @Override
                public void run()
                {
                    checkOnGroupHealth();
                }
            }, Constants.GROUP_HEALTH_CHECK_TIMER_INITIAL_DELAY_MS, Constants.GROUP_HEALTH_CHECK_TIMER_INTERVAL_MS);
        }
    }

    private void stopGroupHealthCheckTimer()
    {
        Log.d(TAG, "stopGroupHealthCheckTimer");
        if(_groupHealthCheckTimer != null)
        {
            _groupHealthCheckTimer.cancel();
            _groupHealthCheckTimer = null;
        }
    }

    private void checkOnGroupHealth()
    {
        if(_activeConfiguration.getNotifyOnNetworkError())
        {
            for (GroupDescriptor gd : _activeConfiguration.getMissionGroups())
            {
                if (gd.created && gd.joined && !gd.connected)
                {
                    long now = Utils.nowMs();
                    if(now - _lastNetworkErrorNotificationPlayed >= Constants.GROUP_HEALTH_CHECK_NETWORK_ERROR_NOTIFICATION_MIN_INTERVAL_MS)
                    {
                        playNetworkDownNotification();
                        _lastNetworkErrorNotificationPlayed = now;
                    }

                    break;
                }
            }
        }
    }

    public void vibrate()
    {
        if(_activeConfiguration.getEnableVibrations())
        {
            Vibrator vibe = (Vibrator) getSystemService(Context.VIBRATOR_SERVICE);

            if (vibe != null && vibe.hasVibrator())
            {
                if (Build.VERSION.SDK_INT >= 26)
                {
                    vibe.vibrate(VibrationEffect.createOneShot(100, VibrationEffect.DEFAULT_AMPLITUDE));
                }
                else
                {
                    vibe.vibrate(100);
                }
            }
        }
    }

    public void playAssetDiscoveredNotification()
    {
        Log.d(TAG, "playAssetDiscoveredOnNotification");

        vibrate();

        float volume = _activeConfiguration.getPttToneNotificationLevel();
        if(volume == 0.0)
        {
            return;
        }

        try
        {
            Globals.getAudioPlayerManager().playNotification(R.raw.asset_discovered, volume, null);
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    public void playAssetUndiscoveredNotification()
    {
        Log.d(TAG, "playAssetUndiscoveredNotification");

        vibrate();

        float volume = _activeConfiguration.getPttToneNotificationLevel();
        if(volume == 0.0)
        {
            return;
        }

        try
        {
            Globals.getAudioPlayerManager().playNotification(R.raw.asset_undiscovered, volume, null);
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    public void playNetworkDownNotification()
    {
        Log.d(TAG, "playNetworkDownNotification");

        vibrate();

        float volume = _activeConfiguration.getErrorToneNotificationLevel();
        if(volume == 0.0)
        {
            return;
        }

        try
        {
            Globals.getAudioPlayerManager().playNotification(R.raw.network_down, volume, null);
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    public void playGeneralErrorNotification()
    {
        Log.d(TAG, "playGeneralErrorNotification");

        vibrate();

        float volume = _activeConfiguration.getErrorToneNotificationLevel();
        if(volume == 0.0)
        {
            return;
        }

        try
        {
            Globals.getAudioPlayerManager().playNotification(R.raw.general_error, volume, null);
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }


    public boolean playTxOnNotification(Runnable onPlayComplete)
    {
        Log.d(TAG, "playTxOnNotification");

        vibrate();

        boolean rc = false;
        float volume = _activeConfiguration.getPttToneNotificationLevel();
        if(volume == 0.0)
        {
            return rc;
        }

        try
        {
            Globals.getAudioPlayerManager().playNotification(R.raw.tx_on, volume, onPlayComplete);
            rc = true;
        }
        catch (Exception e)
        {
            e.printStackTrace();
            rc = false;
        }

        return rc;
    }

    // TODO:
    public void playTxOffNotification()
    {
        Log.d(TAG, "playTxOffNotification");
        /*
        float volume = _activeConfiguration.getPttToneNotificationLevel();
        if(volume == 0.0)
        {
            return;
        }

        try
        {
            Globals.getAudioPlayerManager().playNotification(R.raw.tx_on, volume);
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
        */
    }

    public void restartEngine()
    {
        Log.d(TAG, "restartEngine");
        stopEngine();
        startEngine();
    }

    public void startEngine()
    {
        Log.d(TAG, "startEngine");
        try
        {
            updateActiveConfiguration();
            setMissionChangedStatus(false);

            String enginePolicyJson = getActiveConfiguration().makeEnginePolicyObject(Utils.getStringResource(this, R.raw.sample_engine_policy)).toString();
            String identityJson = getActiveConfiguration().makeIdentityObject().toString();
            String tempDirectory = Environment.getExternalStorageDirectory().getAbsolutePath();

            getEngine().engageInitialize(enginePolicyJson,
                    identityJson,
                    tempDirectory);

            getEngine().engageStart();
        }
        catch (Exception e)
        {
            e.printStackTrace();
            stop();
        }
    }

    public void stopEngine()
    {
        Log.d(TAG, "stopEngine");
        try
        {
            leaveAllGroups();
            stopLocationUpdates();
            getEngine().engageStop();
            getEngine().engageShutdown();
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    public ActiveConfiguration updateActiveConfiguration()
    {
        Log.d(TAG, "updateActiveConfiguration");
        _activeConfiguration = Utils.loadConfiguration(_dynamicGroups);

        if(!_activeConfiguration.getDiscoverSsdpAssets()
                && !_activeConfiguration.getDiscoverCistechGv1Assets()
                && !_activeConfiguration.getDiscoverTrelliswareAssets())
        {
            if(!_dynamicGroups.isEmpty())
            {
                _dynamicGroups.clear();
                _activeConfiguration = Utils.loadConfiguration(null);
            }
        }

        restartStartHumanBiometricsReporting();
        restartDeviceMonitoring();

        return _activeConfiguration;
    }

    public ActiveConfiguration getActiveConfiguration()
    {
        return _activeConfiguration;
    }

    private void runPreflightCheck()
    {
        Log.d(TAG, "runPreflightCheck");

        {
            String mnf = Build.MANUFACTURER;
            String brand = Build.BRAND;
            String model = Build.MODEL;

            Log.i(TAG, "mnf=" + mnf + ", brand=" + brand + ", model=" + model);
        }

        String val;

        // Make sure we have a mission database - even if it's empty
        MissionDatabase database = MissionDatabase.load(Globals.getSharedPreferences(), Constants.MISSION_DATABASE_NAME);
        if(database == null)
        {
            database = new MissionDatabase();
            database.save(Globals.getSharedPreferences(), Constants.MISSION_DATABASE_NAME);
        }

        // We'll need a network interface for binding
        val = Globals.getSharedPreferences().getString(PreferenceKeys.NETWORK_BINDING_NIC_NAME, null);
        if(Utils.isEmptyString(val))
        {
            NetworkInterface ni = Utils.getFirstViableMulticastNetworkInterface();
            if(ni != null)
            {
                Globals.getSharedPreferencesEditor().putString(PreferenceKeys.NETWORK_BINDING_NIC_NAME, ni.getName());
                Globals.getSharedPreferencesEditor().apply();
            }
        }

        // See if we have a node id.  If not, make one
        val = Globals.getSharedPreferences().getString(PreferenceKeys.USER_NODE_ID, null);
        if(Utils.isEmptyString(val))
        {
            Globals.getSharedPreferencesEditor().putString(PreferenceKeys.USER_NODE_ID, Utils.generateUserNodeId());
            Globals.getSharedPreferencesEditor().apply();
        }

        // See if we have a alias.  If not, make one
        val = Globals.getSharedPreferences().getString(PreferenceKeys.USER_ALIAS_ID, null);
        if(Utils.isEmptyString(val))
        {
            Globals.getSharedPreferencesEditor().putString(PreferenceKeys.USER_ALIAS_ID, Utils.generateUserAlias());
            Globals.getSharedPreferencesEditor().apply();
        }

        // Setup user id if we don't have one
        val = Globals.getSharedPreferences().getString(PreferenceKeys.USER_ID, null);
        if(Utils.isEmptyString(val))
        {
            val = Globals.getSharedPreferences().getString(PreferenceKeys.USER_ALIAS_ID, null) + getString(R.string.generated_user_id_email_domain);
            Globals.getSharedPreferencesEditor().putString(PreferenceKeys.USER_ID, val);
            Globals.getSharedPreferencesEditor().apply();
        }

        // Setup a display name if we don't have one
        val = Globals.getSharedPreferences().getString(PreferenceKeys.USER_DISPLAY_NAME, null);
        if(Utils.isEmptyString(val))
        {
            val = "Android User (" + Globals.getSharedPreferences().getString(PreferenceKeys.USER_ID, null) + ")";
            Globals.getSharedPreferencesEditor().putString(PreferenceKeys.USER_DISPLAY_NAME, val);
            Globals.getSharedPreferencesEditor().apply();
        }

        // See if we have an active configuration.  If we don't we'll make one
        val = Globals.getSharedPreferences().getString(PreferenceKeys.ACTIVE_MISSION_CONFIGURATION_JSON, null);
        if(Utils.isEmptyString(val))
        {
            // Make the sample mission
            Utils.generateSampleMission(this);

            // Load it
            ActiveConfiguration ac = Utils.loadConfiguration(null);

            // Get it from our database
            DatabaseMission mission = database.getMissionById(ac.getMissionId());
            if(mission == null)
            {
                // It doesn't exist - add it
                if( database.addOrUpdateMissionFromActiveConfiguration(ac) )
                {
                    database.save(Globals.getSharedPreferences(), Constants.MISSION_DATABASE_NAME);
                }
            }
        }

        updateActiveConfiguration();
    }

    public Engine getEngine()
    {
        return (_svc != null ? _svc.getEngine() : null);
    }

    public boolean isServiceOnline()
    {
        return (_svc != null);
    }

    private void notifyGroupUiListeners(GroupDescriptor gd)
    {
        synchronized (_uiUpdateListeners)
        {
            for(IUiUpdateListener listener : _uiUpdateListeners)
            {
                listener.onGroupUiRefreshNeeded(gd);
            }
        }
    }

    public final void runOnUiThread(Runnable action)
    {
        if (Thread.currentThread() != getMainLooper().getThread())
        {
            new Handler(Looper.getMainLooper()).post(action);
        }
        else
        {
            action.run();
        }
    }

    private HashSet<GroupDescriptor> _groupsSelectedForTx = new HashSet<>();

    public void startTx(final int priority, final int flags)
    {
        try
        {
            synchronized (_groupsSelectedForTx)
            {
                if(!_groupsSelectedForTx.isEmpty())
                {
                    Log.e(TAG, "attempt to begin tx while there is already a pending/active tx");
                    playGeneralErrorNotification();
                    return;
                }

                ArrayList<GroupDescriptor> selected = getActiveConfiguration().getSelectedGroups();
                if(selected.isEmpty())
                {
                    playGeneralErrorNotification();
                    return;
                }

                for(GroupDescriptor gd : selected)
                {
                    _groupsSelectedForTx.add(gd);
                }

                boolean anyGroupToTxOn = false;
                for (GroupDescriptor g : _groupsSelectedForTx)
                {
                    if(getActiveConfiguration().getUiMode() == Constants.UiMode.vSingle ||
                            (getActiveConfiguration().getUiMode() == Constants.UiMode.vMulti) && (!g.txMuted) )
                    {
                        anyGroupToTxOn = true;
                        g.txPending = true;
                        break;
                    }
                }

                if (!anyGroupToTxOn)
                {
                    _groupsSelectedForTx.clear();
                    playGeneralErrorNotification();
                    return;
                }

                Runnable txTask = new Runnable()
                {
                    @Override
                    public void run()
                    {
                        synchronized (_groupsSelectedForTx)
                        {
                            if(!_groupsSelectedForTx.isEmpty())
                            {
                                for (GroupDescriptor g : _groupsSelectedForTx)
                                {
                                    if(getActiveConfiguration().getUiMode() == Constants.UiMode.vSingle ||
                                            (getActiveConfiguration().getUiMode() == Constants.UiMode.vMulti) && (!g.txMuted) )
                                    {
                                        //getEngine().engageBeginGroupTx(g.id, priority, flags);
                                        getEngine().engageBeginGroupTxAdvanced(g.id, buildAdvancedTxJson(flags, priority, 0, true, _activeConfiguration.getUserAlias()));
                                    }
                                }
                            }
                            else
                            {
                                Log.w(TAG, "tx task runnable found no groups to tx on");
                            }
                        }
                    }
                };

                // TODO: we're already in a sync, now going into another.  is this dangerous
                // considering what we're calling (maybe not...?)
                synchronized (_uiUpdateListeners)
                {
                    for(IUiUpdateListener listener : _uiUpdateListeners)
                    {
                        listener.onAnyTxPending();
                    }
                }

                long now = Utils.nowMs();

                if(_activeConfiguration.getNotifyPttEveryTime() ||
                        ((now - _lastTxActivity) > (Constants.TX_IDLE_SECS_BEFORE_NOTIFICATION * 1000)))
                {
                    _lastTxActivity = now;
                    if(!playTxOnNotification(txTask))
                    {
                        txTask.run();
                    }
                }
                else
                {
                    _lastTxActivity = now;
                    vibrate();
                    txTask.run();
                }
            }
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    public void endTx()
    {
        try
        {
            synchronized (_groupsSelectedForTx)
            {
                if (!_groupsSelectedForTx.isEmpty())
                {
                    for (GroupDescriptor g : _groupsSelectedForTx)
                    {
                        getEngine().engageEndGroupTx(g.id);
                    }

                    _groupsSelectedForTx.clear();

                    // TODO: only play tx off notification if something was already in a TX state of some sort
                    playTxOffNotification();
                }

                synchronized (_uiUpdateListeners)
                {
                    for (IUiUpdateListener listener : _uiUpdateListeners)
                    {
                        listener.onAnyTxEnding();
                    }
                }

                checkIfAnyTxStillActiveAndNotify();
            }
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }

        /*
        if(_txPending)
        {
            _txPending = false;
            Log.i(TAG, "cancelling previous tx pending");
            cancelPreviousTxPending();
        }
        */
    }

    /*
    private void cancelPreviousTxPending()
    {
        boolean notify = false;
        for (GroupDescriptor g : _activeConfiguration.getMissionGroups())
        {
            if(g.txPending)
            {
                g.txPending = false;
                notify = true;
            }
        }

        if(notify)
        {
            synchronized (_uiUpdateListeners)
            {
                for(IUiUpdateListener listener : _uiUpdateListeners)
                {
                    listener.onAllTxEnded();
                }
            }
        }
    }
    */

    private void goIdle()
    {
        stopHardwareButtonManager();
        stopGroupHealthCheckTimer();
        stopLocationUpdates();
    }

    public boolean isEngineRunning()
    {
        return _engineRunning;
    }

    private void checkIfAnyTxStillActiveAndNotify()
    {
        synchronized (_groupsSelectedForTx)
        {
            boolean anyStillActive = (!_groupsSelectedForTx.isEmpty());

            // Safety check
            for (GroupDescriptor testGroup : _activeConfiguration.getMissionGroups())
            {
                if(!_groupsSelectedForTx.contains(testGroup))
                {
                    /*
                    if (testGroup.tx || testGroup.txPending)
                    {
                        Log.wtf(TAG, "data model says group is tx or txPending but the group is not in the tx set!!");
                    }
                    */

                    testGroup.tx = false;
                    testGroup.txPending = false;
                }
            }

            if (!anyStillActive)
            {
                synchronized (_uiUpdateListeners)
                {
                    for (IUiUpdateListener listener : _uiUpdateListeners)
                    {
                        listener.onAllTxEnded();
                    }
                }
            }
        }
    }

    public void initiateMissionDownload(final Activity activity)
    {
        LayoutInflater layoutInflater = LayoutInflater.from(activity);
        View promptView = layoutInflater.inflate(R.layout.download_mission_url_dialog, null);
        AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(activity);
        alertDialogBuilder.setView(promptView);

        final EditText etPassword = promptView.findViewById(R.id.etPassword);
        final EditText etUrl = promptView.findViewById(R.id.etUrl);

        //etUrl.setText("https://s3.us-east-2.amazonaws.com/rts-missions/{a50f4cfb-f200-4cf0-8f88-0401585ba034}.json");

        alertDialogBuilder.setCancelable(false)
                .setPositiveButton(R.string.mission_download_button, new DialogInterface.OnClickListener()
                {
                    public void onClick(DialogInterface dialog, int id)
                    {
                        downloadAndSwitchToMission(etUrl.getText().toString(), etPassword.getText().toString());
                    }
                })
                .setNegativeButton(R.string.cancel,
                        new DialogInterface.OnClickListener()
                        {
                            public void onClick(DialogInterface dialog, int id)
                            {
                                dialog.cancel();
                            }
                        });

        AlertDialog alert = alertDialogBuilder.create();
        alert.show();
    }

    private void downloadAndSwitchToMission(final String url, final String password)
    {
        Handler handler = new Handler() {
            @Override
            public void handleMessage(Message msg)
            {
                try
                {
                    int responseCode = msg.arg1;
                    String resultMsg = msg.getData().getString(DownloadMissionTask.BUNDLE_RESULT_MSG);
                    String resultData = msg.getData().getString(DownloadMissionTask.BUNDLE_RESULT_DATA);

                    if(responseCode >= 200 && responseCode <= 299)
                    {
                        processDownloadedMissionAndSwitchIfOk(resultData, password);
                    }
                    else
                    {
                        Toast.makeText(EngageApplication.this, "Mission download failed - " + resultMsg, Toast.LENGTH_LONG).show();
                    }
                }
                catch (Exception e)
                {
                    Toast.makeText(EngageApplication.this, "Mission download failed with exception " + e.getMessage(), Toast.LENGTH_LONG).show();
                }
            }
        };

        DownloadMissionTask dmt = new DownloadMissionTask(handler);
        dmt.execute(url);
    }

    private void processDownloadedMissionAndSwitchIfOk(String missionData, String password)
    {
        try
        {
            ActiveConfiguration ac = new ActiveConfiguration();
            if(!ac.parseTemplate(missionData))
            {
                throw new Exception("Invalid mission data");
            }

            saveAndActivateConfiguration(ac);

            Toast.makeText(EngageApplication.this,  ac.getMissionName() + " downloaded", Toast.LENGTH_LONG).show();

            synchronized (_configurationChangeListeners)
            {
                for (IConfigurationChangeListener listener : _configurationChangeListeners)
                {
                    listener.onMissionChanged();
                }
            }
        }
        catch(Exception e)
        {
            Toast.makeText(EngageApplication.this, "Exception: " + e.getMessage(), Toast.LENGTH_LONG).show();
        }
    }

    public int getQrCodeScannerRequestCode()
    {
        return IntentIntegrator.REQUEST_CODE;
    }

    public void initiateMissionQrCodeScan(final Activity activity)
    {
        // Clear any left-over password
        Globals.getSharedPreferencesEditor().putString(PreferenceKeys.QR_CODE_SCAN_PASSWORD, null);
        Globals.getSharedPreferencesEditor().apply();

        LayoutInflater layoutInflater = LayoutInflater.from(activity);
        View promptView = layoutInflater.inflate(R.layout.qr_code_mission_scan_password_dialog, null);
        AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(activity);
        alertDialogBuilder.setView(promptView);

        final EditText editText = promptView.findViewById(R.id.etPassword);

        alertDialogBuilder.setCancelable(false)
                .setPositiveButton(R.string.qr_code_scan_button, new DialogInterface.OnClickListener()
                {
                    public void onClick(DialogInterface dialog, int id)
                    {
                        // Save the password so the scanned intent result can get it later
                        Globals.getSharedPreferencesEditor().putString(PreferenceKeys.QR_CODE_SCAN_PASSWORD, editText.getText().toString());
                        Globals.getSharedPreferencesEditor().apply();

                        invokeQrCodeScanner(activity);
                    }
                })
                .setNegativeButton(R.string.cancel,
                        new DialogInterface.OnClickListener()
                        {
                            public void onClick(DialogInterface dialog, int id)
                            {
                                dialog.cancel();
                            }
                        });

        AlertDialog alert = alertDialogBuilder.create();
        alert.show();
    }

    private void invokeQrCodeScanner(Activity activity)
    {
        IntentIntegrator ii = new IntentIntegrator(activity);

        ii.setCaptureActivity(OrientationIndependentQrCodeScanActivity.class);
        ii.setPrompt(getString(R.string.qr_scan_prompt));
        ii.setBeepEnabled(true);
        ii.setOrientationLocked(false);
        ii.setDesiredBarcodeFormats(IntentIntegrator.QR_CODE_TYPES);
        ii.setBarcodeImageEnabled(true);
        ii.setTimeout(10000);
        ii.initiateScan();
    }

    private void saveAndActivateConfiguration(ActiveConfiguration ac)
    {
        // Template
        Globals.getSharedPreferencesEditor().putString(PreferenceKeys.ACTIVE_MISSION_CONFIGURATION_JSON, ac.getInputJson());

        // RP
        Globals.getSharedPreferencesEditor().putBoolean(PreferenceKeys.RP_USE, ac.getUseRp());
        Globals.getSharedPreferencesEditor().putString(PreferenceKeys.RP_ADDRESS, ac.getRpAddress());
        Globals.getSharedPreferencesEditor().putString(PreferenceKeys.RP_PORT, Integer.toString(ac.getRpPort()));
        Globals.getSharedPreferencesEditor().apply();

        // Add this guy to our mission database
        MissionDatabase database = MissionDatabase.load(Globals.getSharedPreferences(), Constants.MISSION_DATABASE_NAME);
        if(database != null)
        {
            if( database.addOrUpdateMissionFromActiveConfiguration(ac) )
            {
                database.save(Globals.getSharedPreferences(), Constants.MISSION_DATABASE_NAME);
            }
            else
            {
                // TODO: how do we let the user know that we could not save into our database ??
            }
        }

        // Our mission has changed
        setMissionChangedStatus(true);
    }

    public ActiveConfiguration processScannedQrCode(String scannedString, String pwd) throws Exception
    {
        ActiveConfiguration ac = ActiveConfiguration.parseEncryptedQrCodeString(scannedString, pwd);

        saveAndActivateConfiguration(ac);

        return ac;
    }

    public ActiveConfiguration processScannedQrCodeResultIntent(int requestCode, int resultCode, Intent intent) throws Exception
    {
        // Grab any password that may have been stored for our purposes
        String pwd = Globals.getSharedPreferences().getString(PreferenceKeys.QR_CODE_SCAN_PASSWORD, "");

        // And clear it
        Globals.getSharedPreferencesEditor().putString(PreferenceKeys.QR_CODE_SCAN_PASSWORD, null);
        Globals.getSharedPreferencesEditor().apply();

        IntentResult result = IntentIntegrator.parseActivityResult(requestCode, resultCode, intent);
        if(result == null)
        {
            return null;
        }

        // Get the string we scanned
        String scannedString = result.getContents();

        if (Utils.isEmptyString(scannedString))
        {
            throw new SimpleMessageException(getString(R.string.qr_scan_cancelled));
        }

        return processScannedQrCode(scannedString, pwd);
    }

    public boolean switchToMission(String id)
    {
        boolean rc;

        try
        {
            MissionDatabase database = MissionDatabase.load(Globals.getSharedPreferences(), Constants.MISSION_DATABASE_NAME);
            DatabaseMission mission = database.getMissionById(id);
            if(mission == null)
            {
                throw new Exception("WTF, no mission by this ID");
            }

            ActiveConfiguration ac = ActiveConfiguration.loadFromDatabaseMission(mission);

            if(ac == null)
            {
                throw new Exception("boom!");
            }

            String serializedAc = ac.makeTemplate().toString();
            SharedPreferences.Editor ed = Globals.getSharedPreferencesEditor();
            ed.putString(PreferenceKeys.ACTIVE_MISSION_CONFIGURATION_JSON, serializedAc);

            ed.putBoolean(PreferenceKeys.RP_USE, ac.getUseRp());
            ed.putString(PreferenceKeys.RP_ADDRESS, ac.getRpAddress());
            ed.putString(PreferenceKeys.RP_PORT, Integer.toString(ac.getRpPort()));

            ed.apply();

            rc = true;
        }
        catch (Exception e)
        {
            rc = false;
        }

        return rc;
    }

    // Handlers, listeners, and overrides ========================================
    @Override
    public void onBluetoothDeviceConnected()
    {
        Log.d(TAG, "onBluetoothDeviceConnected");
    }

    @Override
    public void onBluetoothDeviceDisconnected()
    {
        Log.d(TAG, "onBluetoothDeviceDisconnected");
    }

    @Override
    public void requestPttOn(int priority, int flags)
    {
        startTx(priority, flags);
    }

    @Override
    public void requestPttOff()
    {
        endTx();
    }

    @Override
    public void onEngineStarted()
    {
        Log.d(TAG, "onEngineStarted");
        _engineRunning = true;
        createAllGroupObjects();
        joinSelectedGroups();
        startLocationUpdates();
        startHardwareButtonManager();
    }

    @Override
    public void onEngineStartFailed()
    {
        Log.e(TAG, "onEngineStartFailed");
        _engineRunning = false;
        stop();
    }

    @Override
    public void onEngineStopped()
    {
        Log.d(TAG, "onEngineStopped");
        _engineRunning = false;
        goIdle();
    }

    @Override
    public void onGroupCreated(String id)
    {
        GroupDescriptor gd = getGroup(id);
        if(gd == null)
        {
            Log.e(TAG, "onGroupCreated: cannot find group id='" + id + "'");
            return;
        }

        Log.d(TAG, "onGroupCreated: id='" + id + "', n='" + gd.name + "'");

        gd.resetState();
        gd.created = true;
        gd.createError = false;
        gd.joined = false;
        gd.joinError = false;
        gd.connected = false;

        notifyGroupUiListeners(gd);
    }

    @Override
    public void onGroupCreateFailed(String id)
    {
        GroupDescriptor gd = getGroup(id);
        if(gd == null)
        {
            Log.e(TAG, "onGroupCreateFailed: cannot find group id='" + id + "'");
            return;
        }

        Log.d(TAG, "onGroupCreateFailed: id='" + id + "', n='" + gd.name + "'");

        gd.resetState();
        gd.created = false;
        gd.createError = true;

        notifyGroupUiListeners(gd);
    }

    @Override
    public void onGroupDeleted(String id)
    {
        GroupDescriptor gd = getGroup(id);
        if(gd == null)
        {
            Log.e(TAG, "onGroupDeleted: cannot find group id='" + id + "'");
            return;
        }

        Log.d(TAG, "onGroupDeleted: id='" + id + "', n='" + gd.name + "'");

        gd.resetState();
        gd.created = false;
        gd.createError = false;
        gd.joined = false;
        gd.joinError = false;
        gd.connected = false;

        notifyGroupUiListeners(gd);
    }

    @Override
    public void onGroupConnected(String id)
    {
        GroupDescriptor gd = getGroup(id);
        if(gd == null)
        {
            Log.e(TAG, "onGroupConnected: cannot find group id='" + id + "'");
            return;
        }

        Log.d(TAG, "onGroupConnected: id='" + id + "', n='" + gd.name + "'");

        gd.connected = true;

        // If we get connected to a presence group ...
        if(gd.type == GroupDescriptor.Type.gtPresence)
        {
            // TODO: If we have multiple presence groups, this will generate extra traffic

            // Build whatever PD we currently have and send it
            sendUpdatedPd(buildPd());
        }

        notifyGroupUiListeners(gd);
    }

    @Override
    public void onGroupConnectFailed(String id)
    {
        GroupDescriptor gd = getGroup(id);
        if(gd == null)
        {
            Log.e(TAG, "onGroupConnectFailed: cannot find group id='" + id + "'");
            return;
        }

        Log.d(TAG, "onGroupConnectFailed: id='" + id + "', n='" + gd.name + "'");

        gd.connected = false;

        notifyGroupUiListeners(gd);
    }

    @Override
    public void onGroupDisconnected(String id)
    {
        GroupDescriptor gd = getGroup(id);
        if(gd == null)
        {
            Log.e(TAG, "onGroupDisconnected: cannot find group id='" + id + "'");
            return;
        }

        Log.d(TAG, "onGroupDisconnected: id='" + id + "', n='" + gd.name + "'");

        gd.connected = false;

        notifyGroupUiListeners(gd);
    }

    @Override
    public void onGroupJoined(String id)
    {
        GroupDescriptor gd = getGroup(id);
        if(gd == null)
        {
            Log.e(TAG, "onGroupJoined: cannot find group id='" + id + "'");
            return;
        }

        Log.d(TAG, "onGroupJoined: id='" + id + "', n='" + gd.name + "'");

        gd.joined = true;
        gd.joinError = false;

        notifyGroupUiListeners(gd);
    }

    @Override
    public void onGroupJoinFailed(String id)
    {
        GroupDescriptor gd = getGroup(id);
        if(gd == null)
        {
            Log.e(TAG, "onGroupJoinFailed: cannot find group id='" + id + "'");
            return;
        }

        Log.e(TAG, "onGroupJoinFailed: id='" + id + "', n='" + gd.name + "'");

        gd.resetState();
        gd.joinError = true;

        notifyGroupUiListeners(gd);
    }

    @Override
    public void onGroupLeft(String id)
    {
        GroupDescriptor gd = getGroup(id);
        if(gd == null)
        {
            Log.e(TAG, "onGroupLeft: cannot find group id='" + id + "'");
            return;
        }

        Log.d(TAG, "onGroupLeft: id='" + id + "', n='" + gd.name + "'");

        gd.resetState();
        gd.joined = false;
        gd.joinError = false;

        notifyGroupUiListeners(gd);
    }

    @Override
    public void onGroupMemberCountChanged(String id, long newCount)
    {
        GroupDescriptor gd = getGroup(id);
        if(gd == null)
        {
            Log.e(TAG, "onGroupMemberCountChanged: cannot find group id='" + id + "'");
            return;
        }

        Log.d(TAG, "onGroupMemberCountChanged: id='" + id + "', n=" + gd.name + ", c=" + newCount);

        notifyGroupUiListeners(gd);
    }

    @Override
    public void onGroupRxStarted(String id)
    {
        GroupDescriptor gd = getGroup(id);
        if(gd == null)
        {
            Log.e(TAG, "onGroupRxStarted: cannot find group id='" + id + "'");
            return;
        }

        Log.d(TAG, "onGroupRxStarted: id='" + id + "', n='" + gd.name + "'");

        gd.rx = true;

        long now = Utils.nowMs();
        if((now - _lastAudioActivity) > (Constants.RX_IDLE_SECS_BEFORE_NOTIFICATION * 1000))
        {
            if(_activeConfiguration.getNotifyOnNewAudio())
            {
                float volume = _activeConfiguration.getNotificationToneNotificationLevel();
                if (volume != 0.0)
                {
                    try
                    {
                        Globals.getAudioPlayerManager().playNotification(R.raw.incoming_rx, volume, null);
                    }
                    catch (Exception e)
                    {
                    }
                }
            }
        }
        _lastAudioActivity = now;

        notifyGroupUiListeners(gd);
    }

    @Override
    public void onGroupRxEnded(String id)
    {
        GroupDescriptor gd = getGroup(id);
        if(gd == null)
        {
            Log.e(TAG, "onGroupRxEnded: cannot find group id='" + id + "'");
            return;
        }

        Log.d(TAG, "onGroupRxEnded: id='" + id + "', n='" + gd.name + "'");

        gd.rx = false;
        _lastAudioActivity = Utils.nowMs();

        notifyGroupUiListeners(gd);
    }

    @Override
    public void onGroupRxSpeakersChanged(String id, String groupTalkerJson)
    {
        GroupDescriptor gd = getGroup(id);
        if(gd == null)
        {
            Log.e(TAG, "onGroupRxSpeakersChanged: cannot find group id='" + id + "'");
            return;
        }

        Log.d(TAG, "onGroupRxSpeakersChanged: id='" + id + "', n='" + gd.name + "'");

        ArrayList<TalkerDescriptor> talkers = null;

        if(!Utils.isEmptyString(groupTalkerJson))
        {
            try
            {
                JSONObject root = new JSONObject(groupTalkerJson);
                JSONArray list = root.getJSONArray(Engine.JsonFields.GroupTalkers.list);
                if(list != null && list.length() > 0)
                {
                    for(int x = 0; x < list.length(); x++)
                    {
                        JSONObject obj = list.getJSONObject(x);
                        TalkerDescriptor td = new TalkerDescriptor();
                        td.alias = obj.optString(Engine.JsonFields.TalkerInformation.alias);
                        td.nodeId = obj.optString(Engine.JsonFields.TalkerInformation.nodeId);

                        if(talkers == null)
                        {
                            talkers = new ArrayList<>();
                        }

                        talkers.add(td);
                    }
                }
            }
            catch(Exception e)
            {
                if(talkers != null)
                {
                    talkers.clear();
                    talkers = null;
                }

                e.printStackTrace();
            }
        }

        gd.updateTalkers(talkers);
        _lastAudioActivity = Utils.nowMs();

        notifyGroupUiListeners(gd);
    }

    @Override
    public void onGroupRxMuted(String id)
    {
        GroupDescriptor gd = getGroup(id);
        if(gd == null)
        {
            Log.e(TAG, "onGroupRxMuted: cannot find group id='" + id + "'");
            return;
        }

        Log.d(TAG, "onGroupRxMuted: id='" + id + "', n='" + gd.name + "'");

        gd.rxMuted = true;

        notifyGroupUiListeners(gd);
    }

    @Override
    public void onGroupRxUnmuted(String id)
    {
        GroupDescriptor gd = getGroup(id);
        if(gd == null)
        {
            Log.e(TAG, "onGroupRxUnmuted: cannot find group id='" + id + "'");
            return;
        }

        Log.d(TAG, "onGroupRxUnmuted: id='" + id + "', n='" + gd.name + "'");

        gd.rxMuted = false;

        notifyGroupUiListeners(gd);
    }

    @Override
    public void onGroupTxStarted(String id)
    {
        GroupDescriptor gd = getGroup(id);
        if(gd == null)
        {
            Log.e(TAG, "onGroupTxStarted: cannot find group id='" + id + "'");
            return;
        }

        Log.d(TAG, "onGroupTxStarted: id='" + id + "', n='" + gd.name + "'");

        gd.tx = true;
        gd.txPending = false;
        gd.txError = false;
        gd.txUsurped = false;

        _lastAudioActivity = Utils.nowMs();

        notifyGroupUiListeners(gd);

        synchronized (_uiUpdateListeners)
        {
            for(IUiUpdateListener listener : _uiUpdateListeners)
            {
                listener.onAnyTxActive();
            }
        }
    }

    @Override
    public void onGroupTxEnded(String id)
    {
        GroupDescriptor gd = getGroup(id);
        if(gd == null)
        {
            Log.e(TAG, "onGroupTxEnded: cannot find group id='" + id + "'");
            return;
        }

        Log.d(TAG, "onGroupTxEnded: id='" + id + "', n='" + gd.name + "'");

        gd.tx = false;
        gd.txPending = false;
        gd.txError = false;

        _lastAudioActivity = Utils.nowMs();

        synchronized (_groupsSelectedForTx)
        {
            _groupsSelectedForTx.remove(gd);
            notifyGroupUiListeners(gd);
            checkIfAnyTxStillActiveAndNotify();
        }
    }

    @Override
    public void onGroupTxFailed(String id)
    {
        GroupDescriptor gd = getGroup(id);
        if(gd == null)
        {
            Log.e(TAG, "onGroupTxFailed: cannot find group id='" + id + "'");
            return;
        }

        Log.d(TAG, "onGroupTxFailed: id='" + id + "', n='" + gd.name + "'");

        gd.tx = false;
        gd.txPending = false;
        gd.txError = true;
        gd.txUsurped = false;

        _lastAudioActivity = Utils.nowMs();

        synchronized (_groupsSelectedForTx)
        {
            _groupsSelectedForTx.remove(gd);
            playGeneralErrorNotification();
            notifyGroupUiListeners(gd);
            checkIfAnyTxStillActiveAndNotify();
        }
    }

    @Override
    public void onGroupTxUsurpedByPriority(String id)
    {
        GroupDescriptor gd = getGroup(id);
        if(gd == null)
        {
            Log.e(TAG, "onGroupTxUsurpedByPriority: cannot find group id='" + id + "'");
            return;
        }

        Log.d(TAG, "onGroupTxUsurpedByPriority: id='" + id + "', n='" + gd.name + "'");

        gd.tx = false;
        gd.txPending = false;
        gd.txError = false;
        gd.txUsurped = true;

        _lastAudioActivity = Utils.nowMs();

        synchronized (_groupsSelectedForTx)
        {
            _groupsSelectedForTx.remove(gd);
            playGeneralErrorNotification();
            notifyGroupUiListeners(gd);
            checkIfAnyTxStillActiveAndNotify();
        }


        synchronized (_uiUpdateListeners)
        {
            for(IUiUpdateListener listener : _uiUpdateListeners)
            {
                listener.onGroupTxUsurped(gd);
            }
        }
    }

    @Override
    public void onGroupMaxTxTimeExceeded(String id)
    {
        GroupDescriptor gd = getGroup(id);
        if(gd == null)
        {
            Log.e(TAG, "onGroupMaxTxTimeExceeded: cannot find group id='" + id + "'");
            return;
        }

        Log.d(TAG, "onGroupMaxTxTimeExceeded: id='" + id + "', n='" + gd.name + "'");

        gd.tx = false;
        gd.txPending = false;
        gd.txError = false;
        gd.txUsurped = true;

        _lastAudioActivity = Utils.nowMs();

        synchronized (_groupsSelectedForTx)
        {
            _groupsSelectedForTx.remove(gd);
            playGeneralErrorNotification();
            notifyGroupUiListeners(gd);
            checkIfAnyTxStillActiveAndNotify();
        }

        synchronized (_uiUpdateListeners)
        {
            for(IUiUpdateListener listener : _uiUpdateListeners)
            {
                listener.onGroupMaxTxTimeExceeded(gd);
            }
        }
    }

    @Override
    public void onGroupNodeDiscovered(String id, String nodeJson)
    {
        GroupDescriptor gd = getGroup(id);
        if(gd == null)
        {
            Log.e(TAG, "onGroupNodeDiscovered: cannot find group id='" + id + "'");
            return;
        }

        Log.d(TAG, "onGroupNodeDiscovered: id='" + id + "', n='" + gd.name + "'");

        PresenceDescriptor pd = getActiveConfiguration().processNodeDiscovered(nodeJson);
        if(pd != null)
        {
            if(!pd.self && _activeConfiguration.getNotifyOnNodeJoin())
            {
                float volume = _activeConfiguration.getNotificationToneNotificationLevel();
                if (volume != 0.0)
                {
                    try
                    {
                        Globals.getAudioPlayerManager().playNotification(R.raw.node_join, volume, null);
                    }
                    catch (Exception e)
                    {
                    }
                }
            }

            synchronized (_presenceChangeListeners)
            {
                for(IPresenceChangeListener listener : _presenceChangeListeners)
                {
                    listener.onPresenceAdded(pd);
                }
            }

            notifyGroupUiListeners(gd);
        }
    }

    @Override
    public void onGroupNodeRediscovered(String id, String nodeJson)
    {
        GroupDescriptor gd = getGroup(id);
        if(gd == null)
        {
            Log.e(TAG, "onGroupNodeRediscovered: cannot find group id='" + id + "'");
            return;
        }

        Log.d(TAG, "onGroupNodeRediscovered: id='" + id + "', n='" + gd.name + "'");

        PresenceDescriptor pd = getActiveConfiguration().processNodeDiscovered(nodeJson);
        if(pd != null)
        {
            synchronized (_presenceChangeListeners)
            {
                for(IPresenceChangeListener listener : _presenceChangeListeners)
                {
                    listener.onPresenceChange(pd);
                }
            }

            notifyGroupUiListeners(gd);
        }
    }

    @Override
    public void onGroupNodeUndiscovered(String id, String nodeJson)
    {
        GroupDescriptor gd = getGroup(id);
        if(gd == null)
        {
            Log.e(TAG, "onGroupNodeUndiscovered: cannot find group id='" + id + "'");
            return;
        }

        Log.d(TAG, "onGroupNodeUndiscovered: id='" + id + "', n='" + gd.name + "'");

        PresenceDescriptor pd = getActiveConfiguration().processNodeUndiscovered(nodeJson);
        if(pd != null)
        {
            if(!pd.self && _activeConfiguration.getNotifyOnNodeLeave())
            {
                float volume = _activeConfiguration.getNotificationToneNotificationLevel();
                if (volume != 0.0)
                {
                    try
                    {
                        Globals.getAudioPlayerManager().playNotification(R.raw.node_leave, volume, null);
                    }
                    catch (Exception e)
                    {
                    }
                }
            }

            synchronized (_presenceChangeListeners)
            {
                for(IPresenceChangeListener listener : _presenceChangeListeners)
                {
                    listener.onPresenceRemoved(pd);
                }
            }

            notifyGroupUiListeners(gd);
        }
    }

    public boolean getLicenseExpired()
    {
        return _licenseExpired;
    }

    public long getLicenseSecondsLeft()
    {
        return _licenseSecondsLeft;
    }

    @Override
    public void onLicenseChanged()
    {
        Log.d(TAG, "onLicenseChanged");
        _licenseExpired = false;
        _licenseSecondsLeft = 0;
        cancelObtainingActivationCode();

        synchronized (_licenseChangeListeners)
        {
            for (ILicenseChangeListener listener : _licenseChangeListeners)
            {
                listener.onLicenseChanged();
            }
        }
    }

    @Override
    public void onLicenseExpired()
    {
        Log.d(TAG, "onLicenseExpired");
        _licenseExpired = true;
        _licenseSecondsLeft = 0;
        scheduleObtainingActivationCode();

        synchronized (_licenseChangeListeners)
        {
            for (ILicenseChangeListener listener : _licenseChangeListeners)
            {
                listener.onLicenseExpired();
            }
        }
    }

    @Override
    public void onLicenseExpiring(long secondsLeft)
    {
        Log.d(TAG, "onLicenseExpiring - " + secondsLeft + " seconds remaining");
        _licenseExpired = false;
        _licenseSecondsLeft = secondsLeft;
        scheduleObtainingActivationCode();

        synchronized (_licenseChangeListeners)
        {
            for (ILicenseChangeListener listener : _licenseChangeListeners)
            {
                listener.onLicenseExpiring(secondsLeft);
            }
        }
    }

    private String __devOnly__groupId = "SIM0001";

    public void __devOnly__RunTest()
    {
        if(!_dynamicGroups.containsKey(__devOnly__groupId))
        {
            __devOnly__simulateGroupAssetDiscovered();
        }
        else
        {
            __devOnly__simulateGroupAssetUndiscovered();
        }
    }

    public void __devOnly__simulateGroupAssetDiscovered()
    {
        try
        {
            JSONObject group = new JSONObject();

            group.put(Engine.JsonFields.Group.id, __devOnly__groupId);
            group.put(Engine.JsonFields.Group.name, "Simulated Group 1");
            group.put(Engine.JsonFields.Group.type, GroupDescriptor.Type.gtAudio.ordinal());

            // RX
            {
                JSONObject rx = new JSONObject();
                rx.put(Engine.JsonFields.Rx.address, "234.5.6.7");
                rx.put(Engine.JsonFields.Rx.port, 29000);
                group.put(Engine.JsonFields.Rx.objectName, rx);
            }

            // TX
            {
                JSONObject tx = new JSONObject();
                tx.put(Engine.JsonFields.Tx.address, "234.5.6.7");
                tx.put(Engine.JsonFields.Tx.port, 29000);
                group.put(Engine.JsonFields.Tx.objectName, tx);
            }

            // TxAudio
            {
                JSONObject txAudio = new JSONObject();
                txAudio.put(Engine.JsonFields.TxAudio.encoder, 1);
                txAudio.put(Engine.JsonFields.TxAudio.framingMs, 20);
                txAudio.put(Engine.JsonFields.TxAudio.noHdrExt, true);
                group.put(Engine.JsonFields.TxAudio.objectName, txAudio);
            }

            String json = group.toString();

            onGroupAssetDiscovered(__devOnly__groupId, json);
            getEngine().engageCreateGroup(buildFinalGroupJsonConfiguration(json));
        }
        catch (Exception e)
        {
            Toast.makeText(this, "EXCEPTION: " + e.getMessage(), Toast.LENGTH_LONG).show();
        }
    }

    public void __devOnly__simulateGroupAssetUndiscovered()
    {
        try
        {
            JSONObject group = new JSONObject();

            group.put(Engine.JsonFields.Group.id, __devOnly__groupId);
            group.put(Engine.JsonFields.Group.name, "Simulated Group 1");
            group.put(Engine.JsonFields.Group.type, GroupDescriptor.Type.gtAudio.ordinal());

            // RX
            {
                JSONObject rx = new JSONObject();
                rx.put(Engine.JsonFields.Rx.address, "234.5.6.7");
                rx.put(Engine.JsonFields.Rx.port, 29000);
                group.put(Engine.JsonFields.Rx.objectName, rx);
            }

            // TX
            {
                JSONObject tx = new JSONObject();
                tx.put(Engine.JsonFields.Tx.address, "234.5.6.7");
                tx.put(Engine.JsonFields.Tx.port, 29000);
                group.put(Engine.JsonFields.Tx.objectName, tx);
            }

            // TxAudio
            {
                JSONObject txAudio = new JSONObject();
                txAudio.put(Engine.JsonFields.TxAudio.encoder, 1);
                txAudio.put(Engine.JsonFields.TxAudio.framingMs, 20);
                txAudio.put(Engine.JsonFields.TxAudio.noHdrExt, true);
                group.put(Engine.JsonFields.TxAudio.objectName, txAudio);
            }

            String json = group.toString();

            onGroupAssetUndiscovered(__devOnly__groupId, json);
            getEngine().engageDeleteGroup(__devOnly__groupId);
        }
        catch (Exception e)
        {
            Toast.makeText(this, "EXCEPTION: " + e.getMessage(), Toast.LENGTH_LONG).show();
        }
    }

    public void restartStartHumanBiometricsReporting()
    {
        stopHumanBiometricsReporting();
        startHumanBiometricsReporting();
    }


    public void startHumanBiometricsReporting()
    {
        if(_humanBiometricsReportingTimer == null)
        {
            if(Globals.getSharedPreferences().getBoolean(PreferenceKeys.USER_EXPERIMENT_ENABLE_HBM, false))
            {
                _hbmTicksBeforeReport = Integer.parseInt(Globals.getSharedPreferences().getString(PreferenceKeys.USER_EXPERIMENT_HBM_INTERVAL_SECS, "0"));
                if(_hbmTicksBeforeReport >= 1)
                {
                    _hbmTicksSoFar = 0;

                    _hbmHeartRate = new DataSeries(Engine.HumanBiometricsElement.heartRate.toInt());
                    _hbmSkinTemp = new DataSeries(Engine.HumanBiometricsElement.skinTemp.toInt());
                    _hbmCoreTemp = new DataSeries(Engine.HumanBiometricsElement.coreTemp.toInt());
                    _hbmHydration = new DataSeries(Engine.HumanBiometricsElement.hydration.toInt());
                    _hbmBloodOxygenation = new DataSeries(Engine.HumanBiometricsElement.bloodOxygenation.toInt());
                    _hbmFatigueLevel = new DataSeries(Engine.HumanBiometricsElement.fatigueLevel.toInt());
                    _hbmTaskEffectiveness = new DataSeries(Engine.HumanBiometricsElement.taskEffectiveness.toInt());

                    _rhbmgHeart = new RandomHumanBiometricGenerator(50, 175, 15, 75);
                    _rhbmgSkinTemp = new RandomHumanBiometricGenerator(30, 38, 2, 33);
                    _rhbmgCoreTemp = new RandomHumanBiometricGenerator(35, 37, 1, 36);
                    _rhbmgHydration = new RandomHumanBiometricGenerator(60, 100, 3, 90);
                    _rhbmgOxygenation = new RandomHumanBiometricGenerator(87, 100, 5, 94);
                    _rhbmgFatigueLevel = new RandomHumanBiometricGenerator(0, 10, 3, 3);
                    _rhbmgTaskEffectiveness = new RandomHumanBiometricGenerator(0, 10, 3, 3);

                    _humanBiometricsReportingTimer = new Timer();
                    _humanBiometricsReportingTimer.scheduleAtFixedRate(new TimerTask()
                    {
                        @Override
                        public void run()
                        {
                            onHumanBiometricsTimerTick();
                        }
                    }, 0, 1000);
                }
            }
        }
    }

    public void stopHumanBiometricsReporting()
    {
        if(_humanBiometricsReportingTimer != null)
        {
            _humanBiometricsReportingTimer.cancel();
            _humanBiometricsReportingTimer = null;
        }
    }

    private void onHumanBiometricsTimerTick()
    {
        if(_hbmTicksSoFar == 0)
        {
            _hbmHeartRate.restart();
            _hbmSkinTemp.restart();
            _hbmCoreTemp.restart();
            _hbmHydration.restart();
            _hbmBloodOxygenation.restart();
            _hbmFatigueLevel.restart();
            _hbmTaskEffectiveness.restart();
        }

        _hbmHeartRate.addElement((byte)1, (byte)_rhbmgHeart.nextInt());
        _hbmSkinTemp.addElement((byte)1, (byte)_rhbmgSkinTemp.nextInt());
        _hbmCoreTemp.addElement((byte)1, (byte)_rhbmgCoreTemp.nextInt());
        _hbmHydration.addElement((byte)1, (byte)_rhbmgHydration.nextInt());
        _hbmBloodOxygenation.addElement((byte)1, (byte)_rhbmgOxygenation.nextInt());
        _hbmFatigueLevel.addElement((byte)1, (byte)_rhbmgFatigueLevel.nextInt());
        _hbmTaskEffectiveness.addElement((byte)1, (byte)_rhbmgTaskEffectiveness.nextInt());

        _hbmTicksSoFar++;

        if(_hbmTicksSoFar == _hbmTicksBeforeReport)
        {
            try
            {
                ByteArrayOutputStream bas = new ByteArrayOutputStream();

                if(Globals.getSharedPreferences().getBoolean(PreferenceKeys.USER_EXPERIMENT_HBM_ENABLE_HEART_RATE, false))
                {
                    bas.write(_hbmHeartRate.toByteArray());
                }

                if(Globals.getSharedPreferences().getBoolean(PreferenceKeys.USER_EXPERIMENT_HBM_ENABLE_SKIN_TEMP, false))
                {
                    bas.write(_hbmSkinTemp.toByteArray());
                }

                if(Globals.getSharedPreferences().getBoolean(PreferenceKeys.USER_EXPERIMENT_HBM_ENABLE_CORE_TEMP, false))
                {
                    bas.write(_hbmCoreTemp.toByteArray());
                }

                if(Globals.getSharedPreferences().getBoolean(PreferenceKeys.USER_EXPERIMENT_HBM_ENABLE_BLOOD_HYDRO, false))
                {
                    bas.write(_hbmHydration.toByteArray());
                }

                if(Globals.getSharedPreferences().getBoolean(PreferenceKeys.USER_EXPERIMENT_HBM_ENABLE_BLOOD_OXY, false))
                {
                    bas.write(_hbmBloodOxygenation.toByteArray());
                }

                if(Globals.getSharedPreferences().getBoolean(PreferenceKeys.USER_EXPERIMENT_HBM_ENABLE_FATIGUE_LEVEL, false))
                {
                    bas.write(_hbmFatigueLevel.toByteArray());
                }

                if(Globals.getSharedPreferences().getBoolean(PreferenceKeys.USER_EXPERIMENT_HBM_ENABLE_TASK_EFFECTIVENESS_LEVEL, false))
                {
                    bas.write(_hbmTaskEffectiveness.toByteArray());
                }

                byte[] blob = bas.toByteArray();

                if(blob.length > 0)
                {
                    Log.i(TAG, "Reporting human biometrics data - blob size is " + blob.length + " bytes");

                    // Our JSON parameters indicate that the payload is binary human biometric data in Engage format
                    JSONObject bi = new JSONObject();
                    bi.put(Engine.JsonFields.BlobHeader.payloadType, Engine.BlobType.engageHumanBiometrics.toInt());
                    String jsonParams = bi.toString();

                    ActiveConfiguration ac = getActiveConfiguration();
                    for(GroupDescriptor gd : ac.getMissionGroups())
                    {
                        if(gd.type == GroupDescriptor.Type.gtPresence)
                        {
                            getEngine().engageSendGroupBlob(gd.id, blob, blob.length, jsonParams);
                        }
                    }
                }
                else
                {
                    Log.w(TAG, "Cannot report human biometrics data - no presence group found");
                }
            }
            catch(Exception e)
            {
                e.printStackTrace();
            }

            _hbmTicksSoFar = 0;
        }
    }

    @Override
    public void onGroupAssetDiscovered(String id, String nodeJson)
    {
        Log.d(TAG, "onGroupAssetDiscovered: id='" + id + "', json='" + nodeJson + "'");

        synchronized (_assetChangeListeners)
        {
            for(IAssetChangeListener listener : _assetChangeListeners)
            {
                listener.onAssetDiscovered(id, nodeJson);
            }
        }

        boolean notify = false;
        synchronized (_dynamicGroups)
        {
            GroupDescriptor gd = _dynamicGroups.get(id);
            if(gd == null)
            {
                gd = new GroupDescriptor();
                if(gd.loadFromJson(nodeJson))
                {
                    gd.setDynamic(true);
                    gd.selectedForMultiView = true;
                    _dynamicGroups.put(id, gd);
                    notify = true;
                }
                else
                {
                    Log.e(TAG, "onGroupAssetDiscovered: failed to load group descriptor from json");
                }
            }
        }

        if(notify)
        {
            playAssetDiscoveredNotification();

            synchronized (_configurationChangeListeners)
            {
                for (IConfigurationChangeListener listener : _configurationChangeListeners)
                {
                    listener.onCriticalConfigurationChange();
                }
            }
        }
    }

    @Override
    public void onGroupAssetRediscovered(String id, String nodeJson)
    {
        synchronized (_assetChangeListeners)
        {
            for(IAssetChangeListener listener : _assetChangeListeners)
            {
                listener.onAssetRediscovered(id, nodeJson);
            }
        }

        boolean notify = false;
        synchronized (_dynamicGroups)
        {
            GroupDescriptor gd = _dynamicGroups.get(id);
            if(gd == null)
            {
                gd = new GroupDescriptor();
                if(gd.loadFromJson(nodeJson))
                {
                    gd.setDynamic(true);
                    gd.selectedForMultiView = true;
                    _dynamicGroups.put(id, gd);
                    notify = true;
                }
                else
                {
                    Log.e(TAG, "onGroupAssetRediscovered: failed to load group descriptor from json");
                }
            }
        }

        if(notify)
        {
            playAssetDiscoveredNotification();

            synchronized (_configurationChangeListeners)
            {
                for (IConfigurationChangeListener listener : _configurationChangeListeners)
                {
                    listener.onCriticalConfigurationChange();
                }
            }
        }
    }

    @Override
    public void onGroupAssetUndiscovered(String id, String nodeJson)
    {
        synchronized (_assetChangeListeners)
        {
            for(IAssetChangeListener listener : _assetChangeListeners)
            {
                listener.onAssetUndiscovered(id, nodeJson);
            }
        }

        boolean notify = false;
        synchronized (_dynamicGroups)
        {
            if(_dynamicGroups.containsKey(id))
            {
                _dynamicGroups.remove(id);
                notify = true;
            }
        }

        if(notify)
        {
            playAssetUndiscoveredNotification();
            synchronized (_configurationChangeListeners)
            {
                for (IConfigurationChangeListener listener : _configurationChangeListeners)
                {
                    listener.onCriticalConfigurationChange();
                }
            }
        }
    }

    @Override
    public void onGroupBlobSent(String id)
    {
        Log.d(TAG, "onGroupBlobSent");
    }

    @Override
    public void onGroupBlobSendFailed(String id)
    {
        Log.e(TAG, "onGroupBlobSendFailed");
    }

    @Override
    public void onGroupBlobReceived(String id, String blobInfoJson, byte[] blob, long blobSize)
    {
        Log.d(TAG, "onGroupBlobReceived: blobInfoJson=" + blobInfoJson);

        try
        {
            JSONObject blobInfo = new JSONObject(blobInfoJson);

            int payloadType = blobInfo.getInt("payloadType");
            String nodeId = blobInfo.getString("source");
            PresenceDescriptor pd = _activeConfiguration.getPresenceDescriptor(nodeId);

            // Human biometrics ... ?
            if (Engine.BlobType.fromInt(payloadType) == Engine.BlobType.engageHumanBiometrics)
            {
                int blobOffset = 0;
                int bytesLeft = (int)blobSize;
                boolean anythingUpdated = false;

                while( bytesLeft > 0 )
                {
                    DataSeries ds = new DataSeries();
                    int bytesProcessed = ds.parseByteArray(blob, blobOffset, bytesLeft);
                    if(bytesProcessed <= 0)
                    {
                        throw new Exception("Error processing HBM");
                    }

                    bytesLeft -= bytesProcessed;
                    blobOffset += bytesProcessed;

                    if(pd.updateBioMetrics(ds))
                    {
                        anythingUpdated = true;
                    }
                }

                if(anythingUpdated)
                {
                    synchronized (_presenceChangeListeners)
                    {
                        for(IPresenceChangeListener listener : _presenceChangeListeners)
                        {
                            listener.onPresenceChange(pd);
                        }
                    }
                }
            }
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    @Override
    public void onGroupRtpSent(String id)
    {
        Log.d(TAG, "onGroupRtpSent");
    }

    @Override
    public void onGroupRtpSendFailed(String id)
    {
        Log.e(TAG, "onGroupRtpSendFailed");
    }

    @Override
    public void onGroupRtpReceived(String id, String rtpHeaderJson, byte[] payload, long payloadSize)
    {
        Log.d(TAG, "onGroupRtpReceived: rtpHeaderJson=" + rtpHeaderJson);
    }

    public void onGroupRawSent(String id)
    {
        Log.d(TAG, "onGroupRawSent");
    }

    @Override
    public void onGroupRawSendFailed(String id)
    {
        Log.e(TAG, "onGroupRawSendFailed");
    }

    @Override
    public void onGroupRawReceived(String id, byte[] raw, long rawsize)
    {
        Log.d(TAG, "onGroupRawReceived");
    }

    @Override
    public void onGroupTimelineEventStarted(String id, String eventJson)
    {
        Log.d(TAG, "onGroupTimelineEventStarted: " + id);

        final GroupDescriptor gd = getGroup(id);
        if(gd == null)
        {
            Log.e(TAG, "onGroupTimelineEventStarted: cannot find group id='" + id + "'");
            return;
        }

        synchronized (_groupTimelineListeners)
        {
            for (IGroupTimelineListener listener : _groupTimelineListeners)
            {
                listener.onGroupTimelineEventStarted(gd, eventJson);
            }
        }
    }

    @Override
    public void onGroupTimelineEventUpdated(String id, String eventJson)
    {
        Log.d(TAG, "onGroupTimelineEventUpdated: " + id);

        final GroupDescriptor gd = getGroup(id);
        if(gd == null)
        {
            Log.e(TAG, "onGroupTimelineEventUpdated: cannot find group id='" + id + "'");
            return;
        }

        synchronized (_groupTimelineListeners)
        {
            for (IGroupTimelineListener listener : _groupTimelineListeners)
            {
                listener.onGroupTimelineEventUpdated(gd, eventJson);
            }
        }
    }

    @Override
    public void onGroupTimelineEventEnded(String id, String eventJson)
    {
        Log.d(TAG, "onGroupTimelineEventEnded: " + id);

        final GroupDescriptor gd = getGroup(id);
        if(gd == null)
        {
            Log.e(TAG, "onGroupTimelineEventEnded: cannot find group id='" + id + "'");
            return;
        }

        synchronized (_groupTimelineListeners)
        {
            for (IGroupTimelineListener listener : _groupTimelineListeners)
            {
                listener.onGroupTimelineEventEnded(gd, eventJson);
            }
        }
    }

    @Override
    public void onGroupTimelineReport(String id, String reportJson)
    {
        Log.d(TAG, "onGroupTimelineReport: " + id);

        final GroupDescriptor gd = getGroup(id);
        if(gd == null)
        {
            Log.e(TAG, "onGroupTimelineReport: cannot find group id='" + id + "'");
            return;
        }

        synchronized (_groupTimelineListeners)
        {
            for (IGroupTimelineListener listener : _groupTimelineListeners)
            {
                listener.onGroupTimelineReport(gd, reportJson);
            }
        }
    }

    @Override
    public void onGroupTimelineReportFailed(String id)
    {
        Log.d(TAG, "onGroupTimelineReportFailed: " + id);

        final GroupDescriptor gd = getGroup(id);
        if(gd == null)
        {
            Log.e(TAG, "onGroupTimelineReportFailed: cannot find group id='" + id + "'");
            return;
        }

        synchronized (_groupTimelineListeners)
        {
            for (IGroupTimelineListener listener : _groupTimelineListeners)
            {
                listener.onGroupTimelineReportFailed(gd);
            }
        }
    }

    @Override
    public void onRallypointPausingConnectionAttempt(String id)
    {
        Log.d(TAG, "onRallypointPausingConnectionAttempt");
        // Stub
    }

    @Override
    public void onRallypointConnecting(String id)
    {
        Log.d(TAG, "onRallypointConnecting: " + id);
        // Stub
    }

    @Override
    public void onRallypointConnected(String id)
    {
        Log.d(TAG, "onRallypointConnected: " + id);
        // Stub
    }

    @Override
    public void onRallypointDisconnected(String id)
    {
        Log.d(TAG, "onRallypointDisconnected: " + id);
        // Stub
    }

    @Override
    public void onRallypointRoundtripReport(String id, long rtMs, long rtQualityRating)
    {
        Log.d(TAG, "onRallypointRoundtripReport: " + id + ", ms=" + rtMs + ", qual=" + rtQualityRating);
        // Stub
    }

    public void pauseLicenseActivation()
    {
        runOnUiThread(new Runnable()
          {
              @Override
              public void run()
              {
                  Log.d(TAG, "pauseLicenseActivation");
                  _licenseActivationPaused = true;
              }
          });
    }

    public void resumeLicenseActivation()
    {
        runOnUiThread(new Runnable()
        {
            @Override
            public void run()
            {
                Log.d(TAG, "resumeLicenseActivation");
                _licenseActivationPaused = false;
            }
        });
    }

    private void scheduleObtainingActivationCode()
    {
        runOnUiThread(new Runnable()
        {
            @Override
            public void run()
            {
                if(_licenseActivationTimer != null)
                {
                    return;
                }

                long delay = 0;

                if(_licenseExpired)
                {
                    delay = Constants.MIN_LICENSE_ACTIVATION_DELAY_MS;
                }
                else
                {
                    if(_licenseSecondsLeft > 0)
                    {
                        delay = ((_licenseSecondsLeft / 2) * 1000);
                    }
                    else
                    {
                        delay = Constants.MIN_LICENSE_ACTIVATION_DELAY_MS;
                    }
                }

                if(delay < Constants.MIN_LICENSE_ACTIVATION_DELAY_MS)
                {
                    delay = Constants.MIN_LICENSE_ACTIVATION_DELAY_MS;
                }

                Log.i(TAG, "scheduling obtaining of activation code in " + (delay / 1000) + " seconds");

                _licenseActivationTimer = new Timer();
                _licenseActivationTimer.schedule(new TimerTask()
                {
                    @Override
                    public void run()
                    {
                        obtainActivationCode();
                    }
                }, delay);
            }
        });
    }

    private void cancelObtainingActivationCode()
    {
        runOnUiThread(new Runnable()
        {
            @Override
            public void run()
            {
                if(_licenseActivationTimer != null)
                {
                    _licenseActivationTimer.cancel();
                    _licenseActivationTimer = null;
                }
            }
        });
    }

    private void obtainActivationCode()
    {
        runOnUiThread(new Runnable()
        {
            @Override
            public void run()
            {
                if(_licenseActivationPaused)
                {
                    Log.d(TAG, "license activation paused - rescheduling");

                    // Schedule for another time
                    scheduleObtainingActivationCode();
                }
                else
                {
                    try
                    {
                        Log.i(TAG, "attempting to obtain a license activation code");

                        cancelObtainingActivationCode();

                        String jsonData = getEngine().engageGetActiveLicenseDescriptor();
                        JSONObject obj = new JSONObject(jsonData);
                        String deviceId = obj.getString(Engine.JsonFields.License.deviceId);
                        if (Utils.isEmptyString(deviceId))
                        {
                            throw new Exception("no device id available for licensing");
                        }

                        String key = Globals.getSharedPreferences().getString(PreferenceKeys.USER_LICENSING_KEY, "");
                        if (Utils.isEmptyString(key))
                        {
                            throw new Exception("no license key available for licensing");
                        }

                        String url;
                        if(Globals.getSharedPreferences().getBoolean(PreferenceKeys.DEVELOPER_USE_DEV_LICENSING_SYSTEM, false))
                        {
                            url = getString(R.string.online_licensing_activation_url_dev);
                        }
                        else
                        {
                            url = getString(R.string.online_licensing_activation_url_prod);
                        }

                        String ac = Globals.getSharedPreferences().getString(PreferenceKeys.USER_LICENSING_ACTIVATION_CODE, "");

                        String stringToHash = key + deviceId + getString(R.string.licensing_entitlement);
                        String hValue = Utils.md5HashOfString(stringToHash);

                        LicenseActivationTask lat = new LicenseActivationTask(EngageApplication.this, url, getString(R.string.licensing_entitlement), key, ac, deviceId, hValue, EngageApplication.this);

                        lat.execute();
                    }
                    catch (Exception e)
                    {
                        Log.e(TAG, "obtainActivationCode: " + e.getMessage());
                        scheduleObtainingActivationCode();
                    }
                }
            }
        });
    }

    @Override
    public void onLicenseActivationTaskComplete(final int result, final String activationCode, final String resultMessage)
    {
        runOnUiThread(new Runnable()
        {
            @Override
            public void run()
            {
                boolean needScheduling = false;

                if(_licenseActivationPaused)
                {
                    Log.d(TAG, "license activation paused - rescheduling");
                    needScheduling = true;
                }
                else
                {
                    if (result == 0 && !Utils.isEmptyString(activationCode))
                    {
                        String key = Globals.getSharedPreferences().getString(PreferenceKeys.USER_LICENSING_KEY, null);
                        if (!Utils.isEmptyString(key))
                        {
                            Log.i(TAG, "onLicenseActivationTaskComplete: attempt succeeded");

                            String ac = Globals.getSharedPreferences().getString(PreferenceKeys.USER_LICENSING_ACTIVATION_CODE, "");
                            if (ac.compareTo(activationCode) == 0)
                            {
                                Log.w(TAG, "onLicenseActivationTaskComplete: new activation code matches existing activation code");
                            }

                            Globals.getSharedPreferencesEditor().putString(PreferenceKeys.USER_LICENSING_ACTIVATION_CODE, activationCode);
                            Globals.getSharedPreferencesEditor().apply();
                            getEngine().engageUpdateLicense(getString(R.string.licensing_entitlement), key, activationCode);
                        }
                        else
                        {
                            Log.e(TAG, "onLicenseActivationTaskComplete: no license key present");
                            needScheduling = true;
                        }
                    }
                    else
                    {
                        Log.e(TAG, "onLicenseActivationTaskComplete: attempting failed - " + resultMessage);
                        needScheduling = true;
                    }
                }

                if(needScheduling)
                {
                    scheduleObtainingActivationCode();
                }
                else
                {
                    cancelObtainingActivationCode();
                }
            }
        });
    }
}
