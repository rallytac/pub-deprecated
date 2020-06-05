//
//  Copyright (c) 2019 Rally Tactical Systems, Inc.
//  All rights reserved.
//

package com.rallytac.engage.engine;

import android.content.Context;
import android.net.nsd.NsdManager;
import android.net.nsd.NsdServiceInfo;
import android.os.Handler;
import android.support.annotation.Keep;
import android.media.AudioDeviceInfo;
import android.media.AudioManager;

import org.json.JSONObject;
import org.json.JSONArray;

import java.net.InetAddress;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.UUID;

import java.net.Inet4Address;
import java.net.Inet6Address;
import java.net.NetworkInterface;
import java.util.Enumeration;

public final class Engine
{
    public enum CreationStatus
    {
        csUndefined(0),
        csOk(1),
        csNoJson(-1),
        csConflictingRpListAndCluster(-2),
        csAlreadyExists(-3),
        csInvalidConfiguration(-4),
        csInvalidJson(-5),
        csCryptoFailure(-6),
        csAudioInputFailure(-7),
        csAudioOutputFailure(-8),
        csUnsupporttedAudioEncoder(-9),
        ;

        private final int _val;

        private CreationStatus(int val)
        {
            this._val = val;
        }

        public static CreationStatus fromInt(int i)
        {
            for (CreationStatus e : values())
            {
                if (e._val == i)
                {
                    return e;
                }
            }

            return null;
        }

        public static int toInt(CreationStatus e)
        {
            return e.toInt();
        }

        public int toInt()
        {
            return _val;
        }
    }

    public enum TxStatus
    {
        undefined(0),
        started(1),
        ended(2),
        notAnAudioGroup(-1),
        notJoined(-2),
        notConnected(-3),
        alreadyTransmitting(-4),
        invalidParams(-5),
        priorityTooLow(-6),
        rxActiveOnNonFdx(-7),
        cannotSubscribeToMic(-8),
        invalidId(-9),
        ;

        private final int _val;

        private TxStatus(int val)
        {
            this._val = val;
        }

        public static TxStatus fromInt(int i)
        {
            for (TxStatus e : values())
            {
                if (e._val == i)
                {
                    return e;
                }
            }

            return null;
        }

        public static int toInt(TxStatus e)
        {
            return e.toInt();
        }

        public int toInt()
        {
            return _val;
        }
    }

    public enum ConnectionType
    {
        undefined(0),
        ipMulticast(1),
        rallypoint(2)
        ;

        private final int _val;

        private ConnectionType(int val)
        {
            this._val = val;
        }

        public static ConnectionType fromInt(int i)
        {
            for (ConnectionType e : values())
            {
                if (e._val == i)
                {
                    return e;
                }
            }

            return null;
        }

        public static int toInt(ConnectionType e)
        {
            return e.toInt();
        }

        public int toInt()
        {
            return _val;
        }
    }

    public enum LicenseType
    {
        unknown(0),
        perpetual(1),
        expires(2)
        ;

        private final int _val;

        private LicenseType(int val)
        {
            this._val = val;
        }

        public static LicenseType fromInt(int i)
        {
            for (LicenseType e : values())
            {
                if (e._val == i)
                {
                    return e;
                }
            }

            return null;
        }

        public static int toInt(LicenseType e)
        {
            return e.toInt();
        }

        public int toInt()
        {
            return _val;
        }
    }

    public enum LicensingStatusCode
    {
        ok(0),
        nullEntitlementKey(-1),
        nullLicenseKey(-2),
        invaludLicenseKeyLen(-3),
        licenseKeyVerificationFailure(-4),
        activationCodeVerificationFailure(-5),
        invalidExpirationDate(-6),
        generalFailure(-7),
        notInitialized(-8),
        requiresActivation(-9)
        ;

        private final int _val;

        private LicensingStatusCode(int val)
        {
            this._val = val;
        }

        public static LicensingStatusCode fromInt(int i)
        {
            for (LicensingStatusCode e : values())
            {
                if (e._val == i)
                {
                    return e;
                }
            }

            return null;
        }

        public static int toInt(LicensingStatusCode e)
        {
            return e.toInt();
        }

        public int toInt()
        {
            return _val;
        }
    }

    public enum NetworkDeviceFamily
    {
        ipv4(2),
        ipv6(30)
        ;

        private final int _val;

        private NetworkDeviceFamily(int val)
        {
            this._val = val;
        }

        public static NetworkDeviceFamily fromInt(int i)
        {
            for (NetworkDeviceFamily e : values())
            {
                if (e._val == i)
                {
                    return e;
                }
            }

            return null;
        }

        public static int toInt(NetworkDeviceFamily e)
        {
            return e.toInt();
        }

        public int toInt()
        {
            return _val;
        }
    }

    public enum NetworkTxPriority
    {
        priBestEffort(0),
        priSignaling(2),
        priVideo(3),
        priVoice(4),
        ;

        private final int _val;

        private NetworkTxPriority(int val)
        {
            this._val = val;
        }

        public static NetworkTxPriority fromInt(int i)
        {
            for (NetworkTxPriority e : values())
            {
                if (e._val == i)
                {
                    return e;
                }
            }

            return null;
        }

        public static int toInt(NetworkTxPriority e)
        {
            return e.toInt();
        }

        public int toInt()
        {
            return _val;
        }
    }

    public enum BlobType
    {
        undefined(0),
        appTextUtf8(1),
        jsonTextUtf8(2),
        appBinary(3),
        engageHumanBiometrics(4)
        ;

        private final int _val;

        private BlobType(int val)
        {
            this._val = val;
        }

        public static BlobType fromInt(int i)
        {
            for (BlobType e : values())
            {
                if (e._val == i)
                {
                    return e;
                }
            }

            return null;
        }

        public static int toInt(BlobType e)
        {
            return e.toInt();
        }

        public int toInt()
        {
            return _val;
        }
    }

    public enum HumanBiometricsElement
    {
        undefined(0),
        heartRate(1),
        skinTemp(2),
        coreTemp(3),
        hydration(4),
        bloodOxygenation(5),
        fatigueLevel(6),
        taskEffectiveness(7)
        ;

        private final int _val;

        private HumanBiometricsElement(int val)
        {
            this._val = val;
        }

        public static HumanBiometricsElement fromInt(int i)
        {
            for (HumanBiometricsElement e : values())
            {
                if (e._val == i)
                {
                    return e;
                }
            }

            return null;
        }

        public static int toInt(HumanBiometricsElement e)
        {
            return e.toInt();
        }

        public int toInt()
        {
            return _val;
        }
    }

    // Group sources
    public static final String GROUP_SOURCE_ENGAGE_INTERNAL = "com.rallytac.engage.internal";
    public static final String GROUP_SOURCE_ENGAGE_MAGELLAN_CISTECH = "com.rallytac.engage.magellan.cistech";
    public static final String GROUP_SOURCE_ENGAGE_MAGELLAN_TRELLISWARE = "com.rallytac.engage.magellan.trellisware";
    public static final String GROUP_SOURCE_ENGAGE_MAGELLAN_SILVUS = "com.rallytac.engage.magellan.silvus";
    public static final String GROUP_SOURCE_ENGAGE_MAGELLAN_PERSISTENT = "com.rallytac.engage.magellan.persistent";
    public static final String GROUP_SOURCE_ENGAGE_MAGELLAN_DOMO = "com.rallytac.engage.magellan.domo";
    public static final String GROUP_SOURCE_ENGAGE_MAGELLAN_KENWOOD = "com.rallytac.engage.magellan.kenwood";
    public static final String GROUP_SOURCE_ENGAGE_MAGELLAN_TAIT = "com.rallytac.engage.magellan.tait";

    // Group disconnected reasons
    public static final String GROUP_DISCONNECTED_REASON_NO_REAON = "NoReason";
    public static final String GROUP_DISCONNECTED_REASON_NO_LINK = "NoLink";
    public static final String GROUP_DISCONNECTED_REASON_UNREGISTERED = "Unregistered";
    public static final String GROUP_DISCONNECTED_REASON_NOT_ALLOWED = "NotAllowed";
    public static final String GROUP_DISCONNECTED_REASON_GENERAL_DENIAL = "GeneralDenial";

