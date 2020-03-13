//
//  Copyright (c) 2019 Rally Tactical Systems, Inc.
//  All rights reserved.
//

package com.rallytac.engageandroid;

import android.os.Parcel;
import android.os.Parcelable;

import com.rallytac.engage.engine.Engine;

import org.json.JSONObject;

import java.util.ArrayList;

public class GroupDescriptor implements Parcelable
{
    public enum Type {gtUnknown, gtAudio, gtPresence, gtRaw}

    // Config
    public Type type;
    public String id;
    public String name;
    public boolean isEncrypted;
    public boolean fdx;
    public String jsonConfiguration;

    private boolean _isDynamic;

    // State
    public boolean selectedForSingleView;
    public boolean selectedForMultiView;
    public boolean created;
    public boolean createError;
    public boolean joined;
    public boolean joinError;
    public boolean rx;
    public boolean tx;
    public boolean txPending;
    public boolean txError;
    public boolean txUsurped;
    public boolean rxMuted;
    public boolean txMuted;
    public ArrayList<TalkerDescriptor> talkerList = new ArrayList<>();
    public boolean operatingInMulticastFailover;
    public boolean hasMulticastConnection;
    public boolean hasRpConnection;

    public static final Creator CREATOR = new Creator()
    {
        public GroupDescriptor createFromParcel(Parcel in)
        {
            return new GroupDescriptor(in);
        }

        public GroupDescriptor[] newArray(int size)
        {
            return new GroupDescriptor[size];
        }
    };

    public GroupDescriptor()
    {
        resetState();

        created = false;
        createError = false;
        joined = false;
        joinError = false;
        hasMulticastConnection = false;
        hasRpConnection = false;
        _isDynamic = false;
        operatingInMulticastFailover = false;

        this.type = Type.gtUnknown;
    }

    GroupDescriptor(Parcel in)
    {
        resetState();

        created = false;
        createError = false;
        joined = false;
        joinError = false;
        hasMulticastConnection = false;
        hasRpConnection = false;
        _isDynamic = false;
        operatingInMulticastFailover = false;


        this.jsonConfiguration = in.readString();
        this.type = Type.values()[in.readInt()];
        this.id = in.readString();
        this.name = in.readString();
        this.isEncrypted = (in.readInt() == 1);
        this.fdx = (in.readInt() == 1);

        this.selectedForSingleView = (in.readInt() == 1);
        this.selectedForMultiView = (in.readInt() == 1);
        this.created = (in.readInt() == 1);
        this.createError = (in.readInt() == 1);
        this.joined = (in.readInt() == 1);
        this.joinError = (in.readInt() == 1);
        this.hasMulticastConnection = (in.readInt() == 1);
        this.hasRpConnection = (in.readInt() == 1);
        this.rx = (in.readInt() == 1);
        this.tx = (in.readInt() == 1);
        this.txPending = (in.readInt() == 1);
        this.txError = (in.readInt() == 1);
        this.txUsurped = (in.readInt() == 1);
        this.rxMuted = (in.readInt() == 1);
        this.txMuted = (in.readInt() == 1);

        this._isDynamic = (in.readInt() == 1);
        this.operatingInMulticastFailover = (in.readInt() == 1);
    }

    public boolean isDynamic()
    {
        return _isDynamic;
    }

    public void setDynamic(boolean b)
    {
        _isDynamic = b;
    }

    public boolean isConnectedInSomeForm()
    {
        return (hasMulticastConnection || hasRpConnection);
    }

    public void resetState()
    {
        rx = false;
        tx = false;
        txPending = false;
        txError = false;
        txUsurped = false;
        rxMuted = false;
        txMuted = false;
        talkerList.clear();
        hasMulticastConnection = false;
        hasRpConnection = false;
    }

    @Override
    public int describeContents()
    {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags)
    {
        dest.writeString(this.jsonConfiguration);
        dest.writeInt(type.ordinal());
        dest.writeString(this.id);
        dest.writeString(this.name);
        dest.writeInt(this.isEncrypted ? 1 : 0);
        dest.writeInt(this.fdx ? 1 : 0);

        dest.writeInt(this.selectedForSingleView ? 1 : 0);
        dest.writeInt(this.selectedForMultiView ? 1 : 0);
        dest.writeInt(this.created ? 1 : 0);
        dest.writeInt(this.createError ? 1 : 0);
        dest.writeInt(this.joined ? 1 : 0);
        dest.writeInt(this.joinError ? 1 : 0);
        dest.writeInt(this.hasMulticastConnection ? 1 : 0);
        dest.writeInt(this.hasRpConnection ? 1 : 0);
        dest.writeInt(this.rx ? 1 : 0);
        dest.writeInt(this.tx ? 1 : 0);
        dest.writeInt(this.txPending ? 1 : 0);
        dest.writeInt(this.txError ? 1 : 0);
        dest.writeInt(this.txUsurped ? 1 : 0);
        dest.writeInt(this.rxMuted ? 1 : 0);
        dest.writeInt(this.txMuted ? 1 : 0);

        dest.writeInt(this._isDynamic ? 1 : 0);
        dest.writeInt(this.operatingInMulticastFailover ? 1 : 0);
    }

    public void updateTalkers(ArrayList<TalkerDescriptor> list)
    {
        synchronized (talkerList)
        {
            talkerList.clear();
            if (list != null)
            {
                for (TalkerDescriptor td : list)
                {
                    talkerList.add(td);
                }
            }
        }
    }

    public String getTalkers()
    {
        StringBuilder sb = new StringBuilder();

        synchronized (talkerList)
        {
            for(TalkerDescriptor td : talkerList)
            {
                if(sb.length() > 0)
                {
                    sb.append(", ");
                }

                sb.append(td.alias);
            }
        }

        return sb.toString();
    }

    public boolean loadFromJson(String json)
    {
        boolean rc;

        try
        {
            JSONObject obj = new JSONObject(json);

            id = obj.getString(Engine.JsonFields.Group.id);
            type = Type.values()[obj.getInt(Engine.JsonFields.Group.type)];
            name = obj.getString(Engine.JsonFields.Group.name);
            isEncrypted = !Utils.isEmptyString(obj.optString(Engine.JsonFields.Group.cryptoPassword, null));
            fdx = obj.optBoolean(Engine.JsonFields.Group.fdx);

            jsonConfiguration = json;

            rc = true;
        }
        catch (Exception e)
        {
            rc = false;
        }

        return rc;
    }
}
