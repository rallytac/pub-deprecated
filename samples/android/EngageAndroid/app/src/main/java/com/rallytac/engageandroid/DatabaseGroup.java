//
//  Copyright (c) 2019 Rally Tactical Systems, Inc.
//  All rights reserved.
//

package com.rallytac.engageandroid;

import org.json.JSONObject;

public class DatabaseGroup
{
    public String _id;
    public int _type;
    public String _name;
    public boolean _useCrypto;
    public String _cryptoPassword;
    public String _rxAddress;
    public int _rxPort;
    public String _txAddress;
    public int _txPort;
    public int _txCodecId;
    public int _txFramingMs;
    public boolean _noHdrExt;
    public boolean _fdx;
    public int _maxTxSecs;

    @Override
    public String toString()
    {
        JSONObject obj = toJson();
        return (obj != null ? obj.toString() : "null");
    }

    public JSONObject toJson()
    {
        JSONObject root;

        try
        {
            root = new JSONObject();
            root.put("_id", Utils.trimString(_id));
            root.put("_type", _type);
            root.put("_name", Utils.trimString(_name));
            root.put("_useCrypto", _useCrypto);
            root.put("_cryptoPassword", Utils.trimString(_cryptoPassword));
            root.put("_rxAddress", Utils.trimString(_rxAddress));
            root.put("_rxPort", _rxPort);
            root.put("_txAddress", Utils.trimString(_txAddress));
            root.put("_txPort", _txPort);
            root.put("_txCodecId", _txCodecId);
            root.put("_txFramingMs", _txFramingMs);
            root.put("_noHdrExt", _noHdrExt);
            root.put("_fdx", _fdx);
            root.put("_maxTxSecs", _maxTxSecs);
        }
        catch (Exception e)
        {
            root = null;
        }

        return root;
    }

    public static DatabaseGroup parse(String json)
    {
        DatabaseGroup group = new DatabaseGroup();

        try
        {
            JSONObject root = new JSONObject(json);
            group._id = root.getString("_id").trim();
            group._type = root.getInt("_type");
            group._name = Utils.trimString(root.optString("_name"));
            group._useCrypto = root.optBoolean("_useCrypto");
            group._cryptoPassword = Utils.trimString(root.optString("_cryptoPassword"));
            group._rxAddress = Utils.trimString(root.optString("_rxAddress"));
            group._rxPort = root.optInt("_rxPort");
            group._txAddress = Utils.trimString(root.optString("_txAddress"));
            group._txPort = root.optInt("_txPort");
            group._txCodecId = root.optInt("_txCodecId");
            group._txFramingMs = root.optInt("_txFramingMs");
            group._noHdrExt = root.optBoolean("_noHdrExt");
            group._fdx = root.optBoolean("_fdx");
            group._maxTxSecs = root.optInt("_maxTxSecs");
        }
        catch (Exception e)
        {
            group = null;
        }

        return group;
    }
}