    public final class JsonFields
    {
        public final class GroupCreationDetail
        {
            public static final String objectName = "groupCreationDetail";
            public static final String id = "id";
            public static final String status = "status";
        }

        public final class GroupTxDetail
        {
            public static final String objectName = "groupTxDetail";
            public static final String id = "id";
            public static final String status = "status";
            public static final String localPriority = "localPriority";
            public static final String remotePriority = "remotePriority";
            public static final String nonFdxMsHangRemaining = "nonFdxMsHangRemaining";
        }

        public final class RallypointConnectionDetail
        {
            public static final String objectName = "rallypointConnectionDetail";
            public static final String internalId = "internalId";
            public static final String host = "host";
            public static final String port = "port";
            public static final String msToNextConnectionAttempt = "msToNextConnectionAttempt";
        }

        public final class GroupConnectionDetail
        {
            public static final String objectName = "groupConnectionDetail";
            public static final String id = "id";
            public static final String connectionType = "connectionType";
            public static final String peer = "peer";
            public static final String asFailover = "asFailover";
            public static final String reason = "reason";
        }

        public final class CertStoreCertificateElement
        {
            public static final String objectName = "certStoreCertificateElement";
            public static final String arrayName = "certificates";
            public static final String id = "id";
            public static final String hasPrivateKey = "hasPrivateKey";
        }

        public final class CertStoreDescriptor
        {
            public static final String objectName = "certStoreDescriptor";
            public static final String fileName = "fileName";
            public static final String version = "version";
            public static final String flags = "flags";
            public static final String certificates = "certificates";
        }

        public final class ListOfAudioDevice
        {
            public static final String objectName = "list";
        }

        public final class AudioDevice
        {
            public static final String objectName = "audioDevice";
            public static final String deviceId = "deviceId";
            public static final String samplingRate = "samplingRate";
            public static final String msPerBuffer = "msPerBuffer";
            public static final String channels = "channels";
            public static final String direction = "direction";
            public static final String boostPercentage = "boostPercentage";
            public static final String isAdad = "isAdad";
            public static final String name = "name";
            public static final String manufacturer = "manufacturer";
            public static final String model = "model";
            public static final String hardwareId = "hardwareId";
            public static final String serialNumber = "serialNumber";
            public static final String isDefault = "isDefault";
            public static final String extra = "extra";
            public static final String type = "type";
        }

        public final class ListOfNetworkInterfaceDevice
        {
            public static final String objectName = "list";
        }

        public final class NetworkInterfaceDevice
        {
            public static final String objectName = "networkInterfaceDevice";
            public static final String name = "name";
            public static final String friendlyName = "friendlyName";
            public static final String description = "description";
            public static final String family = "family";
            public static final String address = "address";
            public static final String available = "available";
            public static final String isLoopback = "isLoopback";
            public static final String supportsMulticast = "supportsMulticast";
            public static final String hardwareAddress = "hardwareAddress";
        }

        public final class AdvancedTxParams
        {
            public static final String objectName = "advancedTxParams";
            public static final String flags = "flags";
            public static final String priority = "priority";
            public static final String subchannelTag = "subchannelTag";
            public static final String includeNodeId = "includeNodeId";
            public static final String alias = "alias";
            public static final String muted = "muted";
        }

        public final class License
        {
            public static final String objectName = "license";
            public static final String entitlement = "entitlement";
            public static final String key = "key";
            public static final String activationCode = "activationCode";
            public static final String deviceId = "deviceId";
            public static final String type = "type";
            public static final String expires = "expires";
            public static final String expiresFormatted = "expiresFormatted";
            public static final String status = "status";
            public static final String manufacturerId = "manufacturerId";
        }

        public final class TalkerInformation
        {
            public static final String objectName = "talkerInformation";
            public static final String alias = "alias";
            public static final String nodeId = "nodeId";
        }

        public final class GroupTalkers
        {
            public static final String objectName = "GroupTalkers";
            public static final String list = "list";
        }

        public final class EnginePolicy
        {
            public final class Database
            {
                public static final String objectName = "database";
                public static final String enabled = "enabled";
                public static final String type = "type";
                public static final String fixedFileName = "fixedFileName";
            }

            public final class Internals
            {
                public static final String objectName = "internals";
                public static final String disableWatchdog = "disableWatchdog";
                public static final String watchdogIntervalMs = "watchdogIntervalMs";
                public static final String watchdogHangDetectionMs = "watchdogHangDetectionMs";
                public static final String housekeeperIntervalMs = "housekeeperIntervalMs";
                public static final String logTaskQueueStatsIntervalMs = "logTaskQueueStatsIntervalMs";
                public static final String maxTxSecs = "maxTxSecs";
                public static final String maxRxSecs = "maxRxSecs";
                public static final String enableLazySpeakerClosure = "enableLazySpeakerClosure";
            }

            public final class Timelines
            {
                public static final String objectName = "timelines";
                public static final String enabled = "enabled";
                public static final String maxAttachmentQuotaMb = "maxAttachmentQuotaMb";
                public static final String maxEventAgeSecs = "maxEventAgeSecs";
	            public static final String storageRoot = "storageRoot";
                public static final String maxStorageQuotaMb = "maxStorageQuotaMb";
	            public static final String maxEvents = "maxEvents";
                public static final String groomingIntervalSecs = "groomingIntervalSecs";
                public static final String autosaveIntervalSecs = "autosaveIntervalSecs";
                public static final String disableSigningAndVerification = "disableSigningAndVerification";
            }

            public final class Security
            {
                public static final String objectName = "security";
            }

            public final class Certificate
            {
                public static final String objectName = "certificate";
                public static final String certificate = "certificate";
                public static final String key = "key";
            }

            public final class Licensing
            {
                public static final String objectName = "licensing";
                public static final String entitlement = "entitlement";
                public static final String key = "key";
                public static final String activationCode = "activationCode";
                public static final String manufacturerId = "manufacturerId";
            }

            public final class Networking
            {
                public static final String objectName = "networking";
                public static final String defaultNic = "defaultNic";
                public static final String maxOutputQueuePackets = "maxOutputQueuePackets";
                public static final String rtpJitterMinMs = "rtpJitterMinMs";
                public static final String rtpJitterMaxMs = "rtpJitterMaxMs";
                public static final String rtpLatePacketSequenceRange = "rtpLatePacketSequenceRange";
                public static final String rtpLatePacketTimestampRangeMs = "rtpLatePacketTimestampRangeMs";
                public static final String rtpInboundProcessorInactivityMs = "rtpInboundProcessorInactivityMs";
                public static final String multicastRejoinSecs = "multicastRejoinSecs";
                public static final String rpLeafConnectTimeoutSecs = "rpLeafConnectTimeoutSecs";
                public static final String maxReconnectPauseMs = "maxReconnectPauseMs";
                public static final String reconnectFailurePauseIncrementMs = "reconnectFailurePauseIncrementMs";
                public static final String sendFailurePauseMs = "sendFailurePauseMs";
                public static final String rallypointRtTestIntervalMs = "rallypointRtTestIntervalMs";
                public static final String logRtpJitterBufferStats = "logRtpJitterBufferStats";
            }

            public final class Audio
            {
                public static final String objectName = "audio";
                public static final String internalRate = "internalRate";
                public static final String internalChannels = "internalChannels";
                public static final String inputRate = "inputRate";
                public static final String inputChannels = "inputChannels";
                public static final String inputBufferMs = "inputBufferMs";
                public static final String outputRate = "outputRate";
                public static final String outputChannels = "outputChannels";
                public static final String outputBufferMs = "outputBufferMs";
                public static final String outputGainPercentage = "outputGainPercentage";
                public static final String allowOutputOnTransmit = "allowOutputOnTransmit";
                public static final String muteTxOnTx = "muteTxOnTx";

