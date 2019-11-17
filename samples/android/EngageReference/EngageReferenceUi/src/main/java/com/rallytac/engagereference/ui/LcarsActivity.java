//
//  Copyright (c) 2019 Rally Tactical Systems, Inc.
//  All rights reserved.
//

package com.rallytac.engagereference.ui;

import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.ActivityInfo;
import android.media.SoundPool;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentTransaction;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.support.v7.app.AppCompatDelegate;
import android.support.v7.widget.PopupMenu;
import android.text.SpannableString;
import android.text.method.LinkMovementMethod;
import android.util.Log;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.widget.ArrayAdapter;
import android.widget.CompoundButton;
import android.widget.ImageView;
import android.widget.SeekBar;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import com.rallytac.engage.engine.Engine;
import com.rallytac.engagereference.core.AboutActivity;
import com.rallytac.engagereference.core.ActiveConfiguration;
import com.rallytac.engagereference.core.Constants;
import com.rallytac.engagereference.core.ContactActivity;
import com.rallytac.engagereference.core.EngageApplication;
import com.rallytac.engagereference.core.Globals;
import com.rallytac.engagereference.core.GroupDescriptor;
import com.rallytac.engagereference.core.MapActivity;
import com.rallytac.engagereference.core.MissionListActivity;
import com.rallytac.engagereference.core.PreferenceKeys;
import com.rallytac.engagereference.core.PresenceDescriptor;
import com.rallytac.engagereference.core.SettingsActivity;
import com.rallytac.engagereference.core.ShareMissionActivity;
import com.rallytac.engagereference.core.SimpleMessageException;
import com.rallytac.engagereference.core.UserNodeViewActivity;
import com.rallytac.engagereference.core.Utils;
import com.rallytac.engagereference.core.VolumeLevels;
import com.rallytac.engagereference.ui.uifragments.CardFragment;

import org.json.JSONArray;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.Calendar;
import java.util.List;
import java.util.Timer;
import java.util.TimerTask;

