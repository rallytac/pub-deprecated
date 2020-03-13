//
//  Copyright (c) 2019 Rally Tactical Systems, Inc.
//  All rights reserved.
//

package com.rallytac.engageandroid;

public class PreferenceKeys
{
    public static String APP_FIRST_TIME_RUN = "app.firstTimeRun";
    public static String LAUNCHER_RUN_BEFORE = "app.launcherRunBefore";

    public static String UI_MODE = "ui.mode";

    public static String NETWORK_BINDING_NIC_NAME = "network_bindingNic";
    public static String NETWORK_MULTICAST_FAILOVER_ENABLED = "network_multicastFailover.enabled";
    public static String NETWORK_MULTICAST_FAILOVER_SECS = "network_multicastFailover.secs";

    public static String ACTIVE_MISSION_CONFIGURATION_JSON = "activeConfiguration_jsonTemplate";
    public static String ACTIVE_MISSION_CONFIGURATION_SELECTED_GROUPS_SINGLE = "activeConfiguration_selectedGroups_single";
    public static String ACTIVE_MISSION_CONFIGURATION_SELECTED_GROUPS_MULTI = "activeConfiguration_selectedGroups_multi";

    public static String VOLUME_LEFT_FOR_GROUP_BASE_NAME = "volume.left.";
    public static String VOLUME_RIGHT_FOR_GROUP_BASE_NAME = "volume.right.";

    public static String USER_NODE_ID = "user_nodeId";
    public static String USER_ID = "user_id";
    public static String USER_DISPLAY_NAME = "user_displayName";
    public static String USER_ALIAS_ID = "user_Alias";

    public static String USER_TONE_LEVEL_PTT = "user_toneLevel.ptt";
    public static String USER_TONE_LEVEL_ERROR = "user_toneLevel.error";
    public static String USER_TONE_LEVEL_NOTIFICATION = "user_toneLevel.notification";
    public static String USER_SPEAKER_OUTPUT_BOOST_FACTOR = "user_audio.output.boostFactor";

    public static String USER_UI_PTT_LATCHING = "user_ui.pttLatching";
    public static String USER_UI_PTT_VOICE_CONTROL = "user_ui.pttButtonVoiceControl";

    public static String LAST_QRCODE_DEFLECTION_URL = "lastQrCodeDeflectionUrl";
    public static String QR_CODE_SCAN_PASSWORD = "qrCodeScanPassword";

    public static String MAP_OPTION_BASE = "map.option.";
    public static String MAP_OPTION_VIEW_INDEX = MAP_OPTION_BASE + "viewIndex";
    public static String MAP_OPTION_TYPE = MAP_OPTION_BASE + "type";

    public static String MAP_OPTION_CAM_BASE = "map.option.camera.";
    public static String MAP_OPTION_CAM_ZOOM = MAP_OPTION_CAM_BASE + "zoom";
    public static String MAP_OPTION_CAM_BEARING = MAP_OPTION_CAM_BASE + "bearing";
    public static String MAP_OPTION_CAM_LAT = MAP_OPTION_CAM_BASE + "lat";
    public static String MAP_OPTION_CAM_LON = MAP_OPTION_CAM_BASE + "lon";
    public static String MAP_OPTION_CAM_TILT = MAP_OPTION_CAM_BASE + "tilt";

    public static String USER_LOCATION_SHARED = "user_location.enabled";
    public static String USER_LOCATION_INTERVAL_SECS = "user_location.intervalSecs";
    public static String USER_LOCATION_MIN_INTERVAL_SECS = "user_location.minIntervalSecs";
    public static String USER_LOCATION_ACCURACY = "user_location.accuracy";
    public static String USER_LOCATION_MIN_DISPLACEMENT = "user_location.minDisplacement";

    public static String USER_NOTIFY_NODE_JOIN = "user_notify.nodeJoin";
    public static String USER_NOTIFY_NODE_LEAVE = "user_notify.nodeLeave";
    public static String USER_NOTIFY_NEW_AUDIO_RX = "user_notify.newAudioRx";
    public static String USER_NOTIFY_NETWORK_ERROR = "user_notify.networkError";
    public static String USER_NOTIFY_VIBRATIONS = "user_notify.vibrations";
    public static String USER_NOTIFY_PTT_EVERY_TIME = "user_notify.ptt_everyTime";

    public static String USER_EXPERIMENT_ENABLE_SSDP_DISCOVERY = "user_experiment.discovery.ssdp.enable";

    public static String USER_EXPERIMENT_ENABLE_CISTECH_GV1_DISCOVERY = "user_experiment.discovery.cistech.gv1.enable";
    public static String USER_EXPERIMENT_CISTECH_GV1_DISCOVERY_ADDRESS = "user_experiment.discovery.cistech.gv1.address";
    public static String USER_EXPERIMENT_CISTECH_GV1_DISCOVERY_PORT = "user_experiment.discovery.cistech.gv1.port";
    public static String USER_EXPERIMENT_CISTECH_GV1_DISCOVERY_TIMEOUT_SECS = "user_experiment.discovery.cistech.gv1.timeoutSecs";

    public static String USER_EXPERIMENT_ENABLE_TRELLISWARE_DISCOVERY = "user_experiment.discovery.trellisware.enable";

    public static String USER_EXPERIMENT_ENABLE_DEVICE_REPORT_CONNECTIVITY = "user_experiment.deviceReport.connectivity.enable";
    public static String USER_EXPERIMENT_ENABLE_DEVICE_REPORT_POWER = "user_experiment.deviceReport.power.enable";

    public static String USER_EXPERIMENT_ENABLE_HBM = "user_experiment.hbm.simulate.enable";
    public static String USER_EXPERIMENT_HBM_INTERVAL_SECS = "user_experiment.hbm.simulate.intervalSecs";
    public static String USER_EXPERIMENT_HBM_ENABLE_HEART_RATE = "user_experiment.hbm.simulate.heartRate.enable";
    public static String USER_EXPERIMENT_HBM_ENABLE_SKIN_TEMP = "user_experiment.hbm.simulate.skinTemp.enable";
    public static String USER_EXPERIMENT_HBM_ENABLE_CORE_TEMP = "user_experiment.hbm.simulate.coreTemp.enable";
    public static String USER_EXPERIMENT_HBM_ENABLE_BLOOD_OXY = "user_experiment.hbm.simulate.bloodOxygenationPerc.enable";
    public static String USER_EXPERIMENT_HBM_ENABLE_BLOOD_HYDRO = "user_experiment.hbm.simulate.bloodHydrationPerc.enable";
    public static String USER_EXPERIMENT_HBM_ENABLE_FATIGUE_LEVEL = "user_experiment.hbm.simulate.fatigueLevel.enable";
    public static String USER_EXPERIMENT_HBM_ENABLE_TASK_EFFECTIVENESS_LEVEL = "user_experiment.hbm.simulate.taskEffectivenessLevel.enable";

    public static String CONFIGHUB_BASE_URL = "confighub.baseUrl";

    public static String USER_BT_DEVICE_USE = "user_bt.use";
    public static String USER_BT_DEVICE_ADDRESS = "user_bt.deviceAddress";

    public static String USER_LICENSING_KEY = "user_licensing.key";
    public static String USER_LICENSING_ACTIVATION_CODE = "user_licensing.activationCode";

    public static String DEVELOPER_MODE_ACTIVE = "developer_modeActive";
    public static String DEVELOPER_USE_DEV_LICENSING_SYSTEM = "developer_useDevLicensingSystem";
}