                public final class Aec
                {
                    public static final String objectName = "aec";
                    public static final String enabled = "enabled";
                    public static final String mode = "mode";
                    public static final String speakerTailMs = "speakerTailMs";
                    public static final String cng = "cng";
                }

                public final class Vad
                {
                    public static final String objectName = "vad";
                    public static final String enabled = "enabled";
                    public static final String mode = "mode";
                }
            }

            public final class Discovery
            {
                public static final String objectName = "discovery";

                public final class Ssdp
                {
                    public static final String objectName = "ssdp";
                    public static final String enabled = "enabled";
                    public static final String ageTimeoutMs = "ageTimeoutMs";
                    public static final String address = "address";
                }

                public final class Cistech
                {
                    public static final String objectName = "cistech";
                    public static final String enabled = "enabled";
                    public static final String ageTimeoutMs = "ageTimeoutMs";
                    public static final String address = "address";
                }

                public final class Trellisware
                {
                    public static final String objectName = "trellisware";
                    public static final String enabled = "enabled";
                }
            }

            public static final String dataDirectory = "dataDirectory";
        }

        public final class Mission
        {
            public static final String id = "id";
            public static final String name = "name";
            public static final String description = "description";
            public static final String modPin = "modPin";
        }

        public final class Rallypoint
        {
            public static final String objectName = "rallypoint";
            public static final String arrayName = "rallypoints";

            public final class Host
            {
                public static final String objectName = "host";
                public static final String address = "address";
                public static final String port = "port";
            }

            public static final String certificate = "certificate";
            public static final String certificateKey = "certificateKey";
            public static final String verifyPeer = "verifyPeer";
            public static final String allowSelfSignedCertificate = "allowSelfSignedCertificate";
            public static final String transactionTimeoutMs = "transactionTimeoutMs";
            public static final String disableMessageSigning = "disableMessageSigning";
            public static final String use = "use";
        }

        public final class Address
        {
            public static final String objectName = "address";
            public static final String address = "address";
            public static final String port = "port";
        }

        public final class Rx
        {
            public static final String objectName = "rx";
            public static final String address = "address";
            public static final String port = "port";
        }

        public final class Tx
        {
            public static final String objectName = "tx";
            public static final String address = "address";
            public static final String port = "port";
        }

        public final class Group
        {
            public static final String objectName = "group";
            public static final String arrayName = "groups";
            public static final String id = "id";
            public static final String name = "name";
            public static final String type = "type";
            public static final String source = "source";
            public static final String cryptoPassword = "cryptoPassword";
            public static final String fdx = "fdx";
            public static final String alias = "alias";
            public static final String maxRxSecs = "maxRxSecs";
        }

        public final class TxAudio
        {
            public static final String objectName = "txAudio";
            public static final String fdx = "fdx";
            public static final String encoder = "encoder";
            public static final String framingMs = "framingMs";
            public static final String maxTxSecs = "maxTxSecs";
            public static final String noHdrExt = "noHdrExt";
        }

        public final class NetworkTxOptions
        {
            public static final String objectName = "txOptions";
            public static final String priority = "priority";
            public static final String ttl = "ttl";
        }

        public final class Presence
        {
            public static final String objectName = "presence";
            public static final String format = "format";
            public static final String intervalSecs = "intervalSecs";
            public static final String forceOnAudioTransmit = "forceOnAudioTransmit";
            public static final String listenOnly = "listenOnly";
        }

        public final class PresenceDescriptor
        {
            public static final String objectName = "presence";
            public static final String self = "self";
            public static final String comment = "comment";
            public static final String custom = "custom";

            public class GroupAlias
            {
                public static final String arrayName = "groupAliases";
                public static final String groupId = "groupId";
                public static final String alias = "alias";
            }
        }

        public final class Identity
        {
            public static final String objectName = "identity";
            public static final String nodeId = "nodeId";
            public static final String userId = "userId";
            public static final String displayName = "displayName";
            public static final String type = "type";
            public static final String format = "format";
            public static final String avatar = "avatar";
        }

        public final class Location
        {
            public static final String objectName = "location";
            public static final String longitude = "longitude";
            public static final String latitude = "latitude";
            public static final String altitude = "altitude";
            public static final String direction = "direction";
            public static final String speed = "speed";
        }

        public final class Connectivity
        {
            public static final String objectName = "connectivity";
            public static final String type = "type";
            public static final String strength = "strength";
            public static final String rating = "rating";
        }

        public final class Power
        {
            public static final String objectName = "power";
            public static final String source = "source";
            public static final String state = "state";
            public static final String level = "level";
        }

        public final class RtpHeader
        {
            public static final String objectName = "rtpHeader";
            public static final String pt = "pt";
            public static final String marker = "marker";
            public static final String seq = "seq";
            public static final String ssrc = "ssrc";
            public static final String ts = "ts";
        }

        public final class BlobHeader
        {
            public static final String objectName = "blobHeader";
            public static final String source = "source";
            public static final String target = "target";
            public static final String payloadType = "payloadType";
            public static final String blobSize = "size";
        }

        public final class TimelineEvent
        {
            public final class Audio
            {
                public static final String objectName = "audio";
                public static final String ms = "ms";
                public static final String samples = "samples";
            }

            public static final String objectName = "event";
            public static final String alias = "alias";
            public static final String direction = "direction";
            public static final String ended = "ended";
            public static final String groupId = "groupId";
            public static final String id = "id";
            public static final String inProgress = "inProgress";
            public static final String nodeId = "nodeId";
            public static final String started = "started";
            public static final String thisNodeId = "thisNodeId";
            public static final String type = "type";
            public static final String uri = "uri";
        }

        public final class TimelineQuery
        {
            public static final String maxCount = "maxCount";
            public static final String mostRecentFirst = "mostRecentFirst";
            public static final String startedOnOrAfter = "startedOnOrAfter";
            public static final String endedOnOrBefore = "endedOnOrBefore";
            public static final String onlyDirection = "onlyDirection";
            public static final String onlyType = "onlyType";
            public static final String onlyCommitted = "onlyCommitted";
            public static final String onlyAlias = "onlyAlias";
            public static final String onlyNodeId = "onlyNodeId";
            public static final String sql = "sql";
        }

        public final class TimelineReport
        {
            public static final String success = "success";
            public static final String errorMessage = "errorMessage";
            public static final String started = "started";
            public static final String ended = "ended";
            public static final String execMs = "execMs";
            public static final String records = "records";
            public static final String events = "events";
            public static final String count = "count";
        }
    }

    private final static String TAG = Engine.class.getSimpleName();

    public static String bytesToHexString(byte[] bytes)
    {
        if(bytes == null || bytes.length == 0)
        {
            return null;
        }

        StringBuffer hs = new StringBuffer();
        for (int x = 0; x < bytes.length; x++)
        {
            String s = Integer.toHexString(0xFF & bytes[x]);
            if(s.length() == 1)
            {
                s = "0" + s;
            }

            hs.append(s);
        }

        return hs.toString().toUpperCase();
    }

    private Handler _handler = null;
    private static Context _appContext = null;

    // IMPORTANT:  Call this as soon as the application context has been created!
    public static void setApplicationContext(Context appContext)
    {
        _appContext = appContext;
    }

    public void initialize()
    {
        _handler = new Handler();
    }

    public void deinitialize()
    {
    }

    @Keep
    public interface IEngineListener
    {
        void onEngineStarted(String eventExtraJson);
        void onEngineStartFailed(String eventExtraJson);
        void onEngineStopped(String eventExtraJson);
    }

    @Keep
    public interface IRallypointListener
    {
        void onRallypointPausingConnectionAttempt(String id, String eventExtraJson);
        void onRallypointConnecting(String id, String eventExtraJson);
        void onRallypointConnected(String id, String eventExtraJson);
        void onRallypointDisconnected(String id, String eventExtraJson);
        void onRallypointRoundtripReport(String id, long rtMs, long rtQualityRating, String eventExtraJson);
    }

