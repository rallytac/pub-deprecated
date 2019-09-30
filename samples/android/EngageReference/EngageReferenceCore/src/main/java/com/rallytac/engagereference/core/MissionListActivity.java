//
//  Copyright (c) 2019 Rally Tactical Systems, Inc.
//  All rights reserved.
//

package com.rallytac.engagereference.core;

import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.design.widget.FloatingActionButton;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.ActionBar;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.text.SpannableString;
import android.text.method.LinkMovementMethod;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import java.util.ArrayList;

public class MissionListActivity extends AppCompatActivity
{
    private static String TAG = MissionListActivity.class.getSimpleName();

    private static int EDIT_ACTION_REQUEST_CODE = 42;

    private MissionDatabase _database;
    private MissionListAdapter _adapter;
    private Intent _resultIntent = new Intent();
    private String _activeMissionId;
    private String _activeMissionJson;

    private class MissionListAdapter extends ArrayAdapter<DatabaseMission>
    {
        private Context _ctx;
        private int _resId;

        public MissionListAdapter(Context ctx, int resId, ArrayList<DatabaseMission> list)
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

            final DatabaseMission item = getItem(position);

            if(!Utils.isEmptyString(item._name))
            {
                ((TextView)convertView.findViewById(R.id.tvMissionName)).setText(item._name);
            }
            else
            {
                ((TextView)convertView.findViewById(R.id.tvMissionName)).setText("(no name)");
            }

            if(!Utils.isEmptyString(item._description))
            {
                ((TextView)convertView.findViewById(R.id.tvDescription)).setText(item._description);
            }
            else
            {
                ((TextView)convertView.findViewById(R.id.tvDescription)).setText("(no description)");
            }

            int groupCount;

            if(item._groups == null || item._groups.size() == 0)
            {
                groupCount = 0;
            }
            else
            {
                groupCount = item._groups.size();
            }

            ((TextView)convertView.findViewById(R.id.tvGroupCount)).setText(Integer.toString(groupCount));

            convertView.setOnClickListener(new View.OnClickListener()
            {
                @Override
                public void onClick(View v)
                {
                    editMission(item._id);
                }
            });

            if(_activeMissionId.compareTo(item._id) == 0)
            {
                convertView.findViewById(R.id.ivDeleteMission).setVisibility(View.INVISIBLE);
            }
            else
            {
                convertView.findViewById(R.id.ivDeleteMission).setOnClickListener(new View.OnClickListener()
                {
                    @Override
                    public void onClick(View v)
                    {
                        confirmDeleteMission(item._id);
                    }
                });

                if (_activeMissionId.compareTo(item._id) == 0)
                {
                    ((ImageView) convertView.findViewById(R.id.ivActiveMissionIndicator))
                            .setImageDrawable(ContextCompat.getDrawable(MissionListActivity.this,
                                    R.drawable.radio_button_selected));
                }
                else
                {
                    ((ImageView) convertView.findViewById(R.id.ivActiveMissionIndicator))
                            .setImageDrawable(ContextCompat.getDrawable(MissionListActivity.this,
                                    R.drawable.radio_button_unselected));
                }
            }

            convertView.findViewById(R.id.ivActiveMissionIndicator).setOnClickListener(new View.OnClickListener()
            {
                @Override
                public void onClick(View v)
                {
                    activateMission(item._id);
                }
            });

            return convertView;
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_mission_list);

        FloatingActionButton fab = findViewById(R.id.fab);
        fab.setOnClickListener(new View.OnClickListener()
        {
            @Override
            public void onClick(View view)
            {
                addMission();
            }
        });

        _activeMissionId = Globals.getEngageApplication().getActiveConfiguration().getMissionId();

        _database = MissionDatabase.load(Globals.getSharedPreferences(), Constants.MISSION_DATABASE_NAME);
        if(_database == null)
        {
            _database = new MissionDatabase();
        }

