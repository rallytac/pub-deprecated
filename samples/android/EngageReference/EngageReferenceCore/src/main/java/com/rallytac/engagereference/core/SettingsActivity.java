//
//  Copyright (c) 2019 Rally Tactical Systems, Inc.
//  All rights reserved.
//

package com.rallytac.engagereference.core;

import android.annotation.TargetApi;
import android.bluetooth.BluetoothDevice;
import android.content.Context;
import android.content.res.Configuration;
import android.os.Build;
import android.os.Bundle;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.PreferenceManager;
import android.preference.SwitchPreference;
import android.support.v7.app.ActionBar;
import android.util.Log;
import android.view.MenuItem;
import android.widget.Toast;


import java.net.NetworkInterface;
import java.util.ArrayList;

public class SettingsActivity extends AppCompatPreferenceActivity
{
    private static String TAG = SettingsActivity.class.getSimpleName();

    public static int MISSION_CHANGED_RESULT = (RESULT_FIRST_USER + 1);
    private static SettingsActivity _thisActivity;
    private static boolean _prefChangeIsBeingForcedByBinding = false;

    private void indicateMissionChanged()
    {
        ((EngageApplication) getApplication()).setMissionChangedStatus(true);
    }

    private static Preference.OnPreferenceChangeListener sBindPreferenceSummaryToValueListener = new Preference.OnPreferenceChangeListener()
    {
        @Override
        public boolean onPreferenceChange(Preference preference, Object value)
        {
            String stringValue = value.toString();

            //Log.e(TAG, ">>>>>> onPreferenceChange: " + preference.getKey());

            if(!_prefChangeIsBeingForcedByBinding)
            {
                String key = preference.getKey();
                if(key.startsWith("rallypoint_")
                    || key.startsWith("user_")
                    || key.startsWith("network_")
                    || key.startsWith("mission_"))
                {
                    Log.i(TAG, "mission parameters changed");
                    _thisActivity.indicateMissionChanged();
                }
            }

            if (preference instanceof ListPreference)
            {
                ListPreference listPreference = (ListPreference) preference;
                int index = listPreference.findIndexOfValue(stringValue);

                preference.setSummary(
                        index >= 0
                                ? listPreference.getEntries()[index]
                                : null);

            }
            else
            {
                if( !(preference instanceof SwitchPreference) )
                {
                    preference.setSummary(stringValue);
                }
            }
            return true;
        }
    };

    private static boolean isXLargeTablet(Context context)
    {
        return (context.getResources().getConfiguration().screenLayout
                & Configuration.SCREENLAYOUT_SIZE_MASK) >= Configuration.SCREENLAYOUT_SIZE_XLARGE;
    }

    private static void bindPreferenceSummaryToValue(Preference preference)
    {
        preference.setOnPreferenceChangeListener(sBindPreferenceSummaryToValueListener);

        _prefChangeIsBeingForcedByBinding = true;

        if(!(preference instanceof SwitchPreference))
        {
            String strVal;

            String className = preference.getClass().toString();

            if(className.contains("SeekBarPreference"))
            {
                strVal = Integer.toString(PreferenceManager
                                    .getDefaultSharedPreferences(preference.getContext())
                                    .getInt(preference.getKey(), 0));
            }
            else
            {
                strVal = PreferenceManager
                            .getDefaultSharedPreferences(preference.getContext())
                            .getString(preference.getKey(), "");
            }

            strVal = Utils.trimString(strVal);

            sBindPreferenceSummaryToValueListener.onPreferenceChange(preference, strVal);
        }

        _prefChangeIsBeingForcedByBinding = false;
    }


