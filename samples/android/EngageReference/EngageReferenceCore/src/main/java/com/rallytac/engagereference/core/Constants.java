//
//  Copyright (c) 2019 Rally Tactical Systems, Inc.
//  All rights reserved.
//

package com.rallytac.engagereference.core;

public class Constants
{
    public static String QR_CODE_HEADER = "&*3$e1@E";
    public static String QR_VERSION = "001";
    public static String QR_DEFLECTION_URL_SEP = "/??";

    public enum UiMode {vSingle, vMulti}

    public final static int LAUNCH_TIMEOUT_MS = 5000;

    public final static int MIN_IP_PORT = 1;
    public final static int MAX_IP_PORT = 65535;

    public final static String CHARSET = "UTF-8";

    public final static UiMode DEF_UI_MODE = UiMode.vSingle;

    public final static float DEF_PTT_TONE_LEVEL = (float)0.015;
    public final static float DEF_NOTIFICATION_TONE_LEVEL = (float)0.015;
    public final static float DEF_ERROR_TONE_LEVEL = (float)0.2;

    public final static int DEF_SPEAKER_OUTPUT_BOOST_FACTOR = 3;

    public final static boolean DEF_USE_RP = false;
    public final static String DEF_RP_ADDRESS = "";
    public final static int DEF_RP_PORT = 7443;

    public final static String DEF_BINDING_NIC_NAME = "wlan0";

    public final static String DEF_USER_NODE_ID = "";
    public final static String DEF_USER_ID = "";
    public final static String DEF_USER_DISPLAY_NAME = "";
    public final static String DEF_USER_ALIAS_ID = "";
    public final static String USER_ALIAS_ID_GENERATE_FORMAT = "ANDROID-%08X";

    public final static boolean DEF_LOCATION_ENABLED = true;
    public final static int DEF_LOCATION_ACCURACY = LocationManager.PRIORITY_BALANCED_POWER_ACCURACY;
    public final static int DEF_LOCATION_INTERVAL_SECS = 60;
    public final static int DEF_LOCATION_MIN_INTERVAL_SECS = 60;
    public final static float DEF_LOCATION_MIN_DISPLACEMENT = (float)5.0;

    public final static int QR_CODE_WIDTH = 800;
    public final static int QR_CODE_HEIGHT = 800;

    public final static int RX_IDLE_SECS_BEFORE_NOTIFICATION = 30;
    public final static int TX_IDLE_SECS_BEFORE_NOTIFICATION = (RX_IDLE_SECS_BEFORE_NOTIFICATION / 2);

    public final static boolean DEF_NOTIFY_NODE_JOIN = true;
    public final static boolean DEF_NOTIFY_NODE_LEAVE = true;
    public final static boolean DEF_NOTIFY_NEW_AUDIO_RX = true;
    public final static boolean DEF_NOTIFY_NETWORK_ERROR = true;
    public final static boolean DEF_NOTIFY_VIBRATIONS = true;
    public final static boolean DEF_NOTIFY_PTT_EVERY_TIME = false;

    public final static int GROUP_HEALTH_CHECK_TIMER_INITIAL_DELAY_MS = 2000;
    public final static int GROUP_HEALTH_CHECK_TIMER_INTERVAL_MS = 2000;
    public final static int GROUP_HEALTH_CHECK_NETWORK_ERROR_NOTIFICATION_MIN_INTERVAL_MS = 10000;

    public static final String MISSION_DATABASE_NAME = "MissionDatabase";
    public static final String MISSION_EDIT_EXTRA_JSON = "MissionJson";
    public static final String MISSION_ACTIVATED_ID = "ActivatedMissionId";

    public final static String DEF_CONFIGHUB_BASE_URL = "https://s3.us-east-2.amazonaws.com/rts-missions";

    public static final long MIN_LICENSE_ACTIVATION_DELAY_MS = 60000;
}