        // Grab the active mission's JSON - we'll use it later
        DatabaseMission activeDatabaseMission = _database.getMissionById(_activeMissionId);
        if(activeDatabaseMission != null)
        {
            _activeMissionJson = activeDatabaseMission.toString();
        }

        _adapter = new MissionListAdapter(this, R.layout.mission_list_entry, _database._missions);
        ListView lv = findViewById(R.id.lvMissions);
        lv.setAdapter(_adapter);

        setupActionBar();
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

    private void activateMission(String id)
    {
        _resultIntent.putExtra(Constants.MISSION_ACTIVATED_ID, id);
        setResult(RESULT_OK, _resultIntent);
        finish();
    }

    private void addMission()
    {
        Intent intent = new Intent(this, MissionEditActivity.class);
        startActivityForResult(intent, EDIT_ACTION_REQUEST_CODE);
    }

    private void editMission(String id)
    {
        DatabaseMission mission = _database.getMissionById(id);
        Intent intent = new Intent(this, MissionEditActivity.class);
        intent.putExtra(Constants.MISSION_EDIT_EXTRA_JSON, mission.toJson().toString());
        startActivityForResult(intent, EDIT_ACTION_REQUEST_CODE);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent intent)
    {
        Log.d(TAG, "onActivityResult");

        if(resultCode == RESULT_OK)
        {
            if (requestCode == EDIT_ACTION_REQUEST_CODE )
            {
                if(intent != null)
                {
                    String json = intent.getStringExtra(Constants.MISSION_EDIT_EXTRA_JSON);
                    if (!Utils.isEmptyString(json))
                    {
                        DatabaseMission mission = DatabaseMission.parse(json);

                        if(!_database.updateMissionById(mission._id, mission))
                        {
                            _database._missions.add(mission);
                        }

                        _database.save(Globals.getSharedPreferences(), Constants.MISSION_DATABASE_NAME);

                        _adapter.notifyDataSetChanged();

                        // See if what was changed was the active mission, if so, we need to
                        // make sure our resultIntent is set correctly
                        if(mission._id.compareTo(_activeMissionId) == 0)
                        {
                            String newJson = mission.toString();
                            if(newJson.compareTo(_activeMissionJson) != 0)
                            {
                                _resultIntent.putExtra(Constants.MISSION_ACTIVATED_ID, _activeMissionId);
                                setResult(RESULT_OK, _resultIntent);
                            }
                        }
                    }
                }
            }
        }
    }

    private void confirmDeleteMission(final String id)
    {
        if(id.compareTo(_activeMissionId) == 0)
        {
            Toast.makeText(this, "The active mission cannot be deleted", Toast.LENGTH_SHORT).show();
        }
        else
        {
            DatabaseMission mission = _database.getMissionById(id);
            String s;

            s = "Are you sure you want to delete " + mission._name + "?";

            final TextView message = new TextView(this);
            final SpannableString ss = new SpannableString(s);

            message.setText(ss);
            message.setMovementMethod(LinkMovementMethod.getInstance());
            message.setPadding(32, 32, 32, 32);

            AlertDialog dlg = new AlertDialog.Builder(this)
                    .setTitle("Delete Mission")
                    .setCancelable(false)
                    .setPositiveButton("Yes", new DialogInterface.OnClickListener()
                    {
                        @Override
                        public void onClick(DialogInterface dialogInterface, int i)
                        {
                            deleteMission(id);
                        }
                    }).setNegativeButton("Cancel", new DialogInterface.OnClickListener()
                    {
                        @Override
                        public void onClick(DialogInterface dialogInterface, int i)
                        {
                        }
                    }).setView(message).create();

            dlg.show();
        }
    }

    private void deleteMission(String id)
    {
        if(_database.deleteMissionById(id))
        {
            _adapter.notifyDataSetChanged();
        }

        _database.save(PreferenceManager.getDefaultSharedPreferences(this), Constants.MISSION_DATABASE_NAME);
    }
}
