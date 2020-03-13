package com.rallytac.engageandroid;

public class Analytics
{
    public static final String ENGINE_STARTED = "ENGINE_STARTED";
    public static final String ENGINE_START_FAILED = "ENGINE_START_FAILED";
    public static final String ENGINE_STOPPED = "ENGINE_STOPPED";

    public static final String GROUP_CREATED = "GRP_CREATED";
    public static final String GROUP_CREATE_FAILED = "GRP_CREATE_FAILED";
    public static final String GROUP_DELETED = "GRP_DELETED";

    public static final String GROUP_CONNECTED_MC = "GRP_CONN_MC";
    public static final String GROUP_CONNECTED_RP = "GRP_CONN_RP";
    public static final String GROUP_CONNECTED_MC_FAILOVER = "GRP_CONN_MC_FO";
    public static final String GROUP_CONNECTED_OTHER = "GRP_CONN_OTHER";

    public static final String GROUP_CONNECT_FAILED_MC = "GRP_CONN_FAILED_MC";
    public static final String GROUP_CONNECT_FAILED_RP = "GRP_CONN_FAILED_RP";
    public static final String GROUP_CONNECT_FAILED_MC_FAILOVER = "GRP_CONN_FAILED_MC_FO";
    public static final String GROUP_CONNECT_FAILED_OTHER = "GRP_CONN_FAILED_OTHER";

    public static final String GROUP_DISCONNECTED_MC = "GRP_DISCONN_MC";
    public static final String GROUP_DISCONNECTED_RP = "GRP_DISCONN_RP";
    public static final String GROUP_DISCONNECTED_MC_FAILOVER = "GRP_DISCONN_MC_FO";
    public static final String GROUP_DISCONNECTED_OTHER = "GRP_DISCONN_OTHER";

    public static final String GROUP_JOINED = "GRP_JOINED";
    public static final String GROUP_JOIN_FAILED = "GRP_JOIN_FAILED";
    public static final String GROUP_LEFT = "GRP_LEFT";

    //public static final String GROUP_MEMBER_COUNT_CHANGED= "GRP_MEMBER_COUNT_CHANGED";

    public static final String GROUP_RX_STARTED = "GRP_RX_STARTED";
    public static final String GROUP_RX_ENDED = "GRP_RX_ENDED";
    //public static final String GROUP_RX_SPEAKER_COUNT_CHANGED = "GRP_RX_SPEAKER_COUNT_CHANGED";

    public static final String GROUP_RX_MUTED = "GRP_RX_MUTED";
    public static final String GROUP_RX_UNMUTED = "GRP_RX_UNMUTED";

    public static final String GROUP_TX_REQUESTED_SINGLE = "GRP_TX_REQ_SINGLE";
    public static final String GROUP_TX_REQUESTED_MULTIPLE = "GRP_TX_REQ_MULTIPLE";

    public static final String GROUP_TX_STARTED = "GRP_TX_STARTED";
    public static final String GROUP_TX_FAILED = "GRP_TX_FAILED";
    public static final String GROUP_TX_ENDED = "GRP_TX_ENDED";
    public static final String GROUP_TX_MAX_EXCEEDED = "GRP_TX_MAX_EXC";
    public static final String GROUP_TX_USURPED = "GRP_TX_USURPED";

    //public static final String GROUP_TX_MUTED = "GRP_TX_MUTED";
    //public static final String GROUP_TX_UNMUTED = "GRP_TX_UNMUTED";

    //public static final String GROUP_NODE_DISCOVERED = "GRP_NODE_DISCOVERED";
    //public static final String GROUP_NODE_REDISCOVERED = "GRP_NODE_REDISCOVERED";
    //public static final String GROUP_NODE_UNDISCOVERED = "GRP_NODE_UNDISCOVERED";

    public static final String LICENSE_CHANGED = "LIC_CHANGED";
    public static final String LICENSE_EXPIRED = "LIC_EXPIRED";
    //public static final String LICENSE_EXPIRING = "LIC_EXPIRING";

    public static final String LICENSE_ACT_OK = "LIC_ACT_OK";
    public static final String LICENSE_ACT_OK_ALREADY = "LIC_ACT_OK_ALREADY";
    public static final String LICENSE_ACT_FAILED_NO_KEY = "LIC_ACT_FAILED_NO_KEY";
    public static final String LICENSE_ACT_FAILED = "LIC_ACT_FAILED";

    public static final String GROUP_ASSET_DISCOVERED = "GRP_ASSET_DISCOVERED";
    //public static final String GROUP_ASSET_REDISCOVERED = "GRP_ASSET_REDISCOVERED";
    public static final String GROUP_ASSET_UNDISCOVERED = "GRP_ASSET_UNDISCOVERED";

    public static final String GROUP_TIMELINE_REPORT = "GRP_TIMELINE_REPORT";
    public static final String GROUP_TIMELINE_REPORT_FAILED = "GRP_TIMELINE_REPORT_FAILED";

    public static final String GROUP_RP_CONNECTED = "GRP_RP_CONN";
    public static final String GROUP_RP_DISCONNECTED = "GRP_RP_DISCONN";

    public static final String GROUP_RP_RT_100 = "GRP_RP_RT_100";
    public static final String GROUP_RP_RT_75 = "GRP_RP_RT_75";
    public static final String GROUP_RP_RT_50 = "GRP_RP_RT_50";
    public static final String GROUP_RP_RT_25 = "GRP_RP_RT_25";
    public static final String GROUP_RP_RT_10 = "GRP_RP_RT_10";
    public static final String GROUP_RP_RT_0 = "GRP_RP_RT_0";

    public static final String VIEW_SINGLE_MODE = "VIEW_SINGLE_MODE";
    public static final String VIEW_MULTI_MODE = "VIEW_MULTI_MODE";
    public static final String VIEW_TEAM = "VIEW_TEAM";
    public static final String VIEW_MAP = "VIEW_MAP";
    public static final String VIEW_SHARE_MISSION = "VIEW_SHARE_MISSION";
    public static final String VIEW_SETTINGS = "VIEW_SETTINGS";
    public static final String VIEW_MISSION_LIST = "VIEW_MISSION_LIST";
    public static final String VIEW_ABOUT = "VIEW_ABOUT";
    public static final String VIEW_CONTACT = "VIEW_CONTACT";
    public static final String VIEW_GROUP_LIST = "VIEW_GROUP_LIST";
    public static final String VIEW_TIMELINE = "VIEW_TIMELINE";

    public static final String TOGGLE_NETWORKING = "TOGGLE_NETWORKING";
    public static final String MISSION_CHANGED = "MISS_CHANGED";
    public static final String SINGLE_VIEW_GROUP_CHANGED = "SINGLE_VIEW_GROUP_CHANGED";

    public static final String MISSION_QR_CODE_DISPLAYED_FOR_SHARE = "MISS_QR_CODE_DISP_FOR_SHARE";
    public static final String MISSION_QR_CODE_FAILED_CREATE = "MISS_QR_CODE_FAILED_CREATE";
    public static final String MISSION_SHARE_JSON = "MISS_SHARE_JSON";
    public static final String MISSION_SHARE_QR = "MISS_SHARE_QR";
    public static final String MISSION_SHARE_EXCEPTION = "MISS_SHARE_EXCEPTION";
    public static final String MISSION_UPLOAD_REQUESTED = "MISS_UPLOAD_REQUESTED";

    public static final String NEW_LICENSE_FROM_USER = "NEW_LICENSE_FROM_USER";
}