    @Keep
    public interface IGroupListener
    {
        void onGroupCreated(String id, String eventExtraJson);
        void onGroupCreateFailed(String id, String eventExtraJson);
        void onGroupDeleted(String id, String eventExtraJson);
        void onGroupConnected(String id, String eventExtraJson);
        void onGroupConnectFailed(String id, String eventExtraJson);
        void onGroupDisconnected(String id, String eventExtraJson);
        void onGroupJoined(String id, String eventExtraJson);
        void onGroupJoinFailed(String id, String eventExtraJson);
        void onGroupLeft(String id, String eventExtraJson);
        void onGroupMemberCountChanged(String id, long count, String eventExtraJson);
        void onGroupRxStarted(String id, String eventExtraJson);
        void onGroupRxEnded(String id, String eventExtraJson);
        void onGroupRxSpeakersChanged(String id, String groupTalkerJson, String eventExtraJson);
        void onGroupRxMuted(String id, String eventExtraJson);
        void onGroupRxUnmuted(String id, String eventExtraJson);
        void onGroupTxMuted(String id, String eventExtraJson);
        void onGroupTxUnmuted(String id, String eventExtraJson);
        void onGroupTxStarted(String id, String eventExtraJson);
        void onGroupTxEnded(String id, String eventExtraJson);
        void onGroupTxFailed(String id, String eventExtraJson);
        void onGroupTxUsurpedByPriority(String id, String eventExtraJson);
        void onGroupMaxTxTimeExceeded(String id, String eventExtraJson);
        void onGroupNodeDiscovered(String id, String nodeJson, String eventExtraJson);
        void onGroupNodeRediscovered(String id, String nodeJson, String eventExtraJson);
        void onGroupNodeUndiscovered(String id, String nodeJson, String eventExtraJson);
        void onGroupAssetDiscovered(String id, String nodeJson, String eventExtraJson);
        void onGroupAssetRediscovered(String id, String nodeJson, String eventExtraJson);
        void onGroupAssetUndiscovered(String id, String nodeJson, String eventExtraJson);
        void onGroupBlobSent(String id, String eventExtraJson);
        void onGroupBlobSendFailed(String id, String eventExtraJson);
        void onGroupBlobReceived(String id, String blobInfoJson, byte[] blob, long blobSize, String eventExtraJson);
        void onGroupRtpSent(String id, String eventExtraJson);
        void onGroupRtpSendFailed(String id, String eventExtraJson);
        void onGroupRtpReceived(String id, String blobInfoJson, byte[] payload, long payloadSize, String eventExtraJson);
        void onGroupRawSent(String id, String eventExtraJson);
        void onGroupRawSendFailed(String id, String eventExtraJson);
        void onGroupRawReceived(String id, byte[] raw, long rawSize, String eventExtraJson);

        void onGroupTimelineEventStarted(String id, String eventJson, String eventExtraJson);
        void onGroupTimelineEventUpdated(String id, String eventJson, String eventExtraJson);
        void onGroupTimelineEventEnded(String id, String eventJson, String eventExtraJson);
        void onGroupTimelineReport(String id, String reportJson, String eventExtraJson);
        void onGroupTimelineReportFailed(String id, String eventExtraJson);
        void onGroupTimelineGroomed(String id, String eventJson, String eventExtraJson);       

        void onGroupHealthReport(String id, String healthReportJson, String eventExtraJson);
        void onGroupHealthReportFailed(String id, String eventExtraJson);
    }

    @Keep
    public interface ILicenseListener
    {
        void onLicenseChanged(String eventExtraJson);
        void onLicenseExpired(String eventExtraJson);
        void onLicenseExpiring(double secondsLeft, String eventExtraJson);
    }

    // Listeners
    private ArrayList<IEngineListener> _engineListeners = new ArrayList<>();
    private ArrayList<IRallypointListener> _rallypointListeners = new ArrayList<>();
    private ArrayList<IGroupListener> _groupListeners = new ArrayList<>();
    private ArrayList<ILicenseListener> _licenseListeners = new ArrayList<>();

    // Internals
    private MdnsDiscoverer _mdsnDiscoverer = null;

    private class MdnsDiscoverer
    {
        private class DiscoveredEntity
        {
            String _id;
            String _type;
            String _name;

            public JSONObject makeJsonObject(String address, int port)
            {
                JSONObject root = new JSONObject();
                JSONObject addr = new JSONObject();

                try
                {
                    root.put("id", _id);
                    root.put("type", _type);
                    root.put("name", _name);

                    addr.put("address", address);
                    addr.put("port", port);
                    root.put("address", addr);
                }
                catch (Exception e)
                {
                    e.printStackTrace();
                    root = null;
                }

                return root;
            }
        }

        private NsdManager _nsdManager = null;
        private NsdManager.ResolveListener _resolveListener;

        private HashMap<String, NsdManager.DiscoveryListener> _registeredTypes = new HashMap<>();
        private ArrayList<DiscoveredEntity> _discoveredEntities = new ArrayList<>();


        public void addListener(final String serviceType)
        {
            _handler.post(new Runnable()
            {
                @Override
                public void run()
                {
                    String ast = unadornedServiceType(serviceType);

                    if( _registeredTypes.containsKey(ast))
                    {
                        return;
                    }

                    if (_nsdManager == null)
                    {

                        _nsdManager = (NsdManager) _appContext.getSystemService(Context.NSD_SERVICE);
                        initializeResolveListener();
                    }

                    NsdManager.DiscoveryListener dl = createDiscoveryListener();
                    _registeredTypes.put(ast, dl);

                    _nsdManager.discoverServices(ast, NsdManager.PROTOCOL_DNS_SD, dl);
                }
            });
        }


        public void removeListener(final String serviceType)
        {
            _handler.post(new Runnable()
            {
                @Override
                public void run()
                {
                    String ast = unadornedServiceType(serviceType);

                    if(_registeredTypes.containsKey(unadornedServiceType(ast)))
                    {
                        NsdManager.DiscoveryListener dl = _registeredTypes.get(unadornedServiceType(ast));
                        _registeredTypes.remove(ast);
                        _nsdManager.stopServiceDiscovery(dl);
                    }

                    clearAllDiscoveredEntitiesForType(ast);
                }
            });
        }

        private String unadornedServiceType(String serviceType)
        {
            String rc = serviceType;


            if(rc.endsWith("."))
            {
                rc = rc.substring(0, rc.length() - 1);
            }

            if(rc.startsWith("."))
            {
                rc = rc.substring(1);
            }

            return rc;
        }

        private String inetAddressAsString(InetAddress addr)
        {
            String rc = addr.toString();
            if(rc.startsWith("/"))
            {
                rc = rc.substring(1);
            }

            return rc;
        }

        private void clearAllDiscoveredEntities()
        {
            _handler.post(new Runnable()
            {
                @Override
                public void run()
                {
                    for(DiscoveredEntity de : _discoveredEntities)
                    {
                        engagePlatformServiceUndiscovered(de._id);
                    }

                    _discoveredEntities.clear();
                }
            });
        }

        private void clearAllDiscoveredEntitiesForType(final String serviceType)
        {
            _handler.post(new Runnable()
            {
                @Override
                public void run()
                {
                    String ast = unadornedServiceType(serviceType);

                    ArrayList<DiscoveredEntity> trash = new ArrayList<>();

                    for(DiscoveredEntity de : _discoveredEntities)
                    {
                        if(de._type.compareTo(ast) == 0)
                        {
                            trash.add(de);
                        }
                    }

                    for(DiscoveredEntity de : trash)
                    {
                        engagePlatformServiceUndiscovered(de._id);
                        _discoveredEntities.remove(de);
                    }

                    trash.clear();
                }
            });
        }

        private void clearDiscoveredEntity(final String serviceType, final String serviceName)
        {
            _handler.post(new Runnable()
            {
                @Override
                public void run()
                {
                    String ast = unadornedServiceType(serviceType);

                    ArrayList<DiscoveredEntity> trash = new ArrayList<>();

                    for(DiscoveredEntity de : _discoveredEntities)
                    {
                        if(de._type.compareTo(ast) == 0 && de._name.compareTo(serviceName) == 0)
                        {
                            trash.add(de);
                        }
                    }

                    for(DiscoveredEntity de : trash)
                    {
                        engagePlatformServiceUndiscovered(de._id);
                        _discoveredEntities.remove(de);
                    }

                    trash.clear();
                }
            });
        }

