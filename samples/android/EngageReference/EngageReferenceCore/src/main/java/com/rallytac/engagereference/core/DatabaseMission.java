//
//  Copyright (c) 2019 Rally Tactical Systems, Inc.
//  All rights reserved.
//

package com.rallytac.engagereference.core;

import org.json.JSONArray;
import org.json.JSONObject;

import java.util.ArrayList;

public class DatabaseMission
{
    public String _id;
    public String _modPin;
    public String _name;
    public String _description;

    public boolean _useRp;
    public String _rpAddress;
    public int _rpPort;

    public String _mcId;
    public String _mcAddress;
    public int _mcPort;
    public String _mcCryptoPassword;

    public ArrayList<DatabaseGroup> _groups = new ArrayList<>();

    @Override
    public String toString()
    {
        JSONObject obj = toJson();
        return (obj != null ? obj.toString() : "null");
    }

    public DatabaseGroup getGroupById(String id)
    {
        for(DatabaseGroup group : _groups)
        {
            if(group._id.compareTo(id) == 0)
            {
                return group;
            }
        }

        return null;
    }

    public boolean deleteGroupById(String id)
    {
        for(DatabaseGroup group : _groups)
        {
            if(group._id.compareTo(id) == 0)
            {
                _groups.remove(group);
                return true;
            }
        }

        return false;
    }

    public boolean updateGroupById(String id, DatabaseGroup updatedGroup)
    {
        int index = 0;

        for(DatabaseGroup group : _groups)
        {
            if(group._id.compareTo(id) == 0)
            {
                _groups.set(index, updatedGroup);
                return true;
            }

            index++;
        }

        return false;
    }


    public JSONObject toJson()
    {
        JSONObject root;

        try
        {
            root = new JSONObject();
            root.put("_id", Utils.trimString(_id));
            root.put("_modPin", Utils.trimString(_modPin));
            root.put("_name", Utils.trimString(_name));
            root.put("_description", Utils.trimString(_description));
            root.put("_useRp", _useRp);
            root.put("_rpAddress", Utils.trimString(_rpAddress));
            root.put("_rpPort", _rpPort);

            root.put("_mcId", Utils.trimString(_mcId));
            root.put("_mcAddress", Utils.trimString(_mcAddress));
            root.put("_mcPort", _mcPort);
            root.put("_mcCryptoPassword", Utils.trimString(_mcCryptoPassword));

            JSONArray groups = new JSONArray();
            for(DatabaseGroup group : _groups)
            {
                groups.put(group.toJson());
            }

            root.put("groups", groups);
        }
        catch (Exception e)
        {
            root = null;
        }

        return root;
    }

    public static DatabaseMission parse(String json)
    {
        DatabaseMission mission = new DatabaseMission();

        try
        {
            JSONObject root = new JSONObject(json);
            mission._id = Utils.trimString(root.getString("_id"));
            mission._modPin = Utils.trimString(root.optString("_modPin"));
            mission._name = Utils.trimString(root.optString("_name"));
            mission._description = Utils.trimString(root.optString("_description"));
            mission._useRp = root.optBoolean("_useRp");
            mission._rpAddress = Utils.trimString(root.optString("_rpAddress"));
            mission._rpPort = root.optInt("_rpPort");
            mission._mcId = Utils.trimString(root.getString("_mcId"));
            mission._mcAddress = Utils.trimString(root.getString("_mcAddress"));
            mission._mcPort = root.getInt("_mcPort");
            mission._mcCryptoPassword = Utils.trimString(root.getString("_mcCryptoPassword"));

            JSONArray groups = root.optJSONArray("groups");
            if(groups != null)
            {
                for(int x = 0; x < groups.length(); x++)
                {
                    DatabaseGroup group = DatabaseGroup.parse(groups.get(x).toString());
                    if(group != null)
                    {
                        mission._groups.add(group);
                    }
                }
            }
        }
        catch (Exception e)
        {
            mission = null;
        }

        return mission;
    }
}