public class LcarsActivity
                            extends
                                AppCompatActivity

                            implements
                                EngageApplication.IUiUpdateListener,
                                EngageApplication.IAssetChangeListener,
                                EngageApplication.IConfigurationChangeListener,
                                EngageApplication.ILicenseChangeListener,
                                EngageApplication.IGroupTimelineListener
{
    private static String TAG = LcarsActivity.class.getSimpleName();

    private static int SETTINGS_REQUEST_CODE = 42;
    private static int MISSION_LISTING_REQUEST_CODE = 43;
    private ActiveConfiguration _ac = null;
    private Timer _waitForEngineStartedTimer = null;
    private boolean _anyTxActive = false;
    private boolean _anyTxPending = false;
    private Animation _notificationBarAnimation = null;
    private Runnable _actionOnNotificationBarClick = null;
    private boolean _pttRequested = false;

    private Animation _licensingBarAnimation = null;
    private Runnable _actionOnLicensingBarClick = null;

    private Animation _humanBiometricsAnimation = null;
    private Runnable _actionOnHumanBiometricsClick = null;

    private long _lastHeadsetKeyhookDown = 0;

    private VolumeLevels _vlSaved;
    private VolumeLevels _vlInProgress;
    private boolean _volumeSynced = true;
    private SeekBar _volumeSliderLastMoved = null;

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        Log.d(TAG, "onCreate");

        AppCompatDelegate.setCompatVectorFromResourcesEnabled(true);

        _ac = Globals.getEngageApplication().getActiveConfiguration();

        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);

        super.onCreate(savedInstanceState);

        if(_ac.getUiMode() == Constants.UiMode.vSingle)
        {
            setContentView(R.layout.activity_main_single);
        }
        else if(_ac.getUiMode() == Constants.UiMode.vMulti)
        {
            setContentView(R.layout.activity_main_multi);
        }

        String title;

        title = _ac.getMissionName();
        if(_ac.getUseRp())
        {
            title += " @ " + _ac.getRpAddress() + ":" + _ac.getRpPort();
        }
        else
        {
            title += " @ MC";
        }

        title = title.toUpperCase();
        setTitle(title);

        restoreSavedState(savedInstanceState);

        assignGroupsToFragments();
        setupMainScreen();
        redrawPttButton();
    }

    @Override
    protected void onSaveInstanceState(Bundle outState)
    {
        Log.d(TAG, "onSaveInstanceState");
        saveState(outState);
        super.onSaveInstanceState(outState);
    }

    @Override
    protected void onStart()
    {
        Log.d(TAG, "onStart");
        super.onStart();
    }

    @Override
    protected void onResume()
    {
        Log.d(TAG, "onResume");
        super.onResume();
        registerWithApp();
        updateLicensingBar();
        updateBiometricsIconDisplay();
    }

    @Override
    protected void onPause()
    {
        Log.d(TAG, "onPause");
        super.onPause();
        cancelTimers();
        unregisterFromApp();
    }

    @Override
    protected void onStop()
    {
        Log.d(TAG, "onStop");
        cancelTimers();
        super.onStop();
    }

    @Override
    protected void onDestroy()
    {
        Log.d(TAG, "onDestroy");
        cancelTimers();
        super.onDestroy();
    }

    @Override
    public void onBackPressed()
    {
        toggleViewMode();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent intent)
    {
        if (requestCode == SETTINGS_REQUEST_CODE)
        {
            if(resultCode == SettingsActivity.MISSION_CHANGED_RESULT || Globals.getEngageApplication().getMissionChangedStatus())
            {
                Log.i(TAG, "============= mission has changed, recreating =======================");
                onMissionChanged();
            }
        }
        else if(requestCode == MISSION_LISTING_REQUEST_CODE)
        {
            if(intent != null && intent.hasExtra(Constants.MISSION_ACTIVATED_ID))
            {
                String activatedId = intent.getStringExtra(Constants.MISSION_ACTIVATED_ID);
                if(Globals.getEngageApplication().switchToMission(activatedId))
                {
                    ActiveConfiguration ac = Globals.getEngageApplication().updateActiveConfiguration();
                    Toast.makeText(this, "Activated " + ac.getMissionName(), Toast.LENGTH_SHORT).show();
                    onMissionChanged();
                }
            }
        }
        else if (requestCode == Globals.getEngageApplication().getQrCodeScannerRequestCode())
        {
            try
            {
                ActiveConfiguration ac = Globals.getEngageApplication().processScannedQrCodeResultIntent(requestCode, resultCode, intent);
                if(ac != null)
                {
                    Utils.showLongPopupMsg(LcarsActivity.this, "Loaded " + ac.getMissionName());
                    onMissionChanged();
                }
            }
            catch(SimpleMessageException sme)
            {
                Utils.showPopupMsg(LcarsActivity.this, sme.getMessage());
                sme.printStackTrace();
            }
            catch(Exception e)
            {
                Utils.showPopupMsg(LcarsActivity.this, e.getMessage());
                e.printStackTrace();
            }
        }
    }

    @Override
    public void onAttachedToWindow()
    {
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_SHOW_WHEN_LOCKED);
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event)
    {
        //Log.d(TAG, "---onKeyDown keyCode=" + keyCode + ", event=" + event.toString() + ", _lastHeadsetKeyhookDown=" + _lastHeadsetKeyhookDown);

        if( keyCode == KeyEvent.KEYCODE_HEADSETHOOK )
        {
            if(_lastHeadsetKeyhookDown == 0)
            {
                _lastHeadsetKeyhookDown = event.getDownTime();
            }
            else
            {
                long diffTime = (event.getDownTime() - _lastHeadsetKeyhookDown);
                if(diffTime <= 500)
                {
                    _lastHeadsetKeyhookDown = 0;

                    _pttRequested = !_pttRequested;

                    if(_pttRequested)
                    {
                        Log.d(TAG, "---onKeyDown requesting startTx due to media button double-push");
                        Globals.getEngageApplication().startTx(0, 0);
                    }
                    else
                    {
                        Log.d(TAG, "---onKeyDown requesting endTx due to media button double-push");
                        Globals.getEngageApplication().endTx();
                    }
                }
            }
        }

        /*
        // Handle repeat counts - for those headsets that can generate a long-push
        if(keyCode == KeyEvent.KEYCODE_HEADSETHOOK && event.getRepeatCount() > 2)
        {
            if(!_pttRequested)
            {
                Log.d(TAG, "---onKeyDown requesting startTx due to media button push");
                _pttRequested = true;
                Globals.getEngageApplication().startTx(0, 0);
            }
        }
        */

        return super.onKeyDown(keyCode, event);
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event)
    {
        /*
        long diffTime = (event.getEventTime() - event.getDownTime());
        Log.e(TAG, "---onKeyUp keyCode=" + keyCode + ", event=" + event.toString() + ", diff=" + diffTime);

        // Handle PTT indication via specialized key
        if(keyCode == KeyEvent.KEYCODE_HEADSETHOOK)
        {
            if (diffTime > 240 && diffTime < 260)
            {
                if (!_pttRequested)
                {
                    Log.d(TAG, "---onKeyUp requesting startTx due to media button PTT");
                    Globals.getEngageApplication().startTx(0, 0);
                }
                else
                {
                    Log.d(TAG, "---onKeyUp requesting endTx due to media button PTT");
                    Globals.getEngageApplication().endTx();
                }

                _pttRequested = !_pttRequested;
            }
            else
            {
                if (_pttRequested)
                {
                    Log.d(TAG, "---onKeyUp requesting endTx due to media button release");
                    _pttRequested = false;
                    Globals.getEngageApplication().endTx();
                }
            }
        }
        */

        return super.onKeyUp(keyCode, event);
    }

    private void showNotificationBar(final String msg)
    {
        runOnUiThread(new Runnable()
        {
            @Override
            public void run()
            {
                View v = findViewById(R.id.tvNotificationBar);
                if(v != null)
                {
                    ((TextView)v).setText(msg);
                    if(_notificationBarAnimation == null)
                    {
                        _notificationBarAnimation = AnimationUtils.loadAnimation(LcarsActivity.this, R.anim.notification_bar_pulse);
                        v.startAnimation(_notificationBarAnimation);
                        v.setVisibility(View.VISIBLE);
                        v.setOnClickListener(new View.OnClickListener()
                        {
                            @Override
                            public void onClick(View v)
                            {
                                hideNotificationBar();
                                executeActionOnNotificationBarClick();
                            }
                        });
                    }
                }
            }
        });
    }

    private void hideNotificationBar()
    {
        runOnUiThread(new Runnable()
        {
            @Override
            public void run()
            {
                if(_notificationBarAnimation != null)
                {
                    _notificationBarAnimation.cancel();
                    _notificationBarAnimation.reset();
                    _notificationBarAnimation = null;
                }

                View v = findViewById(R.id.tvNotificationBar);
                if(v != null)
                {
                    v.setVisibility(View.GONE);
                    v.clearAnimation();
                }
            }
        });
    }

    private void executeActionOnNotificationBarClick()
    {
        runOnUiThread(new Runnable()
        {
            @Override
            public void run()
            {
                if(_actionOnNotificationBarClick != null)
                {
                    _actionOnNotificationBarClick.run();
                }
            }
        });
    }


    private void showLicensingBar(final String msg)
    {
        runOnUiThread(new Runnable()
        {
            @Override
            public void run()
            {
                View v = findViewById(R.id.tvLicensingBar);
                if(v != null)
                {
                    ((TextView)v).setText(msg);
                    if(_licensingBarAnimation == null)
                    {
                        _actionOnLicensingBarClick = new Runnable()
                        {
                            @Override
                            public void run()
                            {
                                startAboutActivity();
                            }
                        };

                        _licensingBarAnimation = AnimationUtils.loadAnimation(LcarsActivity.this, R.anim.licensing_bar_pulse);
                        v.startAnimation(_licensingBarAnimation);
                        v.setVisibility(View.VISIBLE);
                        v.setOnClickListener(new View.OnClickListener()
                        {
                            @Override
                            public void onClick(View v)
                            {
                                hideLicensingBar();
                                executeActionOnLicensingBarClick();
                            }
                        });
                    }
                }
            }
        });
    }

    private void hideLicensingBar()
    {
        runOnUiThread(new Runnable()
        {
            @Override
            public void run()
            {
                if(_licensingBarAnimation != null)
                {
                    _licensingBarAnimation.cancel();
                    _licensingBarAnimation.reset();
                    _licensingBarAnimation = null;
                }

                View v = findViewById(R.id.tvLicensingBar);
                if(v != null)
                {
                    v.setVisibility(View.GONE);
                    v.clearAnimation();
                }
            }
        });
    }

    private void executeActionOnLicensingBarClick()
    {
        runOnUiThread(new Runnable()
        {
            @Override
            public void run()
            {
                if(_actionOnLicensingBarClick != null)
                {
                    _actionOnLicensingBarClick.run();
                }
            }
        });
    }

    private void showBiometricsReporting()
    {
        runOnUiThread(new Runnable()
        {
            @Override
            public void run()
            {
                View v = findViewById(R.id.ivHeart);
                if(v != null)
                {
                    if(_humanBiometricsAnimation == null)
                    {
                        _actionOnHumanBiometricsClick = new Runnable()
                        {
                            @Override
                            public void run()
                            {
                                startSettingsActivity();
                            }
                        };

                        _humanBiometricsAnimation = AnimationUtils.loadAnimation(LcarsActivity.this, R.anim.heart_pulse);
                        v.startAnimation(_humanBiometricsAnimation);
                        v.setVisibility(View.VISIBLE);
                        v.setOnClickListener(new View.OnClickListener()
                        {
                            @Override
                            public void onClick(View v)
                            {
                                executeActionOnBiometricsClick();
                            }
                        });
                    }
                }
            }
        });
    }

    private void hideBiometricsReporting()
    {
        runOnUiThread(new Runnable()
        {
            @Override
            public void run()
            {
                if(_humanBiometricsAnimation != null)
                {
                    _humanBiometricsAnimation.cancel();
                    _humanBiometricsAnimation.reset();
                    _humanBiometricsAnimation = null;
                }

                View v = findViewById(R.id.ivHeart);
                if(v != null)
                {
                    v.setVisibility(View.GONE);
                    v.clearAnimation();
                }
            }
        });
    }

    private void executeActionOnBiometricsClick()
    {
        runOnUiThread(new Runnable()
        {
            @Override
            public void run()
            {
                if(_actionOnHumanBiometricsClick != null)
                {
                    _actionOnHumanBiometricsClick.run();
                }
            }
        });
    }

    @Override
    public void onMissionChanged()
    {
        runOnUiThread(new Runnable()
            {
                @Override
                public void run()
                {
                    recreateWhenEngineIsRestarted();
                }
            });
    }

    @Override
    public void onCriticalConfigurationChange()
    {
        _actionOnNotificationBarClick = new Runnable()
        {
            @Override
            public void run()
            {
                doRecreate();
            }
        };

        showNotificationBar("Configuration change detected - Tap to activate");
    }

    @Override
    public void onAssetDiscovered(String id, String json)
    {
        try
        {
            JSONObject j = new JSONObject(json);
            String msg = "Talk group '" + j.getString(Engine.JsonFields.Group.name) + "' discovered!";
            Toast.makeText(this, msg, Toast.LENGTH_LONG).show();
        }
        catch (Exception e)
        {
        }
    }

    @Override
    public void onAssetRediscovered(String id, String json)
    {
        // TODO: Do we want to do anything here?
    }

    @Override
    public void onAssetUndiscovered(String id, String json)
    {
        try
        {
            JSONObject j = new JSONObject(json);
            String msg = "Talk group '" + j.getString(Engine.JsonFields.Group.name) + "' lost!";
            Toast.makeText(this, msg, Toast.LENGTH_LONG).show();
        }
        catch (Exception e)
        {
        }
    }

    @Override
    public void onAnyTxPending()
    {
        Log.d(TAG, "onAnyTxPending");
        _anyTxPending = true;
        redrawPttButton();
        redrawCardFragments();
    }

    @Override
    public void onAnyTxActive()
    {
        Log.d(TAG, "onAnyTxActive");
        _anyTxActive = true;
        _anyTxPending = true;
        redrawPttButton();
        redrawCardFragments();
    }

    @Override
    public void onAnyTxEnding()
    {
        Log.d(TAG, "onAnyTxEnding");
        // Nothing to do here
    }

    @Override
    public void onAllTxEnded()
    {
        Log.d(TAG, "onAllTxEnded");
        _anyTxActive = false;
        _anyTxPending = false;
        _pttRequested = false;
        _lastHeadsetKeyhookDown = 0;
        redrawPttButton();
        redrawCardFragments();
    }

    @Override
    public void onGroupUiRefreshNeeded(GroupDescriptor gd)
    {
        updateFragmentForGroup(gd.id);
    }

    @Override
    public void onGroupTxUsurped(GroupDescriptor gd)
    {
        runOnUiThread(new Runnable()
        {
            @Override
            public void run()
            {
                Toast.makeText(LcarsActivity.this, "Transmit usurped by higher priority", Toast.LENGTH_SHORT).show();
            }
        });
    }

    @Override
    public void onGroupMaxTxTimeExceeded(GroupDescriptor gd)
    {
        runOnUiThread(new Runnable()
        {
            @Override
            public void run()
            {
                Toast.makeText(LcarsActivity.this, "Maximum transmit time exceeded", Toast.LENGTH_SHORT).show();
            }
        });
    }

    @Override
    public void onLicenseChanged()
    {
        updateLicensingBar();
    }

    @Override
    public void onLicenseExpired()
    {
        updateLicensingBar();
    }

    @Override
    public void onLicenseExpiring(long secondsLeft)
    {
        updateLicensingBar();
    }

    private void updateLicensingBar()
    {
        if(Globals.getEngageApplication().getLicenseExpired())
        {
            showLicensingBar(getString(R.string.license_has_expired));
        }
        else
        {
            long secondsLeft = Globals.getEngageApplication().getLicenseSecondsLeft();

            if(secondsLeft > 0)
            {
                // Only show this if our license is going to expire within 10 days
                if (secondsLeft <= (86400 * 10))
                {
                    Calendar now = Calendar.getInstance();
                    Calendar exp = Calendar.getInstance();
                    exp.add(Calendar.SECOND, (int) secondsLeft);
                    String expMsg = Utils.formattedTimespan(now.getTimeInMillis(), exp.getTimeInMillis());
                    showLicensingBar("License expires " + expMsg);
                }
                else
                {
                    hideLicensingBar();
                }
            }
            else
            {
                showLicensingBar(getString(R.string.license_has_expired));
            }
        }
    }

    private void updateBiometricsIconDisplay()
    {
        if(Globals.getSharedPreferences().getBoolean(PreferenceKeys.USER_EXPERIMENT_ENABLE_HBM, false))
        {
            showBiometricsReporting();
        }
        else
        {
            hideBiometricsReporting();
        }
    }

    private void registerWithApp()
    {
        Globals.getEngageApplication().addUiUpdateListener(this);
        Globals.getEngageApplication().addAssetChangeListener(this);
        Globals.getEngageApplication().addConfigurationChangeListener(this);
        Globals.getEngageApplication().addLicenseChangeListener(this);
        Globals.getEngageApplication().addGroupTimelineListener(this);
    }

    private void unregisterFromApp()
    {
        Globals.getEngageApplication().removeUiUpdateListener(this);
        Globals.getEngageApplication().removeAssetChangeListener(this);
        Globals.getEngageApplication().removeConfigurationChangeListener(this);
        Globals.getEngageApplication().removeLicenseChangeListener(this);
        Globals.getEngageApplication().removeGroupTimelineListener(this);
    }

    private void saveState(Bundle bundle)
    {
        Log.d(TAG, "saveState");
    }

    private void restoreSavedState(Bundle bundle)
    {
        Log.d(TAG, "restoreSavedState");
    }

    private void performDevSimulation()
    {
        Toast.makeText(this, "DEVELOPER!! - __devOnly__RunTest", Toast.LENGTH_LONG).show();
        Globals.getEngageApplication().__devOnly__RunTest();
    }

    @Override
    public void onGroupTimelineEventStarted(final GroupDescriptor gd, final String eventJson)
    {
        runOnUiThread(new Runnable()
        {
            @Override
            public void run()
            {
                //Toast.makeText(LcarsActivity.this, "TODO: Event started for " + gd.name, Toast.LENGTH_SHORT).show();
            }
        });
    }

    @Override
    public void onGroupTimelineEventUpdated(final GroupDescriptor gd, final String eventJson)
    {
        runOnUiThread(new Runnable()
        {
            @Override
            public void run()
            {
                //Toast.makeText(LcarsActivity.this, "TODO: Event updated for " + gd.name, Toast.LENGTH_SHORT).show();
            }
        });
    }

    @Override
    public void onGroupTimelineEventEnded(final GroupDescriptor gd, final String eventJson)
    {
        runOnUiThread(new Runnable()
        {
            @Override
            public void run()
            {
                //Toast.makeText(LcarsActivity.this, "TODO: Event ended for " + gd.name, Toast.LENGTH_SHORT).show();
            }
        });
    }

    SoundPool _soundpool = null;

    private class TimelineEvent
    {
        public int typeIcon;
        public String sourceEntity;
        public String audioUri;
        public long started;
        public long ended;
        public int audioId = -1;
        public long audioLengthMs = 0;
        public View view = null;
    }

    void stopSoundpoolPlayer()
    {
        if(_soundpool != null)
        {
            _soundpool.release();
            _soundpool = null;
        }
    }

    void playWaveFileForEvent(final TimelineEvent event)
    {
        runOnUiThread(new Runnable()
        {
            @Override
            public void run()
            {
                if(_soundpool == null)
                {
                    _soundpool = new SoundPool.Builder().build();
                    _soundpool.setOnLoadCompleteListener(new SoundPool.OnLoadCompleteListener()
                    {
                        @Override
                        public void onLoadComplete(SoundPool soundPool, int sampleId, int status)
                        {
                            if(sampleId > 0)
                            {
                                _soundpool.play(sampleId, 1.0f, 1.0f, 1, 0, 1);
                            }
                        }
                    });
                }

                if(event.audioId == -1)
                {
                    String path = event.audioUri.substring(7);
                    event.audioId = _soundpool.load(path, 1);
                }
                else
                {
                    if (event.audioId > 0)
                    {
                        _soundpool.play(event.audioId, 1.0f, 1.0f, 1, 0, 1);
                    }
                    else
                    {
                        Toast.makeText(LcarsActivity.this, "Cannot play " + event.audioUri, Toast.LENGTH_SHORT).show();
                    }
                }
            }
        });
    }

    private class TimelineEventListAdapter extends ArrayAdapter<TimelineEvent>
    {
        private Context _ctx;
        private int _resId;

        public TimelineEventListAdapter(Context ctx, int resId, ArrayList<TimelineEvent> list)
        {
            super(ctx, resId, list);
            _ctx = ctx;
            _resId = resId;
        }

        @NonNull
        @Override
        public View getView(int position, @Nullable View convertView, @NonNull ViewGroup parent)
        {
            LayoutInflater inflator = LayoutInflater.from(_ctx);
            convertView = inflator.inflate(_resId, parent, false);

            final TimelineEvent item = getItem(position);
            item.view = convertView;

            ((ImageView)convertView.findViewById(R.id.ivEventType)).setImageDrawable(ContextCompat.getDrawable(_ctx, item.typeIcon));
            ((TextView)convertView.findViewById(R.id.tvSourceEntity)).setText(item.sourceEntity);
            ((TextView)convertView.findViewById(R.id.tvAudioLengthMs)).setText(Float.toString(item.audioLengthMs / 1000) + " secs");

            String extraInfo;

            if(item.started > 0)
            {
                extraInfo = Utils.javaDateFromUnixMilliseconds(item.started).toString();
            }
            else
            {
                extraInfo = "";
            }

            ((TextView)convertView.findViewById(R.id.tvExtraInformation)).setText(extraInfo);

            convertView.setOnClickListener(new View.OnClickListener()
            {
                @Override
                public void onClick(View v)
                {
                    playWaveFileForEvent(item);
                }
            });

            return convertView;
        }
    }


    @Override
    public void onGroupTimelineReport(final GroupDescriptor gd, final String reportJson)
    {
        runOnUiThread(new Runnable()
        {
            @Override
            public void run()
            {
                ArrayList<TimelineEvent> events = null;

                if(!Utils.isEmptyString(reportJson))
                {
                    try
                    {
                        JSONObject root = new JSONObject(reportJson);
                        JSONArray list = root.getJSONArray(Engine.JsonFields.TimelineReport.events);
                        if(list != null && list.length() > 0)
                        {
                            for(int x = 0; x < list.length(); x++)
                            {
                                JSONObject obj = list.getJSONObject(x);
                                TimelineEvent te = new TimelineEvent();

                                int dir = obj.getInt(Engine.JsonFields.TimelineEvent.direction);

                                if(dir == 1)
                                {
                                    te.typeIcon = com.rallytac.engagereference.core.R.drawable.ic_event_receive;
                                }
                                else if(dir == 2)
                                {
                                    te.typeIcon = com.rallytac.engagereference.core.R.drawable.ic_event_transmit;
                                }
                                else
                                {
                                    te.typeIcon = com.rallytac.engagereference.core.R.drawable.ic_event_type_error;
                                }

                                te.started = obj.getLong(Engine.JsonFields.TimelineEvent.started);
                                te.ended = obj.optLong(Engine.JsonFields.TimelineEvent.ended, 0);
                                te.sourceEntity = obj.optString(Engine.JsonFields.TimelineEvent.alias);
                                te.audioUri = obj.optString(Engine.JsonFields.TimelineEvent.uri);

                                JSONObject audio = obj.optJSONObject(Engine.JsonFields.TimelineEvent.Audio.objectName);
                                if(audio != null)
                                {
                                    te.audioLengthMs = audio.getLong(Engine.JsonFields.TimelineEvent.Audio.ms);
                                }

                                if(events == null)
                                {
                                    events = new ArrayList<>();
                                }

                                events.add(te);
                            }
                        }
                    }
                    catch(Exception e)
                    {
                        if(events != null)
                        {
                            events.clear();
                            events = null;
                        }

                        e.printStackTrace();
                    }
                }

                if(events == null)
                {
                    Toast.makeText(LcarsActivity.this, "No events found in timeline report", Toast.LENGTH_SHORT).show();
                }
                else
                {
                    AlertDialog.Builder builder = new AlertDialog.Builder(LcarsActivity.this);

                    final TimelineEventListAdapter arrayAdapter = new TimelineEventListAdapter(LcarsActivity.this, com.rallytac.engagereference.core.R.layout.timeline_event_list_entry, events);

                    builder.setAdapter(arrayAdapter, null);
                    builder.setPositiveButton(com.rallytac.engagereference.core.R.string.button_close, new DialogInterface.OnClickListener()
                    {
                        @Override
                        public void onClick(DialogInterface dialog, int which)
                        {
                            LcarsActivity.this.stopSoundpoolPlayer();
                        }
                    });

                    builder.show();
                }
            }
        });
    }

    @Override
    public void onGroupTimelineReportFailed(final GroupDescriptor gd)
    {
        runOnUiThread(new Runnable()
        {
            @Override
            public void run()
            {
                Toast.makeText(LcarsActivity.this, "TODO: Failed to obtain a timeline report for " + gd.name, Toast.LENGTH_SHORT).show();
            }
        });
    }

    private class TeamListAdapter extends ArrayAdapter<PresenceDescriptor>
    {
        private Context _ctx;
        private int _resId;

        public TeamListAdapter(Context ctx, int resId, ArrayList<PresenceDescriptor> list)
        {
            super(ctx, resId, list);
            _ctx = ctx;
            _resId = resId;
        }

        @NonNull
        @Override
        public View getView(int position, @Nullable View convertView, @NonNull ViewGroup parent)
        {
            LayoutInflater inflator = LayoutInflater.from(_ctx);
            convertView = inflator.inflate(_resId, parent, false);

            final PresenceDescriptor item = getItem(position);

            ImageView iv = convertView.findViewById(com.rallytac.engagereference.core.R.id.ivType);
            iv.setImageDrawable(ContextCompat.getDrawable(_ctx, com.rallytac.engagereference.core.R.drawable.ic_engage_logo));

            String displayName = item.displayName;
            if(Utils.isEmptyString(displayName))
            {
                displayName = item.userId;
                if(Utils.isEmptyString(displayName))
                {
                    displayName = item.nodeId;
                }
            }

            ((TextView)convertView.findViewById(com.rallytac.engagereference.core.R.id.tvDisplayName)).setText(displayName);

            convertView.setOnClickListener(new View.OnClickListener()
            {
                @Override
                public void onClick(View v)
                {
                    Intent intent = new Intent(_ctx, UserNodeViewActivity.class);
                    intent.putExtra(UserNodeViewActivity.EXTRA_NODE_ID, item.nodeId);
                    startActivity(intent);
                }
            });

            return convertView;
        }
    }

    public void showTeamList()
    {
        if(_ac == null || _ac.getMissionNodeCount() == 0)
        {
            Utils.showPopupMsg(this, getString(com.rallytac.engagereference.core.R.string.no_team_members_present));
            return;
        }

        AlertDialog.Builder builder = new AlertDialog.Builder(this);

        final ArrayList<PresenceDescriptor> theList = _ac.getMissionNodes();

        final TeamListAdapter arrayAdapter = new TeamListAdapter(this, com.rallytac.engagereference.core.R.layout.team_list_row_item, theList);

        builder.setAdapter(arrayAdapter, null);
        builder.setPositiveButton(com.rallytac.engagereference.core.R.string.button_close, null);

        builder.show();
    }

    private void startDevTestActivity()
    {
        Intent intent = new Intent(this, DeveloperTestActivity.class);
        startActivity(intent);
    }

    private void startContactActivity()
    {
        Intent intent = new Intent(this, ContactActivity.class);
        startActivity(intent);
    }

    private void startAboutActivity()
    {
        Intent intent = new Intent(this, AboutActivity.class);
        startActivity(intent);
    }

    private void startMissionListActivity()
    {
        Intent intent = new Intent(this, MissionListActivity.class);
        startActivityForResult(intent, MISSION_LISTING_REQUEST_CODE);
    }

    private void startSettingsActivity()
    {
        Intent intent = new Intent(this, SettingsActivity.class);
        startActivityForResult(intent, SETTINGS_REQUEST_CODE);
    }

    private void startShareMissionActivity()
    {
        Intent intent = new Intent(this, ShareMissionActivity.class);
        startActivity(intent);
    }

    private void startMapActivity()
    {
        Intent intent = new Intent(this, MapActivity.class);
        startActivity(intent);
    }

    private void requestGroupTimeline(String groupId)
    {
        if(!Utils.isEmptyString(groupId))
        {
            try
            {
                JSONObject obj = new JSONObject();

                obj.put(Engine.JsonFields.TimelineQuery.maxCount, 10);
                obj.put(Engine.JsonFields.TimelineQuery.mostRecentFirst, true);

                Globals.getEngageApplication().getEngine().engageQueryGroupTimeline(groupId, obj.toString());
            }
            catch(Exception e)
            {
                e.printStackTrace();
                Toast.makeText(this, "Error constructing timeline query", Toast.LENGTH_SHORT).show();
            }
        }
    }

    private void cancelTimers()
    {
        if(_waitForEngineStartedTimer != null)
        {
            _waitForEngineStartedTimer.cancel();
            _waitForEngineStartedTimer = null;
        }
    }

    private void recreateWhenEngineIsRestarted()
    {
        Globals.getEngageApplication().restartEngine();

        _waitForEngineStartedTimer = new Timer();
        _waitForEngineStartedTimer.scheduleAtFixedRate(new TimerTask()
        {
            @Override
            public void run()
            {
                if(Globals.getEngageApplication().isEngineRunning())
                {
                    Log.i(TAG, "engine is running, proceeding");
                    _waitForEngineStartedTimer.cancel();

                    runOnUiThread(new Runnable()
                    {
                        @Override
                        public void run()
                        {
                            doRecreate();
                        }
                    });
                }
                else
                {
                    Log.i(TAG, "waiting for engage engine to restart");
                }
            }
        }, 0, 100);
    }

    private void doRecreate()
    {
        removeAllFragments();
        Globals.getEngageApplication().leaveAllGroups();
        Globals.getEngageApplication().updateActiveConfiguration();
        Globals.getEngageApplication().joinSelectedGroups();
        recreate();
    }

    public void toggleViewMode()
    {
        if(_ac.getUiMode() == Constants.UiMode.vSingle)
        {
            showMultiView();
        }
        else
        {
            showSingleView(null);
        }
    }

    public void showSingleView(String groupId)
    {
        if(!Utils.isEmptyString(groupId))
        {
            Globals.getSharedPreferencesEditor().putString(PreferenceKeys.ACTIVE_MISSION_CONFIGURATION_SELECTED_GROUPS_SINGLE, groupId);
            Globals.getSharedPreferencesEditor().apply();
        }

        _ac.setUiMode(Constants.UiMode.vSingle);
        runOnUiThread(new Runnable()
        {
            @Override
            public void run()
            {
                doRecreate();
            }
        });
    }

    public void showMultiView()
    {
        _ac.setUiMode(Constants.UiMode.vMulti);
        runOnUiThread(new Runnable()
        {
            @Override
            public void run()
            {
                doRecreate();
            }
        });
    }

    private void assignGroupsToFragments()
    {
        FragmentManager fragMan = getSupportFragmentManager();
        List<Fragment> fragments = fragMan.getFragments();

        if(_ac.getUiMode() == Constants.UiMode.vSingle)
        {
            boolean gotOne = false;
            for(GroupDescriptor gd : _ac.getMissionGroups())
            {
                if(gd.selectedForSingleView && gd.type == GroupDescriptor.Type.gtAudio )
                {
                    gotOne = true;
                    break;
                }
            }

            if(!gotOne)
            {
                for(GroupDescriptor gd : _ac.getMissionGroups())
                {
                    if(gd.type == GroupDescriptor.Type.gtAudio )
                    {
                        gd.selectedForSingleView = true;
                        break;
                    }
                }
            }
        }

        if(_ac.getUiMode() == Constants.UiMode.vSingle)
        {
            for(GroupDescriptor gd : _ac.getMissionGroups())
            {
                if(gd.selectedForSingleView && gd.type == GroupDescriptor.Type.gtAudio )
                {
                    for(Fragment f : fragments)
                    {
                        if(f instanceof CardFragment)
                        {
                            ((CardFragment)f).setGroupDescriptor(gd);

                            Globals.getSharedPreferencesEditor().putString(PreferenceKeys.ACTIVE_MISSION_CONFIGURATION_SELECTED_GROUPS_SINGLE, gd.id);
                            Globals.getSharedPreferencesEditor().apply();

                            break;
                        }
                    }

                    break;
                }
            }
        }
        else if(_ac.getUiMode() == Constants.UiMode.vMulti)
        {
            for(GroupDescriptor gd : _ac.getMissionGroups())
            {
                if(gd.selectedForMultiView && gd.type == GroupDescriptor.Type.gtAudio)
                {
                    for(Fragment f : fragments)
                    {
                        if(f instanceof CardFragment)
                        {
                            if(((CardFragment)f).getGroupDescriptor() == null)
                            {
                                ((CardFragment)f).setGroupDescriptor(gd);
                                break;
                            }
                        }
                    }
                }
            }
        }

        ArrayList<Fragment> trash = new ArrayList<>();

        // Now, go through the fragments and hide those that have no group
        for(Fragment f : fragments)
        {
            if(f instanceof CardFragment)
            {
                if(((CardFragment)f).getGroupDescriptor() == null)
                {
                    trash.add(f);
                }
            }
        }

        for(Fragment f : trash)
        {
            int id = f.getId();
            FragmentTransaction ft = fragMan.beginTransaction();
            ft.remove(f);
            ft.commit();
            findViewById(id).setVisibility(View.GONE);
        }
    }

    private void switchNetworking(boolean useRp)
    {
        // Do nothing if we're already in this state
        if(_ac.getUseRp() == useRp)
        {
            return;
        }

        SharedPreferences.Editor ed = Globals.getSharedPreferencesEditor();
        ed.putBoolean(PreferenceKeys.RP_USE, useRp);
        ed.apply();

        onMissionChanged();
    }

    private void handleClickOnNetworkIcon()
    {
        if(_ac.getCanUseRp())
        {
            String msg;

            AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(this);

            alertDialogBuilder.setTitle("Networking");

            if(_ac.getUseRp())
            {
                msg = "Currently connected globally via " + _ac.getRpAddress() + ":" + _ac.getRpPort();
                alertDialogBuilder.setPositiveButton("Go Local", new DialogInterface.OnClickListener()
                {
                    @Override
                    public void onClick(DialogInterface dialog, int which)
                    {
                        switchNetworking(false);
                    }
                });
            }
            else
            {
                msg = "Currently connected locally via IP multicast";
                alertDialogBuilder.setPositiveButton("Go Global", new DialogInterface.OnClickListener()
                {
                    @Override
                    public void onClick(DialogInterface dialog, int which)
                    {
                        switchNetworking(true);
                    }
                });
            }

            alertDialogBuilder.setMessage(msg);
            alertDialogBuilder.setCancelable(true);
            alertDialogBuilder.setNegativeButton("Cancel", new DialogInterface.OnClickListener()
            {
                @Override
                public void onClick(DialogInterface dialog, int which)
                {
                    dialog.cancel();
                }
            });

            AlertDialog dlg = alertDialogBuilder.create();
            dlg.show();
        }
        else
        {
            Utils.showLongPopupMsg(LcarsActivity.this, "Local connection via IP multicast");
        }
    }

    private void setupMainScreen()
    {
        ImageView iv;
        TextView tv;

        // Team name
        //tv = findViewById(R.id.tvTeamName);
        //tv.setText(_ac.getMissionName().toUpperCase());

        // Network
        iv = findViewById(R.id.ivNetwork);
        if(iv != null)
        {
            iv.setOnClickListener(new View.OnClickListener()
            {
                @Override
                public void onClick(View v)
                {
                    handleClickOnNetworkIcon();
                }
            });

            if(_ac.getUseRp())
            {
                iv.setImageDrawable(ContextCompat.getDrawable(this, R.drawable.net_global));
            }
            else
            {
                iv.setImageDrawable(ContextCompat.getDrawable(this, R.drawable.net_local));
            }
        }

        // Output type
        /*
        iv = findViewById(R.id.ivOutputType);
        iv.setOnClickListener(new View.OnClickListener()
        {
            @Override
            public void onClick(View v)
            {
                // TODO - get rid of this
            }
        });
        */

        // Map
        iv = findViewById(R.id.ivMap);
        if(iv != null)
        {
            iv.setOnClickListener(new View.OnClickListener()
            {
                @Override
                public void onClick(View v)
                {
                    // TODO
                    startMapActivity();
                }
            });
        }

        // Timeline
        iv = findViewById(R.id.ivTimeline);
        if(iv != null)
        {
            iv.setOnClickListener(new View.OnClickListener()
            {
                @Override
                public void onClick(View v)
                {
                    // TODO
                    String gid = Globals.getSharedPreferences().getString(PreferenceKeys.ACTIVE_MISSION_CONFIGURATION_SELECTED_GROUPS_SINGLE, "");
                    if(!Utils.isEmptyString(gid))
                    {
                        requestGroupTimeline(gid);
                    }
                }
            });
        }

        // Setting
        iv = findViewById(R.id.ivSettings);
        if(iv != null)
        {
            iv.setOnClickListener(new View.OnClickListener()
            {
                @Override
                public void onClick(View v)
                {
                    showPopupMenu();
                }
            });
        }

        // PTT
        iv = findViewById(R.id.ivPtt);
        iv.setOnTouchListener(new View.OnTouchListener()
        {
            @Override
            public boolean onTouch(View v, MotionEvent event)
            {
                if(event.getAction() == MotionEvent.ACTION_DOWN)
                {
                    _pttRequested = true;
                    Globals.getEngageApplication().startTx(0, 0);
                }
                else if(event.getAction() == MotionEvent.ACTION_UP)
                {
                    _pttRequested = false;
                    Globals.getEngageApplication().endTx();
                }

                return true;
            }
        });
    }

    private void redrawPttButton()
    {
        runOnUiThread(new Runnable()
        {
            @Override
            public void run()
            {
                if(_anyTxActive)
                {
                    ((ImageView) findViewById(R.id.ivPtt)).setImageDrawable(ContextCompat.getDrawable(LcarsActivity.this, R.drawable.ptt_active));
                }
                else if(_anyTxPending)
                {
                    ((ImageView) findViewById(R.id.ivPtt)).setImageDrawable(ContextCompat.getDrawable(LcarsActivity.this, R.drawable.ptt_transition));
                }
                else
                {
                    ((ImageView) findViewById(R.id.ivPtt)).setImageDrawable(ContextCompat.getDrawable(LcarsActivity.this, R.drawable.ptt_idle));
                }
            }
        });
    }

    private void redrawCardFragments()
    {
        FragmentManager fragMan = getSupportFragmentManager();
        List<Fragment> fragments = fragMan.getFragments();
        for(Fragment f : fragments)
        {
            if(f instanceof CardFragment)
            {
                ((CardFragment)f).draw();
            }
        }
    }

    private void removeAllFragments()
    {
        FragmentManager fm = getSupportFragmentManager();
        List<Fragment> fragments = fm.getFragments();
        ArrayList<Fragment> trash = new ArrayList<>();
        for (Fragment f : fragments)
        {
            trash.add(f);
        }

        if (!trash.isEmpty())
        {
            FragmentTransaction ft = fm.beginTransaction();
            for (Fragment f : trash)
            {
                ft.remove(f);
            }
            ft.commit();
        }
    }

    private CardFragment getCardForGroup(String id)
    {
        FragmentManager fragMan = getSupportFragmentManager();
        List<Fragment> fragments = fragMan.getFragments();

        for(Fragment f : fragments)
        {
            if(f instanceof CardFragment)
            {
                String groupId = ((CardFragment)f).getGroupId();

                if(groupId != null && groupId.compareTo(id) == 0)
                {
                    return (CardFragment)f;
                }
            }
        }

        return null;
    }

    private void updateFragmentForGroup(String id)
    {
        CardFragment card = getCardForGroup(id);
        if(card == null)
        {
            // TODO: assert?
            return;
        }

        card.draw();
    }

    public void showPopupMenu()
    {
        PopupMenu popup = new PopupMenu(this, findViewById(R.id.ivWorkPanel));

        MenuInflater inflater = popup.getMenuInflater();
        inflater.inflate(R.menu.main_activity_menu, popup.getMenu());

        // Display the developer test menu option if necessary
        popup.getMenu()
                .findItem(R.id.action_dev_test)
                .setVisible(Globals.getSharedPreferences().getBoolean(PreferenceKeys.DEVELOPER_MODE_ACTIVE, false));

        popup.setOnMenuItemClickListener(new PopupMenu.OnMenuItemClickListener()
        {
            @Override
            public boolean onMenuItemClick(MenuItem item)
            {
                int id = item.getItemId();

                if (id == R.id.action_settings)
                {
                    startSettingsActivity();
                    return true;
                }
                else if (id == R.id.action_team_list)
                {
                    showTeamList();
                    return true;
                }
                else if (id == R.id.action_map)
                {
                    startMapActivity();
                    return true;
                }
                else if (id == R.id.action_missions)
                {
                    startMissionListActivity();
                    return true;
                }
                else if (id == R.id.action_share)
                {
                    startShareMissionActivity();
                    return true;
                }
                else if (id == R.id.action_scan)
                {
                    Globals.getEngageApplication().initiateMissionQrCodeScan(LcarsActivity.this);
                    return true;
                }
                else if (id == R.id.action_download)
                {
                    Globals.getEngageApplication().initiateMissionDownload(LcarsActivity.this);
                    return true;
                }
                else if (id == R.id.action_about)
                {
                    startAboutActivity();
                    //performDevSimulation();
                    return true;
                }
                else if (id == R.id.action_contact)
                {
                    startContactActivity();
                    return true;
                }
                else if (id == R.id.action_dev_test)
                {
                    startDevTestActivity();
                    return true;
                }
                else if (id == R.id.action_shutdown)
                {
                    verifyShutdown();
                    return true;
                }

                return false;
            }
        });

        popup.show();
    }

    private void verifyShutdown()
    {
        String s;

        s = getString(R.string.confirm_shutdown);

        final TextView message = new TextView(this);
        final SpannableString ss = new SpannableString(s);

        message.setText(ss);
        message.setMovementMethod(LinkMovementMethod.getInstance());
        message.setPadding(32, 32, 32, 32);

        AlertDialog dlg = new AlertDialog.Builder(this)
                .setTitle(R.string.title_shutdown)
                .setCancelable(false)
                .setPositiveButton(R.string.button_yes, new DialogInterface.OnClickListener()
                {
                    @Override
                    public void onClick(DialogInterface dialogInterface, int i)
                    {
                        Globals.getEngageApplication().terminateApplicationAndReturnToAndroid(LcarsActivity.this);
                    }
                }).setNegativeButton(R.string.button_no, new DialogInterface.OnClickListener()
                {
                    @Override
                    public void onClick(DialogInterface dialogInterface, int i)
                    {
                    }
                }).setView(message).create();

        dlg.show();
    }

    private void setVolumeLevels(String groupId, VolumeLevels vl)
    {
        Globals.getEngageApplication().getEngine().engageSetGroupRxVolume(groupId, vl.left, vl.right);
    }

    private int getSelectedVolumeLeft()
    {
        return _vlInProgress.left;
    }

    private int getSelectedVolumeRight()
    {
        return _vlInProgress.right;
    }

    public void showVolumeSliders(final String groupId)
    {
        if(Utils.isEmptyString(groupId))
        {
            return;
        }

        _vlSaved = Globals.getEngageApplication().loadVolumeLevels(groupId);

        _vlInProgress = new VolumeLevels();
        _vlInProgress.left = _vlSaved.left;
        _vlInProgress.right = _vlSaved.right;

        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        LayoutInflater inflater = (LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        View v = inflater.inflate(R.layout.volume_dialog, null);

        builder
                .setView(v)
                .setCancelable(false)
                .setTitle("Adjust Group Volume")
                .setPositiveButton("Save", new DialogInterface.OnClickListener()
                {
                    @Override
                    public void onClick(DialogInterface dialog, int which)
                    {
                        setVolumeLevels(groupId, _vlInProgress);
                        Globals.getEngageApplication().saveVolumeLevels(groupId, _vlInProgress);
                    }
                })
                .setNegativeButton("Cancel", new DialogInterface.OnClickListener()
                {
                    @Override
                    public void onClick(DialogInterface dialog, int which)
                    {
                        _vlInProgress.left = _vlSaved.left;
                        _vlInProgress.right = _vlSaved.right;
                        setVolumeLevels(groupId, _vlInProgress);
                    }
                })
                .setNeutralButton("Reset", new DialogInterface.OnClickListener()
                {
                    @Override
                    public void onClick(DialogInterface dialog, int which)
                    {
                        _vlInProgress.left = 100;
                        _vlInProgress.right = 100;
                        setVolumeLevels(groupId, _vlInProgress);
                        Globals.getEngageApplication().saveVolumeLevels(groupId, _vlInProgress);
                    }
                });

        final SeekBar sbLeft = v.findViewById(R.id.volumeSeekBarLeft);
        final SeekBar sbRight = v.findViewById(R.id.volumeSeekBarRight);
        final Switch swSync = v.findViewById(R.id.swSync);

        swSync.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener()
        {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked)
            {
                _volumeSynced = isChecked;

                if(_volumeSliderLastMoved != null)
                {
                    if(_volumeSliderLastMoved == sbLeft)
                    {
                        sbRight.setProgress(_vlInProgress.left);
                    }
                    else
                    {
                        sbLeft.setProgress(_vlInProgress.right);
                    }
                }
            }
        });

        swSync.setChecked(_volumeSynced);

        sbLeft.setMax(200);
        sbLeft.setProgress(getSelectedVolumeLeft());
        sbLeft.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener()
        {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser)
            {
                _vlInProgress.left = progress;
                if(_volumeSynced)
                {
                    sbRight.setProgress(progress);
                }

                setVolumeLevels(groupId, _vlInProgress);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar)
            {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar)
            {
                _volumeSliderLastMoved = seekBar;
            }
        });


        sbRight.setMax(200);
        sbRight.setProgress(getSelectedVolumeRight());
        sbRight.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener()
        {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser)
            {
                _vlInProgress.right = progress;
                if(_volumeSynced)
                {
                    sbLeft.setProgress(progress);
                }

                setVolumeLevels(groupId, _vlInProgress);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar)
            {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar)
            {
                _volumeSliderLastMoved = seekBar;
            }
        });

        builder.create().show();
    }
}