        private void cleanup()
        {
            _handler.post(new Runnable()
            {
                @Override
                public void run()
                {
                    for(NsdManager.DiscoveryListener dl : _registeredTypes.values())
                    {
                        _nsdManager.stopServiceDiscovery(dl);
                    }

                    _registeredTypes.clear();

                    clearAllDiscoveredEntities();

                    _nsdManager = null;
                }
            });
        }

        private NsdManager.DiscoveryListener getListenerForServiceType(String serviceType)
        {
            String ast = unadornedServiceType(serviceType);

            if(!_registeredTypes.containsKey(ast))
            {
                return null;
            }

            return _registeredTypes.get(ast);
        }

        private DiscoveredEntity getDiscoveredEntity(String serviceType, String serviceName)
        {
            String ast = unadornedServiceType(serviceType);

            for(DiscoveredEntity de : _discoveredEntities)
            {
                if(de._type.compareTo(ast) == 0 && de._name.compareTo(serviceName) == 0)
                {
                    return de;
                }
            }

            return null;
        }

        private NsdManager.DiscoveryListener createDiscoveryListener()
        {
            NsdManager.DiscoveryListener rc = new NsdManager.DiscoveryListener()
            {
                // Called as soon as service discovery begins.
                @Override
                public void onDiscoveryStarted(final String regType)
                {
                    _handler.post(new Runnable()
                    {
                        @Override
                        public void run()
                        {
                            //Log.d(TAG, ">>>>==== Service discovery started");
                        }
                    });

                }

                @Override
                public void onServiceFound(final NsdServiceInfo service)
                {
                    _handler.post(new Runnable()
                    {
                        @Override
                        public void run()
                        {
                            //Log.d(TAG, ">>>>==== Service discovery success " + service);

                            String ast = unadornedServiceType(service.getServiceType());

                            NsdManager.DiscoveryListener dl = getListenerForServiceType(ast);
                            if(dl != null)
                            {
                                _nsdManager.resolveService(service, _resolveListener);
                            }
                        }
                    });
                }

                @Override
                public void onServiceLost(final NsdServiceInfo service)
                {
                    _handler.post(new Runnable()
                    {
                        @Override
                        public void run()
                        {
                            //Log.e(TAG, ">>>>==== Service lost: " + service);

                            clearDiscoveredEntity(service.getServiceType(), service.getServiceName());
                        }
                    });
                }

                @Override
                public void onDiscoveryStopped(final String serviceType)
                {
                    _handler.post(new Runnable()
                    {
                        @Override
                        public void run()
                        {
                            //Log.i(TAG, ">>>>==== Discovery stopped: " + serviceType);

                            clearAllDiscoveredEntitiesForType(serviceType);
                        }
                    });
                }

                @Override
                public void onStartDiscoveryFailed(final String serviceType, final int errorCode)
                {
                    _handler.post(new Runnable()
                    {
                        @Override
                        public void run()
                        {
                            //Log.e(TAG, ">>>>==== Discovery failed: Error code:" + errorCode);

                            clearAllDiscoveredEntitiesForType(serviceType);

                            NsdManager.DiscoveryListener dl = getListenerForServiceType(unadornedServiceType(serviceType));
                            if(dl != null)
                            {
                                _nsdManager.stopServiceDiscovery(dl);
                            }
                        }
                    });
                }

                @Override
                public void onStopDiscoveryFailed(final String serviceType, final int errorCode)
                {
                    _handler.post(new Runnable()
                    {
                        @Override
                        public void run()
                        {
                            //Log.e(TAG, ">>>>==== Discovery failed: Error code:" + errorCode);
                        }
                    });
                }
            };


            return rc;
        }

        private void initializeResolveListener()
        {
            _resolveListener = new NsdManager.ResolveListener()
            {
                @Override
                public void onResolveFailed(final NsdServiceInfo serviceInfo, final int errorCode)
                {
                    _handler.post(new Runnable()
                    {
                        @Override
                        public void run()
                        {
                            //Log.e(TAG, ">>>>==== Resolve failed: " + errorCode);
                        }
                    });
                }

                @Override
                public void onServiceResolved(final NsdServiceInfo serviceInfo)
                {
                    _handler.post(new Runnable()
                    {
                        @Override
                        public void run()
                        {
                            //Log.e(TAG, ">>>>==== Resolve Succeeded. " + serviceInfo);

                            String ast = unadornedServiceType(serviceInfo.getServiceType());
                            int port = serviceInfo.getPort();
                            String address = inetAddressAsString(serviceInfo.getHost());

                            DiscoveredEntity de = getDiscoveredEntity(ast, serviceInfo.getServiceName());
                            if(de == null)
                            {
                                de = new DiscoveredEntity();
                                de._id = "{" + UUID.randomUUID().toString() + "}";
                                de._type = ast;
                                de._name = serviceInfo.getServiceName();
                                _discoveredEntities.add(de);

                                JSONObject obj = de.makeJsonObject(address, port);
                                if(obj != null)
                                {
                                    engagePlatformServiceDiscovered(de._id, obj.toString());
                                }
                            }
                            else
                            {
                                JSONObject obj = de.makeJsonObject(address, port);
                                if(obj != null)
                                {
                                    engagePlatformServiceRediscovered(de._id, obj.toString());
                                }
                            }
                        }
                    });
                }
            };
        }
    }