    @Override
    public boolean onOptionsItemSelected(MenuItem item)
    {
        int id = item.getItemId();
        if (id == android.R.id.home)
        {
            onBackPressed();
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        _thisActivity = this;
        super.onCreate(savedInstanceState);
        setupActionBar();

        getFragmentManager().beginTransaction()
                .replace(android.R.id.content, new GeneralPreferenceFragment()).commit();
    }

    @Override
    public void onBackPressed()
    {
        super.onBackPressed();
    }

    @Override
    protected void onPause()
    {
        super.onPause();
    }

    @Override
    protected void onStop()
    {
        super.onStop();
    }

    @Override
    protected void onDestroy()
    {
        super.onDestroy();
    }

    private void setupActionBar()
    {
        ActionBar actionBar = getSupportActionBar();
        if (actionBar != null)
        {
            actionBar.setDisplayHomeAsUpEnabled(true);
        }
    }

    @Override
    public boolean onIsMultiPane()
    {
        return isXLargeTablet(this);
    }

    protected boolean isValidFragment(String fragmentName)
    {
        return PreferenceFragment.class.getName().equals(fragmentName)
                || GeneralPreferenceFragment.class.getName().equals(fragmentName);
    }


    @TargetApi(Build.VERSION_CODES.HONEYCOMB)
    public static class BasePreferenceFragment extends PreferenceFragment
    {
        @Override
        public void onCreate(Bundle savedInstanceState)
        {
            super.onCreate(savedInstanceState);
            setHasOptionsMenu(true);
        }

        @Override
        public boolean onOptionsItemSelected(MenuItem item)
        {
            int id = item.getItemId();
            if (id == android.R.id.home)
            {
                getActivity().onBackPressed();
                return true;
            }
            return super.onOptionsItemSelected(item);
        }
    }

    @TargetApi(Build.VERSION_CODES.HONEYCOMB)
    public static class GeneralPreferenceFragment extends BasePreferenceFragment
    {
        @Override
        public void onCreate(Bundle savedInstanceState)
        {
            super.onCreate(savedInstanceState);
            addPreferencesFromResource(R.xml.pref_general);

            bindPreferenceSummaryToValue(findPreference(PreferenceKeys.USER_ID));
            bindPreferenceSummaryToValue(findPreference(PreferenceKeys.USER_DISPLAY_NAME));
            bindPreferenceSummaryToValue(findPreference(PreferenceKeys.USER_ALIAS_ID));
            bindPreferenceSummaryToValue(findPreference(PreferenceKeys.USER_TONE_LEVEL_PTT));
            bindPreferenceSummaryToValue(findPreference(PreferenceKeys.USER_TONE_LEVEL_NOTIFICATION));
            bindPreferenceSummaryToValue(findPreference(PreferenceKeys.USER_TONE_LEVEL_ERROR));
            bindPreferenceSummaryToValue(findPreference(PreferenceKeys.USER_LOCATION_SHARED));
            bindPreferenceSummaryToValue(findPreference(PreferenceKeys.USER_LOCATION_ACCURACY));
            bindPreferenceSummaryToValue(findPreference(PreferenceKeys.USER_LOCATION_INTERVAL_SECS));
            bindPreferenceSummaryToValue(findPreference(PreferenceKeys.USER_LOCATION_MIN_DISPLACEMENT));
            bindPreferenceSummaryToValue(findPreference(PreferenceKeys.USER_NOTIFY_NODE_JOIN));
            bindPreferenceSummaryToValue(findPreference(PreferenceKeys.USER_NOTIFY_NODE_LEAVE));
            bindPreferenceSummaryToValue(findPreference(PreferenceKeys.USER_NOTIFY_NEW_AUDIO_RX));
            bindPreferenceSummaryToValue(findPreference(PreferenceKeys.USER_NOTIFY_NETWORK_ERROR));
            bindPreferenceSummaryToValue(findPreference(PreferenceKeys.USER_SPEAKER_OUTPUT_BOOST_FACTOR));
            bindPreferenceSummaryToValue(findPreference(PreferenceKeys.USER_NOTIFY_VIBRATIONS));
            bindPreferenceSummaryToValue(findPreference(PreferenceKeys.USER_NOTIFY_PTT_EVERY_TIME));

            {
                final ListPreference listPreference = (ListPreference) findPreference(PreferenceKeys.NETWORK_BINDING_NIC_NAME);
                ArrayList<NetworkInterface> ifs = Utils.getNetworkInterfaces();
                final CharSequence[] entries = new CharSequence[ifs.size()];
                final CharSequence[] entryValues = new CharSequence[ifs.size()];
                int index = 0;
                for (NetworkInterface nif : ifs)
                {
                    entries[index] = nif.getDisplayName();
                    entryValues[index] = nif.getName();
                    index++;
                }
                listPreference.setEntries(entries);
                listPreference.setEntryValues(entryValues);
                bindPreferenceSummaryToValue(findPreference(PreferenceKeys.NETWORK_BINDING_NIC_NAME));
            }

            {
                final ListPreference listPreference = (ListPreference) findPreference(PreferenceKeys.USER_BT_DEVICE_ADDRESS);
                ArrayList<BluetoothDevice> btDevs = BluetoothManager.getDevices();
                final CharSequence[] entries = new CharSequence[btDevs.size()];
                final CharSequence[] entryValues = new CharSequence[btDevs.size()];
                int index = 0;
                for (BluetoothDevice dev : btDevs)
                {
                    entries[index] = dev.getName();
                    entryValues[index] = dev.getAddress();
                    index++;
                }
                listPreference.setEntries(entries);
                listPreference.setEntryValues(entryValues);
                bindPreferenceSummaryToValue(findPreference(PreferenceKeys.USER_BT_DEVICE_ADDRESS));
            }

            bindPreferenceSummaryToValue(findPreference(PreferenceKeys.USER_EXPERIMENT_ENABLE_SSDP_DISCOVERY));
            bindPreferenceSummaryToValue(findPreference(PreferenceKeys.USER_EXPERIMENT_ENABLE_CISTECH_GV1_DISCOVERY));
            bindPreferenceSummaryToValue(findPreference(PreferenceKeys.USER_EXPERIMENT_CISTECH_GV1_DISCOVERY_ADDRESS));
            bindPreferenceSummaryToValue(findPreference(PreferenceKeys.USER_EXPERIMENT_CISTECH_GV1_DISCOVERY_PORT));
            bindPreferenceSummaryToValue(findPreference(PreferenceKeys.USER_EXPERIMENT_CISTECH_GV1_DISCOVERY_TIMEOUT_SECS));

            bindPreferenceSummaryToValue(findPreference(PreferenceKeys.USER_EXPERIMENT_ENABLE_TRELLISWARE_DISCOVERY));

            bindPreferenceSummaryToValue(findPreference(PreferenceKeys.USER_EXPERIMENT_ENABLE_DEVICE_REPORT_CONNECTIVITY));
            bindPreferenceSummaryToValue(findPreference(PreferenceKeys.USER_EXPERIMENT_ENABLE_DEVICE_REPORT_POWER));

            bindPreferenceSummaryToValue(findPreference(PreferenceKeys.USER_EXPERIMENT_ENABLE_HBM));
            bindPreferenceSummaryToValue(findPreference(PreferenceKeys.USER_EXPERIMENT_HBM_INTERVAL_SECS));
            bindPreferenceSummaryToValue(findPreference(PreferenceKeys.USER_EXPERIMENT_HBM_ENABLE_HEART_RATE));
            bindPreferenceSummaryToValue(findPreference(PreferenceKeys.USER_EXPERIMENT_HBM_ENABLE_SKIN_TEMP));
            bindPreferenceSummaryToValue(findPreference(PreferenceKeys.USER_EXPERIMENT_HBM_ENABLE_CORE_TEMP));
            bindPreferenceSummaryToValue(findPreference(PreferenceKeys.USER_EXPERIMENT_HBM_ENABLE_BLOOD_OXY));
            bindPreferenceSummaryToValue(findPreference(PreferenceKeys.USER_EXPERIMENT_HBM_ENABLE_BLOOD_HYDRO));
            bindPreferenceSummaryToValue(findPreference(PreferenceKeys.USER_EXPERIMENT_HBM_ENABLE_FATIGUE_LEVEL));
            bindPreferenceSummaryToValue(findPreference(PreferenceKeys.USER_EXPERIMENT_HBM_ENABLE_TASK_EFFECTIVENESS_LEVEL));
        }
    }
}