    @Keep
    public void addEngineListener(final IEngineListener listener)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                {
                    _engineListeners.add(listener);
                }
            }
        });
    }

    @Keep
    public void removeEngineListener(final IEngineListener listener)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                _engineListeners.remove(listener);
            }
        });
    }

    @Keep
    public void addRallypointListener(final IRallypointListener listener)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                _rallypointListeners.add(listener);
            }
        });
    }

    @Keep
    public void removeRallypointListener(final IRallypointListener listener)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                _rallypointListeners.remove(listener);
            }
        });
    }

    @Keep
    public void addGroupListener(final IGroupListener listener)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                _groupListeners.add(listener);
            }
        });
    }

    @Keep
    public void removeGroupListener(final IGroupListener listener)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                _groupListeners.remove(listener);
            }
        });
    }

    @Keep
    public void addLicenseListener(final ILicenseListener listener)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                _licenseListeners.add(listener);
            }
        });
    }

    @Keep
    public void removeLicenseListener(final ILicenseListener listener)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                _licenseListeners.remove(listener);
            }
        });
    }

    // Engage engine library interface
    static
    {
        System.loadLibrary("engage-shared");
    }

    // API calls
    @Keep
    public native String engageGetVersion();

    @Keep
    public native String engageGetNetworkInterfaceDevices();

    @Keep
    public native String engageGetAudioDevices();

    @Keep
    public native int engageInitialize(String enginePolicyConfiguration, String userIdentity, String tempStoragePath);

    @Keep
    public native int engageShutdown();

    @Keep
    public native int engageStart();

    @Keep
    public native int engageStop();

    @Keep
    public native int engageCreateGroup(String jsonConfiguration);

    @Keep
    public native int engageDeleteGroup(String id);

    @Keep
    public native int engageJoinGroup(String id);

    @Keep
    public native int engageLeaveGroup(String id);

    @Keep
    public native int engageBeginGroupTx(String id, int txPriority, int txFlags);

    @Keep
    public native int engageBeginGroupTxAdvanced(String id, String jsonParams);

    @Keep
    public native int engageEndGroupTx(String id);

    @Keep
    public native int engageSetGroupRxTag(String id, int tag);

    @Keep
    public native int engageMuteGroupRx(String id);

    @Keep
    public native int engageUnmuteGroupRx(String id);

    @Keep
    public native int engageMuteGroupTx(String id);

    @Keep
    public native int engageUnmuteGroupTx(String id);

    @Keep
    public native int engageSetGroupRxVolume(String id, int left, int right);

    @Keep
    public native int engageUpdatePresenceDescriptor(String id, String jsonDescriptor, int forceBeacon);

    @Keep
    public native int engageSendGroupBlob(String id, byte[] blob, int size, String jsonParams);

    @Keep
    public native int engageSendGroupRtp(String id, byte[] payload, int size, String jsonParams);

    @Keep
    public native int engageSendGroupRaw(String id, byte[] raw, int size, String jsonParams);

    @Keep
    public native int engageRegisterGroupRtpHandler(String id, int payloadId);

    @Keep
    public native int engageUnregisterGroupRtpHandler(String id, int payloadId);

    @Keep
    public native int engageEncrypt(byte[] src, int size, byte[] dst, String passwordHexByteString);

    @Keep
    public byte[] encryptSimple(byte[] clearData, String passwordHexByteString)
    {
        byte[] rc = null;

        try
        {
            byte[] dst = new byte[clearData.length + 16 + 512];
            int transformedLen = engageEncrypt(clearData, clearData.length, dst, passwordHexByteString);
            if(transformedLen > 0)
            {
                rc = new byte[transformedLen];
                System.arraycopy(dst, 0, rc, 0, transformedLen);
            }
            System.gc();
        }
        catch (Exception e)
        {
            rc = null;
            e.printStackTrace();
        }

        return rc;
    }

    @Keep
    public byte[] decryptSimple(byte[] clearData, String passwordHexByteString)
    {
        byte[] rc = null;

        try
        {
            byte[] dst = new byte[clearData.length + 16 + 512];
            int transformedLen = engageDecrypt(clearData, clearData.length, dst, passwordHexByteString);
            if(transformedLen > 0)
            {
                rc = new byte[transformedLen];
                System.arraycopy(dst, 0, rc, 0, transformedLen);
            }
            System.gc();
        }
        catch (Exception e)
        {
            rc = null;
            e.printStackTrace();
        }

        return rc;
    }

    @Keep
    public native int engageDecrypt(byte[] src,
                                    int size,
                                    byte[] dst,
                                    String passwordHexByteString);


    @Keep
    public native String engageGetActiveLicenseDescriptor();

    @Keep
    public native String engageGetLicenseDescriptor(String entitlement, String key, String activationCode, String manufacturerId);

    @Keep
    public native int engageUpdateLicense(String entitlement, String key, String activationCode, String manufacturerId);

    @Keep
    public native void engagePlatformServiceDiscovered(String id, String jsonParams);

    @Keep
    public native void engagePlatformServiceRediscovered(String id, String jsonParams);

    @Keep
    public native void engagePlatformServiceUndiscovered(String id);

    @Keep
    public native void engageQueryGroupTimeline(String id, String jsonParams);

    @Keep
    public native void engageQueryGroupHealth(String id);

    @Keep
    public native int engageLogMsg(int level, String msg);

    @Keep
    public native String engageGenerateMission(String keyPhrase, int audioGroupCount, String rallypointHost, String missionName);

    @Keep
    public native int engageSetMissionId(String missionId);

    @Keep
    public native int engageOpenCertStore(String fileName, String passwordHexByteString);

    @Keep
    public native int engageGetCertStoreDescriptor();

    @Keep
    public native int engageCloseCertStore();

    @Keep
    public native int engageSetCertStoreCertificatePem(String id, String certificatePem, String privateKeyPem);

    @Keep
    public native int engageSetCertStoreCertificateP12(String id, byte[] data, int size, String password);

    @Keep
    public native int engageDeleteCertStoreCertificate(String id);

    @Keep
    public native String engageGetCertStoreCertificatePem(String id);

    @Keep
    public native String engageGetCertificateDescriptorFromPem(String pem);

    @Keep
    public native int engageImportCertStoreElementFromCertStore(String id, String srcId, String srcFileName, String srcPasswordHexByteString);

    @Keep
    public native String engageQueryCertStoreContents(String fileName, String passwordHexByteString);

    // Platform services requests ("upcalls" from the Engine)
    @Keep
    private void onPlatformRequestBeginServiceDiscovery(final String serviceType, final int preferredProtocol)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                if (_mdsnDiscoverer == null)
                {
                    _mdsnDiscoverer = new MdnsDiscoverer();
                }

                _mdsnDiscoverer.addListener(serviceType);
            }
        });
    }

    @Keep
    private void onPlatformRequestEndServiceDiscovery(final String serviceType, final int preferredProtocol)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                if (_mdsnDiscoverer == null)
                {
                    return;
                }

                _mdsnDiscoverer.removeListener(serviceType);
            }
        });
    }

    @Keep
    private String onPlatformGetAudioDeviceList(int forInput)
    {
        String rc = null;

        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.M)
        {
            try
            {
                AudioManager audioManager = (AudioManager) _appContext.getSystemService(Context.AUDIO_SERVICE);
                if(audioManager != null)
                {
                    JSONObject root = new JSONObject();
                    JSONArray list = new JSONArray();

                    final AudioDeviceInfo[] devices = audioManager.getDevices(AudioManager.GET_DEVICES_ALL);

                    for(AudioDeviceInfo d : devices)
                    {
                        if( ((forInput == 1) && d.isSource()) || ((forInput == 0) && d.isSink()) )
                        {
                            JSONObject di = new JSONObject();

                            di.put(JsonFields.AudioDevice.hardwareId, Integer.toString(d.getId()));
                            di.put(JsonFields.AudioDevice.name, d.getProductName());
                            di.put(JsonFields.AudioDevice.type, Integer.toString(d.getType()));
                            di.put(JsonFields.AudioDevice.manufacturer, "");
                            di.put(JsonFields.AudioDevice.model, "");
                            di.put(JsonFields.AudioDevice.serialNumber, "");
                            di.put(JsonFields.AudioDevice.isDefault, false);
                            di.put(JsonFields.AudioDevice.extra, d.getAddress());

                            list.put(di);
                        }
                    }

                    // Add a default device
                    {
                        JSONObject di = new JSONObject();

                        di.put(JsonFields.AudioDevice.hardwareId, "0");
                        di.put(JsonFields.AudioDevice.name, "default");
                        di.put(JsonFields.AudioDevice.type, "0");
                        di.put(JsonFields.AudioDevice.manufacturer, "");
                        di.put(JsonFields.AudioDevice.model, "");
                        di.put(JsonFields.AudioDevice.serialNumber, "");
                        di.put(JsonFields.AudioDevice.isDefault, true);
                        di.put(JsonFields.AudioDevice.extra, "");

                        list.put(di);
                    }

                    root.put(JsonFields.ListOfAudioDevice.objectName, list);
                    rc = root.toString();
                }
                else
                {
                    throw new Exception("onPlatformGetAudioDeviceList: failed to acquire audio manager");
                }
            }
            catch (Exception e)
            {
                e.printStackTrace();
                rc = null;
            }
        }

        if(rc == null)
        {
            try
            {
                JSONObject root = new JSONObject();
                JSONArray list = new JSONArray();

                JSONObject di = new JSONObject();

                di.put(JsonFields.AudioDevice.hardwareId, "0");
                di.put(JsonFields.AudioDevice.name, "default");
                di.put(JsonFields.AudioDevice.type, "0");
                di.put(JsonFields.AudioDevice.manufacturer, "");
                di.put(JsonFields.AudioDevice.model, "");
                di.put(JsonFields.AudioDevice.serialNumber, "");
                di.put(JsonFields.AudioDevice.isDefault, true);
                di.put(JsonFields.AudioDevice.extra, "");

                list.put(di);
                root.put(JsonFields.ListOfAudioDevice.objectName, list);
                rc = root.toString();
            }
            catch (Exception e)
            {
                rc = "{}";
            }
        }

        return rc;
    }

    @Keep
    private String onPlatformGetNetworkInterfaceDeviceList()
    {
        String rc = null;

        try
        {
            Enumeration<NetworkInterface> interfaces = NetworkInterface.getNetworkInterfaces();

            JSONObject root = new JSONObject();
            JSONArray list = new JSONArray();

            while( interfaces.hasMoreElements() )
            {
                NetworkInterface networkInterface = interfaces.nextElement();

                String nm = networkInterface.getName();
                String dn = networkInterface.getDisplayName();
                String hw = bytesToHexString(networkInterface.getHardwareAddress());

                if(dn.compareTo(nm) == 0)
                {
                    dn = "";
                }

                JSONObject di;

                Enumeration<InetAddress> inetAddresses = networkInterface.getInetAddresses();

                if(inetAddresses.hasMoreElements())
                {
                    while (inetAddresses.hasMoreElements())
                    {
                        InetAddress ina = inetAddresses.nextElement();

                        di = new JSONObject();
                        di.put(JsonFields.NetworkInterfaceDevice.name, nm);
                        di.put(JsonFields.NetworkInterfaceDevice.friendlyName, dn);
                        di.put(JsonFields.NetworkInterfaceDevice.description, "");
                        di.put(JsonFields.NetworkInterfaceDevice.hardwareAddress, hw);
                        di.put(JsonFields.NetworkInterfaceDevice.available, networkInterface.isUp());
                        di.put(JsonFields.NetworkInterfaceDevice.isLoopback, networkInterface.isLoopback());
                        di.put(JsonFields.NetworkInterfaceDevice.supportsMulticast, networkInterface.supportsMulticast());

                        if (ina instanceof Inet4Address || ina instanceof Inet6Address)
                        {
                            String sAddress = ina.toString();
                            if (sAddress != null && sAddress.length() > 1)
                            {
                                sAddress = sAddress.substring(1);
                            }

                            di.put(JsonFields.NetworkInterfaceDevice.address, sAddress);

                            if (ina instanceof Inet4Address)
                            {
                                di.put(JsonFields.NetworkInterfaceDevice.family, NetworkDeviceFamily.ipv4.toInt());
                            }
                            else if (ina instanceof Inet6Address)
                            {
                                di.put(JsonFields.NetworkInterfaceDevice.family, NetworkDeviceFamily.ipv6.toInt());
                            }
                            else
                            {
                                di.put(JsonFields.NetworkInterfaceDevice.family, -1);
                            }
                        }

                        list.put(di);
                    }
                }
                else
                {
                    di = new JSONObject();
                    di.put(JsonFields.NetworkInterfaceDevice.name, nm);
                    di.put(JsonFields.NetworkInterfaceDevice.friendlyName, dn);
                    di.put(JsonFields.NetworkInterfaceDevice.description, "");
                    di.put(JsonFields.NetworkInterfaceDevice.hardwareAddress, hw);
                    di.put(JsonFields.NetworkInterfaceDevice.available, networkInterface.isUp());
                    di.put(JsonFields.NetworkInterfaceDevice.isLoopback, networkInterface.isLoopback());
                    di.put(JsonFields.NetworkInterfaceDevice.supportsMulticast, networkInterface.supportsMulticast());
                    di.put(JsonFields.NetworkInterfaceDevice.family, -1);
                    di.put(JsonFields.NetworkInterfaceDevice.address, "");
                    list.put(di);
                }
            }

            root.put(JsonFields.ListOfNetworkInterfaceDevice.objectName, list);
            rc = root.toString();
        }
        catch(Exception e)
        {
            rc = null;
        }

        if(rc == null)
        {
            rc = "{}";
        }

        return rc;
    }

    // Event callbacks
    @Keep
    private void onEngineStarted(final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IEngineListener listener : _engineListeners)
                {
                    listener.onEngineStarted(eventExtraJson);
                }
            }
        });
    }

    @Keep
    private void onEngineStartFailed(final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IEngineListener listener : _engineListeners)
                {
                    listener.onEngineStartFailed(eventExtraJson);
                }
            }
        });
    }

    @Keep
    private void onEngineStopped(final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IEngineListener listener : _engineListeners)
                {
                    listener.onEngineStopped(eventExtraJson);
                }
            }
        });
    }

    @Keep
    private void onRpLeafPausingConnectionAttempt(final String id, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IRallypointListener listener : _rallypointListeners)
                {
                    listener.onRallypointPausingConnectionAttempt(id, eventExtraJson);
                }
            }
        });
    }

    @Keep
    private void onRpLeafConnecting(final String id, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IRallypointListener listener : _rallypointListeners)
                {
                    listener.onRallypointConnecting(id, eventExtraJson);
                }
            }
        });
    }

    @Keep
    private void onRpLeafConnected(final String id, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IRallypointListener listener : _rallypointListeners)
                {
                    listener.onRallypointConnected(id, eventExtraJson);
                }
            }
        });
    }

    @Keep
    private void onRpLeafDisconnected(final String id, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IRallypointListener listener : _rallypointListeners)
                {
                    listener.onRallypointDisconnected(id, eventExtraJson);
                }
            }
        });
    }

    @Keep
    public void onRpLeafRoundtripReport(final String id, final long rtMs, final long rtQualityRating, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IRallypointListener listener : _rallypointListeners)
                {
                    listener.onRallypointRoundtripReport(id, rtMs, rtQualityRating, eventExtraJson);
                }
            }
        });
    }

    @Keep
    private void onGroupCreated(final String id, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IGroupListener listener : _groupListeners)
                {
                    listener.onGroupCreated(id, eventExtraJson);
                }
            }
        });
    }

    @Keep
    private void onGroupCreateFailed(final String id, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IGroupListener listener : _groupListeners)
                {
                    listener.onGroupCreateFailed(id, eventExtraJson);
                }
            }
        });
    }


    @Keep
    private void onGroupDeleted(final String id, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IGroupListener listener : _groupListeners)
                {
                    listener.onGroupDeleted(id, eventExtraJson);
                }
            }
        });
    }

    @Keep
    private void onGroupConnected(final String id, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IGroupListener listener : _groupListeners)
                {
                    listener.onGroupConnected(id, eventExtraJson);
                }
            }
        });
    }

    @Keep
    private void onGroupConnectFailed(final String id, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IGroupListener listener : _groupListeners)
                {
                    listener.onGroupConnectFailed(id, eventExtraJson);
                }
            }
        });
    }

    @Keep
    private void onGroupDisconnected(final String id, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IGroupListener listener : _groupListeners)
                {
                    listener.onGroupDisconnected(id, eventExtraJson);
                }
            }
        });
    }

    @Keep
    private void onGroupJoined(final String id, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IGroupListener listener : _groupListeners)
                {
                    listener.onGroupJoined(id, eventExtraJson);
                }
            }
        });
    }

    @Keep
    private void onGroupJoinFailed(final String id, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IGroupListener listener : _groupListeners)
                {
                    listener.onGroupJoinFailed(id, eventExtraJson);
                }
            }
        });
    }

    @Keep
    private void onGroupLeft(final String id, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IGroupListener listener : _groupListeners)
                {
                    listener.onGroupLeft(id, eventExtraJson);
                }
            }
        });
    }

    @Keep
    private void onGroupMemberCountChanged(final String id, final long count, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IGroupListener listener : _groupListeners)
                {
                    listener.onGroupMemberCountChanged(id, count, eventExtraJson);
                }
            }
        });
    }

    @Keep
    private void onGroupRxStarted(final String id, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IGroupListener listener : _groupListeners)
                {
                    listener.onGroupRxStarted(id, eventExtraJson);
                }
            }
        });
    }

    @Keep
    private void onGroupRxEnded(final String id, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IGroupListener listener : _groupListeners)
                {
                    listener.onGroupRxEnded(id, eventExtraJson);
                }
            }
        });
    }

    @Keep
    private void onGroupRxMuted(final String id, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IGroupListener listener : _groupListeners)
                {
                    listener.onGroupRxMuted(id, eventExtraJson);
                }
            }
        });
    }

    @Keep
    private void onGroupRxUnmuted(final String id, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IGroupListener listener : _groupListeners)
                {
                    listener.onGroupRxUnmuted(id, eventExtraJson);
                }
            }
        });
    }

    @Keep
    private void onGroupTxMuted(final String id, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IGroupListener listener : _groupListeners)
                {
                    listener.onGroupTxMuted(id, eventExtraJson);
                }
            }
        });
    }

    @Keep
    private void onGroupTxUnmuted(final String id, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IGroupListener listener : _groupListeners)
                {
                    listener.onGroupTxUnmuted(id, eventExtraJson);
                }
            }
        });
    }

    @Keep
    private void onGroupTxStarted(final String id, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IGroupListener listener : _groupListeners)
                {
                    listener.onGroupTxStarted(id, eventExtraJson);
                }
            }
        });
    }

    @Keep
    private void onGroupTxEnded(final String id, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IGroupListener listener : _groupListeners)
                {
                    listener.onGroupTxEnded(id, eventExtraJson);
                }
            }
        });
    }

    @Keep
    private void onGroupTxFailed(final String id, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IGroupListener listener : _groupListeners)
                {
                    listener.onGroupTxFailed(id, eventExtraJson);
                }
            }
        });
    }

    @Keep
    private void onGroupTxUsurpedByPriority(final String id, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IGroupListener listener : _groupListeners)
                {
                    listener.onGroupTxUsurpedByPriority(id, eventExtraJson);
                }
            }
        });
    }

    @Keep
    private void onGroupMaxTxTimeExceeded(final String id, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IGroupListener listener : _groupListeners)
                {
                    listener.onGroupMaxTxTimeExceeded(id, eventExtraJson);
                }
            }
        });
    }

    @Keep
    private void onGroupRxSpeakersChanged(final String id, final String groupTalkerJson, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IGroupListener listener : _groupListeners)
                {
                    listener.onGroupRxSpeakersChanged(id, groupTalkerJson, eventExtraJson);
                }
            }
        });
    }

    @Keep
    private void onGroupNodeDiscovered(final String id, final String nodeJson, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IGroupListener listener : _groupListeners)
                {
                    listener.onGroupNodeDiscovered(id, nodeJson, eventExtraJson);
                }
            }
        });
    }

    @Keep
    private void onGroupNodeRediscovered(final String id, final String nodeJson, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IGroupListener listener : _groupListeners)
                {
                    listener.onGroupNodeRediscovered(id, nodeJson, eventExtraJson);
                }
            }
        });
    }

    @Keep
    private void onGroupNodeUndiscovered(final String id, final String nodeJson, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IGroupListener listener : _groupListeners)
                {
                    listener.onGroupNodeUndiscovered(id, nodeJson, eventExtraJson);
                }
            }
        });
    }

    @Keep
    private void onGroupAssetDiscovered(final String id, final String nodeJson, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IGroupListener listener : _groupListeners)
                {
                    listener.onGroupAssetDiscovered(id, nodeJson, eventExtraJson);
                }
            }
        });
    }

    @Keep
    private void onGroupAssetRediscovered(final String id, final String nodeJson, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IGroupListener listener : _groupListeners)
                {
                    listener.onGroupAssetRediscovered(id, nodeJson, eventExtraJson);
                }
            }
        });
    }

    @Keep
    private void onGroupAssetUndiscovered(final String id, final String nodeJson, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IGroupListener listener : _groupListeners)
                {
                    listener.onGroupAssetUndiscovered(id, nodeJson, eventExtraJson);
                }
            }
        });
    }

    @Keep
    private void onGroupBlobSent(final String id, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IGroupListener listener : _groupListeners)
                {
                    listener.onGroupBlobSent(id, eventExtraJson);
                }
            }
        });
    }

    @Keep
    private void onGroupBlobSendFailed(final String id, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IGroupListener listener : _groupListeners)
                {
                    listener.onGroupBlobSendFailed(id, eventExtraJson);
                }
            }
        });
    }

    @Keep
    void onGroupBlobReceived(final String id, final String blobInfoJson, final byte[] blob, final long blobSize, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IGroupListener listener : _groupListeners)
                {
                    listener.onGroupBlobReceived(id, blobInfoJson, blob, blobSize, eventExtraJson);
                }
            }
        });
    }

    @Keep
    void onGroupRtpSent(final String id, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IGroupListener listener : _groupListeners)
                {
                    listener.onGroupRtpSent(id, eventExtraJson);
                }
            }
        });
    }

    @Keep
    void onGroupRtpSendFailed(final String id, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IGroupListener listener : _groupListeners)
                {
                    listener.onGroupRtpSendFailed(id, eventExtraJson);
                }
            }
        });
    }

    @Keep
    void onGroupRtpReceived(final String id, final String rtpHeaderJson, final byte[] payload, final long payloadSize, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IGroupListener listener : _groupListeners)
                {
                    listener.onGroupRtpReceived(id, rtpHeaderJson, payload, payloadSize, eventExtraJson);
                }
            }
        });
    }

    @Keep
    void onGroupRawSent(final String id, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IGroupListener listener : _groupListeners)
                {
                    listener.onGroupRawSent(id, eventExtraJson);
                }
            }
        });
    }

    @Keep
    void onGroupRawSendFailed(final String id, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IGroupListener listener : _groupListeners)
                {
                    listener.onGroupRawSendFailed(id, eventExtraJson);
                }
            }
        });
    }

    @Keep
    void onGroupRawReceived(final String id, final byte[] raw, final long rawSize, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IGroupListener listener : _groupListeners)
                {
                    listener.onGroupRawReceived(id, raw, rawSize, eventExtraJson);
                }
            }
        });
    }

    @Keep
    private void onLicenseChanged(final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (ILicenseListener listener : _licenseListeners)
                {
                    listener.onLicenseChanged(eventExtraJson);
                }
            }
        });
    }

    @Keep
    private void onLicenseExpired(final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (ILicenseListener listener : _licenseListeners)
                {
                    listener.onLicenseExpired(eventExtraJson);
                }
            }
        });
    }

    @Keep
    private void onLicenseExpiring(final String secondsLeft, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                double doubleSecondsLeft;
                doubleSecondsLeft = Double.parseDouble(secondsLeft);
                for(ILicenseListener listener :_licenseListeners)
                {
                    listener.onLicenseExpiring(doubleSecondsLeft, eventExtraJson);
                }
            }
        });
    }

    @Keep
    private void onGroupTimelineEventStarted(final String id, final String eventJson, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IGroupListener listener : _groupListeners)
                {
                    listener.onGroupTimelineEventStarted(id, eventJson, eventExtraJson);
                }
            }
        });
    }

    @Keep
    private void onGroupTimelineEventUpdated(final String id, final String eventJson, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IGroupListener listener : _groupListeners)
                {
                    listener.onGroupTimelineEventUpdated(id, eventJson, eventExtraJson);
                }
            }
        });
    }

    @Keep
    private void onGroupTimelineEventEnded(final String id, final String eventJson, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IGroupListener listener : _groupListeners)
                {
                    listener.onGroupTimelineEventEnded(id, eventJson, eventExtraJson);
                }
            }
        });
    }

    @Keep
    private void onGroupTimelineReport(final String id, final String reportJson, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IGroupListener listener : _groupListeners)
                {
                    listener.onGroupTimelineReport(id, reportJson, eventExtraJson);
                }
            }
        });
    }

    @Keep
    private void onGroupTimelineReportFailed(final String id, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IGroupListener listener : _groupListeners)
                {
                    listener.onGroupTimelineReportFailed(id, eventExtraJson);
                }
            }
        });
    }

    @Keep
    private void onGroupTimelineGroomed(final String id, final String eventListJson, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IGroupListener listener : _groupListeners)
                {
                    listener.onGroupTimelineGroomed(id, eventListJson, eventExtraJson);
                }
            }
        });
    }

    @Keep
    private void onGroupHealthReport(final String id, final String healthReportJson, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IGroupListener listener : _groupListeners)
                {
                    listener.onGroupHealthReport(id, healthReportJson, eventExtraJson);
                }
            }
        });
    }

    @Keep
    private void onGroupHealthReportFailed(final String id, final String eventExtraJson)
    {
        _handler.post(new Runnable()
        {
            @Override
            public void run()
            {
                for (IGroupListener listener : _groupListeners)
                {
                    listener.onGroupHealthReportFailed(id, eventExtraJson);
                }
            }
        });
    }
}
