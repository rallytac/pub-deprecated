//
//  Copyright (c) 2018 Rally Tactical Systems, Inc.
//  All rights reserved.
//

using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

public class Engage
{
    #region Interfaces and such
    public class GroupDescriptor
    {
        public string id;
        public string name;
        public bool isEncrypted;
        public bool allowsFullDuplex;
    }

    public interface IEngineNotifications
    {
        void onEngineStarted();
        void onEngineStopped();
    }

    public interface ILicenseNotifications
    {
        void onLicenseChanged();
        void onLicenseExpired();
        void onLicenseExpiring(long secondsLeft);
    }

    public interface IRallypointNotifications
    {
        void onRallypointPausingConnectionAttempt(string id);
        void onRallypointConnecting(string id);
        void onRallypointConnected(string id);
        void onRallypointDisconnected(string id);
        void onRallypointRoundtripReport(string id, int rtMs, int rtRating);
    }

    public interface IGroupNotifications
    {
        void onGroupCreated(string id);
        void onGroupCreateFailed(string id);
        void onGroupDeleted(string id);
        void onGroupConnected(string id);
        void onGroupConnectFailed(string id);
        void onGroupDisconnected(string id);
        void onGroupJoined(string id);
        void onGroupJoinFailed(string id);
        void onGroupLeft(string id);
        void onGroupMemberCountChanged(string id, int newCount);
        void onGroupRxStarted(string id);
        void onGroupRxEnded(string id);
        void onGroupRxMuted(string id);
        void onGroupRxUnmuted(string id);
        void onGroupRxSpeakersChanged(string id, string groupTalkerJson);
        void onGroupTxStarted(string id);
        void onGroupTxEnded(string id);
        void onGroupTxFailed(string id);
        void onGroupTxUsurpedByPriority(string id);
        void onGroupMaxTxTimeExceeded(string id);
        void onGroupNodeDiscovered(string id, string nodeJson);
        void onGroupNodeRediscovered(string id, string nodeJson);
        void onGroupNodeUndiscovered(string id, string nodeJson);
        void onGroupAssetDiscovered(string id, string nodeJson);
        void onGroupAssetRediscovered(string id, string nodeJson);
        void onGroupAssetUndiscovered(string id, string nodeJson);
        void onGroupBlobSent(string id);
        void onGroupBlobSendFailed(string id);
        void onGroupBlobReceived(string id, string blobInfoJson, byte[] blob, int blobSize);
        void onGroupRtpSent(string id);
        void onGroupRtpSendFailed(string id);
        void onGroupRtpReceived(string id, string rtpInfoJson, byte[] payload, int payloadSize);
        void onGroupRawSent(string id);
        void onGroupRawSendFailed(string id);
        void onGroupRawReceived(string id, byte[] raw, int rawSize);

        void onGroupTimelineEventStarted(string id, string eventJson);
        void onGroupTimelineEventUpdated(string id, string eventJson);
        void onGroupTimelineEventEnded(string id, string eventJson);
        void onGroupTimelineReport(string id, string reportJson);
        void onGroupTimelineReportFailed(string id);
    }
    #endregion


    // The Engage DLL
    private const string ENGAGE_DLL = "engage-shared.dll";

    // Limits
    public const int ENGAGE_MAX_GROUP_ID_SZ = 64;
    public const int ENGAGE_MAX_GROUP_NAME_SZ = 128;

    // Result codes
    public const int ENGAGE_RESULT_OK = 0;
    public const int ENGAGE_RESULT_INVALID_PARAMETERS = -1;
    public const int ENGAGE_RESULT_NOT_INITIALIZED = -2;
    public const int ENGAGE_RESULT_ALREADY_INITIALIZED = -3;
    public const int ENGAGE_RESULT_GENERAL_FAILURE = -4;

    // License status codes 

    public enum LicensingStatusCode : int
    {
        OK = 0,
        ERR_NULL_ENTITLEMENT_KEY = -1,
        ERR_NULL_LICENSE_KEY = -2,
        ERR_INVALID_LICENSE_KEY_LEN = -3,
        ERR_LICENSE_KEY_VERIFICATION_FAILURE = -4,
        ERR_ACTIVATION_CODE_VERIFICATION_FAILURE = -5,
        ERR_INVALID_EXPIRATION_DATE = -6,
        ERR_GENERAL_FAILURE = -7,
        ERR_NOT_INITIALIZED = -8,
        ERR_REQUIRES_ACTIVATION = -9
    }

    #region Callback delegate types
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    private delegate void EngageVoidCallback();

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    private delegate void EngageStringCallback(string s);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    private delegate void EngageString2Callback(string s1, string s2);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    private delegate void EngageStringAndIntCallback(string s, int i);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    private delegate void EngageStringAndArgvCallback(string s, IntPtr ptr);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    private delegate void EngageStringAndBlobCallback(string s, IntPtr ptr, int i);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    private delegate void EngageString2AndBlobCallback(string s, string j, IntPtr ptr, int i);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    private delegate void EngageStringAndTwoIntCallback(string s, int i1, int i2);
    #endregion

    #region Structures
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    private struct EngageGroupDescriptor_t
    {
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = ENGAGE_MAX_GROUP_ID_SZ)]
        public byte[] id;

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = ENGAGE_MAX_GROUP_NAME_SZ)]
        public byte[] name;

        [MarshalAs(UnmanagedType.U1)]
        public Boolean isEncrypted;

        [MarshalAs(UnmanagedType.U1)]
        public Boolean allowsFullDuplex;
    }

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    private struct EngageCallbacks_t
    {
        public EngageVoidCallback PFN_ENGAGE_ENGINE_STARTED;
        public EngageVoidCallback PFN_ENGAGE_ENGINE_STOPPED;

        public EngageStringCallback PFN_ENGAGE_RP_PAUSING_CONNECTION_ATTEMPT;
        public EngageStringCallback PFN_ENGAGE_RP_CONNECTING;
        public EngageStringCallback PFN_ENGAGE_RP_CONNECTED;
        public EngageStringCallback PFN_ENGAGE_RP_DISCONNECTED;
        public EngageStringAndTwoIntCallback PFN_ENGAGE_RP_ROUNDTRIP_REPORT;

        public EngageStringCallback PFN_ENGAGE_GROUP_CREATED;
        public EngageStringCallback PFN_ENGAGE_GROUP_CREATE_FAILED;
        public EngageStringCallback PFN_ENGAGE_GROUP_DELETED;

        public EngageStringCallback PFN_ENGAGE_GROUP_CONNECTED;
        public EngageStringCallback PFN_ENGAGE_GROUP_CONNECT_FAILED;
        public EngageStringCallback PFN_ENGAGE_GROUP_DISCONNECTED;

        public EngageStringCallback PFN_ENGAGE_GROUP_JOINED;
        public EngageStringCallback PFN_ENGAGE_GROUP_JOIN_FAILED;
        public EngageStringCallback PFN_ENGAGE_GROUP_LEFT;

        public EngageStringAndIntCallback PFN_ENGAGE_GROUP_MEMBER_COUNT_CHANGED;

        public EngageString2Callback PFN_ENGAGE_GROUP_NODE_DISCOVERED;
        public EngageString2Callback PFN_ENGAGE_GROUP_NODE_REDISCOVERED;
        public EngageString2Callback PFN_ENGAGE_GROUP_NODE_UNDISCOVERED;

        public EngageStringCallback PFN_ENGAGE_GROUP_RX_STARTED;
        public EngageStringCallback PFN_ENGAGE_GROUP_RX_ENDED;
        public EngageString2Callback PFN_ENGAGE_GROUP_RX_SPEAKERS_CHANGED;
        public EngageStringCallback PFN_ENGAGE_GROUP_RX_MUTED;
        public EngageStringCallback PFN_ENGAGE_GROUP_RX_UNMUTED;

        public EngageStringCallback PFN_ENGAGE_GROUP_TX_STARTED;
        public EngageStringCallback PFN_ENGAGE_GROUP_TX_ENDED;
        public EngageStringCallback PFN_ENGAGE_GROUP_TX_FAILED;
        public EngageStringCallback PFN_ENGAGE_GROUP_TX_USURPED_BY_PRIORITY;
        public EngageStringCallback PFN_ENGAGE_GROUP_MAX_TX_TIME_EXCEEDED;

        public EngageString2Callback PFN_ENGAGE_GROUP_ASSET_DISCOVERED;
        public EngageString2Callback PFN_ENGAGE_GROUP_ASSET_REDISCOVERED;
        public EngageString2Callback PFN_ENGAGE_GROUP_ASSET_UNDISCOVERED;

        public EngageVoidCallback PFN_ENGAGE_LICENSE_CHANGED;
        public EngageVoidCallback PFN_ENGAGE_LICENSE_EXPIRED;
        public EngageStringCallback PFN_ENGAGE_LICENSE_EXPIRING;

        public EngageStringCallback PFN_ENGAGE_GROUP_BLOB_SENT;
        public EngageStringCallback PFN_ENGAGE_GROUP_BLOB_SEND_FAILED;
        public EngageString2AndBlobCallback PFN_ENGAGE_GROUP_BLOB_RECEIVED;

        public EngageStringCallback PFN_ENGAGE_GROUP_RTP_SENT;
        public EngageStringCallback PFN_ENGAGE_GROUP_RTP_SEND_FAILED;
        public EngageString2AndBlobCallback PFN_ENGAGE_GROUP_RTP_RECEIVED;

        public EngageStringCallback PFN_ENGAGE_GROUP_RAW_SENT;
        public EngageStringCallback PFN_ENGAGE_GROUP_RAW_SEND_FAILED;
        public EngageStringAndBlobCallback PFN_ENGAGE_GROUP_RAW_RECEIVED;

        public EngageString2Callback PFN_ENGAGE_GROUP_TIMELINE_EVENT_STARTED;
        public EngageString2Callback PFN_ENGAGE_GROUP_TIMELINE_EVENT_UPDATED;
        public EngageString2Callback PFN_ENGAGE_GROUP_TIMELINE_EVENT_ENDED;
        public EngageString2Callback PFN_ENGAGE_GROUP_TIMELINE_REPORT;
        public EngageStringCallback PFN_ENGAGE_GROUP_TIMELINE_REPORT_FAILED;
    }
    #endregion

    #region Library functions
    [DllImport(ENGAGE_DLL, CallingConvention = CallingConvention.Cdecl)]
    private static extern int engageRegisterCallbacks(ref EngageCallbacks_t callbacks);

    [DllImport(ENGAGE_DLL, CallingConvention = CallingConvention.Cdecl)]
    private static extern int engageInitialize(string enginePolicyConfiguration, 
                                               string userIdentity,
                                               string tempStoragePath);

    [DllImport(ENGAGE_DLL, CallingConvention = CallingConvention.Cdecl)]
    private static extern int engageShutdown();

    [DllImport(ENGAGE_DLL, CallingConvention = CallingConvention.Cdecl)]
    private static extern int engageStart();

    [DllImport(ENGAGE_DLL, CallingConvention = CallingConvention.Cdecl)]
    private static extern int engageStop();

    [DllImport(ENGAGE_DLL, CallingConvention = CallingConvention.Cdecl)]
    private static extern int engageCreateGroup(string jsonConfiguration);

    [DllImport(ENGAGE_DLL, CallingConvention = CallingConvention.Cdecl)]
    private static extern int engageDeleteGroup(string id);

    [DllImport(ENGAGE_DLL, CallingConvention = CallingConvention.Cdecl)]
    private static extern int engageJoinGroup(string id);

    [DllImport(ENGAGE_DLL, CallingConvention = CallingConvention.Cdecl)]
    private static extern int engageLeaveGroup(string id);

    [DllImport(ENGAGE_DLL, CallingConvention = CallingConvention.Cdecl)]
    private static extern int engageBeginGroupTx(string id, int txPriority, int txFlags);

    [DllImport(ENGAGE_DLL, CallingConvention = CallingConvention.Cdecl)]
    private static extern int engageBeginGroupTxAdvanced(string id, string jsonParams);

    [DllImport(ENGAGE_DLL, CallingConvention = CallingConvention.Cdecl)]
    private static extern int engageEndGroupTx(string id);

    [DllImport(ENGAGE_DLL, CallingConvention = CallingConvention.Cdecl)]
    private static extern int engageSetGroupRxTag(string id, int tag);

    [DllImport(ENGAGE_DLL, CallingConvention = CallingConvention.Cdecl)]
    private static extern int engageMuteGroupRx(string id);

    [DllImport(ENGAGE_DLL, CallingConvention = CallingConvention.Cdecl)]
    private static extern int engageUnmuteGroupRx(string id);

    [DllImport(ENGAGE_DLL, CallingConvention = CallingConvention.Cdecl)]
    private static extern int engageSetGroupRxVolume(string id, int left, int right);

    [DllImport(ENGAGE_DLL, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr engageGetVersion();

    [DllImport(ENGAGE_DLL, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr engageGetActiveLicenseDescriptor();

    [DllImport(ENGAGE_DLL, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr engageGetLicenseDescriptor(string entitlement, string key, string activationCode);

    [DllImport(ENGAGE_DLL, CallingConvention = CallingConvention.Cdecl)]
    private static extern int engageUpdateLicense(string entitlement, string key, string activationCode);

    [DllImport(ENGAGE_DLL, CallingConvention = CallingConvention.Cdecl)]
    private static extern int engageSendGroupBlob(string id, 
                                                  byte[] blob,
                                                  int blobSize,
                                                  string jsonBlobParams);

    [DllImport(ENGAGE_DLL, CallingConvention = CallingConvention.Cdecl)]
    private static extern int engageSendGroupRtp(string id, 
                                                 byte[] payload,
                                                 int payloadSize,
                                                 string jsonRtpHeader);

    [DllImport(ENGAGE_DLL, CallingConvention = CallingConvention.Cdecl)]
    private static extern int engageSendGroupRaw(string id, 
                                                 byte[] raw,
                                                 int rawSize,
                                                 string jsonRtpHeader);    

    [DllImport(ENGAGE_DLL, CallingConvention = CallingConvention.Cdecl)]
    private static extern int engageQueryGroupTimeline(string id,
                                                       string jsonParams);        

    [DllImport(ENGAGE_DLL, CallingConvention = CallingConvention.Cdecl)]
    private static extern int engageLogMsg(int level,
                                           string tag,
                                           string msg);                                                                                                
    #endregion

    #region Internal functions
    private static string[] stringArrayFromArgvStrPtrArray(IntPtr ptr)
    {
        string[] rc = null;
        int count = 0;
        IntPtr arrayPtr;
        IntPtr strPtr;

        // First count how many we have
        arrayPtr = ptr;
        while (true)
        {
            strPtr = Marshal.ReadIntPtr(arrayPtr);
            if (strPtr == (IntPtr)0)
            {
                break;
            }

            count++;
            arrayPtr += Marshal.SizeOf(arrayPtr);
        }

        // Now, allocate the array and copy over the strings
        if (count > 0)
        {
            rc = new string[count];

            int idx = 0;
            arrayPtr = ptr;
            while (true)
            {
                strPtr = Marshal.ReadIntPtr(arrayPtr);
                if (strPtr == (IntPtr)0)
                {
                    break;
                }

                rc[idx] = Marshal.PtrToStringAnsi(strPtr);

                arrayPtr += Marshal.SizeOf(arrayPtr);
                idx++;
            }
        }

        return rc;
    }

    private int registerCallbacks()
    {
        EngageCallbacks_t cb = new EngageCallbacks_t();

        cb.PFN_ENGAGE_ENGINE_STARTED = on_ENGAGE_ENGINE_STARTED;
        cb.PFN_ENGAGE_ENGINE_STOPPED = on_ENGAGE_ENGINE_STOPPED;

        cb.PFN_ENGAGE_RP_PAUSING_CONNECTION_ATTEMPT = on_ENGAGE_RP_PAUSING_CONNECTION_ATTEMPT;
        cb.PFN_ENGAGE_RP_CONNECTING = on_ENGAGE_RP_CONNECTING;
        cb.PFN_ENGAGE_RP_CONNECTED = on_ENGAGE_RP_CONNECTED;
        cb.PFN_ENGAGE_RP_DISCONNECTED = on_ENGAGE_RP_DISCONNECTED;
        cb.PFN_ENGAGE_RP_ROUNDTRIP_REPORT = on_ENGAGE_RP_ROUNDTRIP_REPORT;

        cb.PFN_ENGAGE_GROUP_CREATED = on_ENGAGE_GROUP_CREATED;
        cb.PFN_ENGAGE_GROUP_CREATE_FAILED = on_ENGAGE_GROUP_CREATE_FAILED;
        cb.PFN_ENGAGE_GROUP_DELETED = on_ENGAGE_GROUP_DELETED;

        cb.PFN_ENGAGE_GROUP_CONNECTED = on_ENGAGE_GROUP_CONNECTED;
        cb.PFN_ENGAGE_GROUP_CONNECT_FAILED = on_ENGAGE_GROUP_CONNECT_FAILED;
        cb.PFN_ENGAGE_GROUP_DISCONNECTED = on_ENGAGE_GROUP_DISCONNECTED;

        cb.PFN_ENGAGE_GROUP_JOINED = on_ENGAGE_GROUP_JOINED;
        cb.PFN_ENGAGE_GROUP_JOIN_FAILED = on_ENGAGE_GROUP_JOIN_FAILED;
        cb.PFN_ENGAGE_GROUP_LEFT = on_ENGAGE_GROUP_LEFT;

        // TODO: FIXME!
        cb.PFN_ENGAGE_GROUP_MEMBER_COUNT_CHANGED = null;
        //cb.PFN_ENGAGE_GROUP_MEMBER_COUNT_CHANGED = on_ENGAGE_GROUP_MEMBER_COUNT_CHANGED;

        cb.PFN_ENGAGE_GROUP_RX_STARTED = on_ENGAGE_GROUP_RX_STARTED;
        cb.PFN_ENGAGE_GROUP_RX_ENDED = on_ENGAGE_GROUP_RX_ENDED;

        cb.PFN_ENGAGE_GROUP_RX_MUTED = on_ENGAGE_GROUP_RX_MUTED;
        cb.PFN_ENGAGE_GROUP_RX_UNMUTED = on_ENGAGE_GROUP_RX_UNMUTED;

        cb.PFN_ENGAGE_GROUP_RX_SPEAKERS_CHANGED = on_ENGAGE_GROUP_RX_SPEAKERS_CHANGED;

        cb.PFN_ENGAGE_GROUP_TX_STARTED = on_ENGAGE_GROUP_TX_STARTED;
        cb.PFN_ENGAGE_GROUP_TX_ENDED = on_ENGAGE_GROUP_TX_ENDED;
        cb.PFN_ENGAGE_GROUP_TX_FAILED = on_ENGAGE_GROUP_TX_FAILED;
        cb.PFN_ENGAGE_GROUP_TX_USURPED_BY_PRIORITY = on_ENGAGE_GROUP_TX_USURPED_BY_PRIORITY;
        cb.PFN_ENGAGE_GROUP_MAX_TX_TIME_EXCEEDED = on_ENGAGE_GROUP_MAX_TX_TIME_EXCEEDED;

        cb.PFN_ENGAGE_GROUP_NODE_DISCOVERED = on_ENGAGE_GROUP_NODE_DISCOVERED;
        cb.PFN_ENGAGE_GROUP_NODE_REDISCOVERED = on_ENGAGE_GROUP_NODE_REDISCOVERED;
        cb.PFN_ENGAGE_GROUP_NODE_UNDISCOVERED = on_ENGAGE_GROUP_NODE_UNDISCOVERED;

        cb.PFN_ENGAGE_GROUP_ASSET_DISCOVERED = on_ENGAGE_GROUP_ASSET_DISCOVERED;
        cb.PFN_ENGAGE_GROUP_ASSET_REDISCOVERED = on_ENGAGE_GROUP_ASSET_REDISCOVERED;
        cb.PFN_ENGAGE_GROUP_ASSET_UNDISCOVERED = on_ENGAGE_GROUP_ASSET_UNDISCOVERED;

        cb.PFN_ENGAGE_LICENSE_CHANGED = on_ENGAGE_LICENSE_CHANGED;
        cb.PFN_ENGAGE_LICENSE_EXPIRED = on_ENGAGE_LICENSE_EXPIRED;
        cb.PFN_ENGAGE_LICENSE_EXPIRING = on_ENGAGE_LICENSE_EXPIRING;

        cb.PFN_ENGAGE_GROUP_BLOB_SENT = on_ENGAGE_GROUP_BLOB_SENT;
        cb.PFN_ENGAGE_GROUP_BLOB_SEND_FAILED = on_ENGAGE_GROUP_BLOB_SEND_FAILED;
        cb.PFN_ENGAGE_GROUP_BLOB_RECEIVED = on_ENGAGE_GROUP_BLOB_RECEIVED;

        cb.PFN_ENGAGE_GROUP_RTP_SENT = on_ENGAGE_GROUP_RTP_SENT;
        cb.PFN_ENGAGE_GROUP_RTP_SEND_FAILED = on_ENGAGE_GROUP_RTP_SEND_FAILED;
        cb.PFN_ENGAGE_GROUP_RTP_RECEIVED = on_ENGAGE_GROUP_RTP_RECEIVED;

        cb.PFN_ENGAGE_GROUP_RAW_SENT = on_ENGAGE_GROUP_RAW_SENT;
        cb.PFN_ENGAGE_GROUP_RAW_SEND_FAILED = on_ENGAGE_GROUP_RAW_SEND_FAILED;
        cb.PFN_ENGAGE_GROUP_RAW_RECEIVED = on_ENGAGE_GROUP_RAW_RECEIVED;

        cb.PFN_ENGAGE_GROUP_TIMELINE_EVENT_STARTED = on_ENGAGE_GROUP_TIMELINE_EVENT_STARTED;
        cb.PFN_ENGAGE_GROUP_TIMELINE_EVENT_UPDATED = on_ENGAGE_GROUP_TIMELINE_EVENT_UPDATED;
        cb.PFN_ENGAGE_GROUP_TIMELINE_EVENT_ENDED = on_ENGAGE_GROUP_TIMELINE_EVENT_ENDED;

        cb.PFN_ENGAGE_GROUP_TIMELINE_REPORT = on_ENGAGE_GROUP_TIMELINE_REPORT;
        cb.PFN_ENGAGE_GROUP_TIMELINE_REPORT_FAILED = on_ENGAGE_GROUP_TIMELINE_REPORT_FAILED;

        return engageRegisterCallbacks(ref cb);
    }

    private string makeUserJsonConfiguration(string alias, string displayName, int txPriority)
    {
        StringBuilder sb = new StringBuilder();

        // Note:  Alias is maxed at 16, so if we precede it with "C#", we 
        // can only use 14 hex characters for our random number portion of the ID.

        string myAlias = alias;
        string myDisplayName = displayName;
        int myTransmitPirority = txPriority;

        if (myAlias == null || myAlias.Length == 0)
        {
            myAlias = String.Format("C#{0:X14}", new Random().Next());
        }

        if (myDisplayName == null || myDisplayName.Length == 0)
        {
            myDisplayName = "C# User " + myAlias;
        }

        if(myTransmitPirority < 0)
        {
            myTransmitPirority = 0;
        }

        sb.Append("{");
            sb.Append("\"alias\":");
                sb.Append("\"" + myAlias + "\"");

            sb.Append(",\"displayName\":");
                sb.Append("\"" + myDisplayName + "\"");

            sb.Append(",\"txPriority\":");
                sb.Append(myTransmitPirority);
        sb.Append("}");

        return sb.ToString();
    }
    #endregion

    #region Member variables
    private static List<IEngineNotifications> _engineNotificationSubscribers = new List<IEngineNotifications>();
    private static List<IRallypointNotifications> _rallypointNotificationSubscribers = new List<IRallypointNotifications>();
    private static List<IGroupNotifications> _groupNotificationSubscribers = new List<IGroupNotifications>();
    private static List<ILicenseNotifications> _licenseNotificationSubscribers = new List<ILicenseNotifications>();
    #endregion

    #region Callback delegates
    private EngageVoidCallback on_ENGAGE_ENGINE_STARTED = () =>
    {
        lock (_engineNotificationSubscribers)
        {
            foreach (IEngineNotifications n in _engineNotificationSubscribers)
            {
                n.onEngineStarted();
            }
        }
    };

    private EngageVoidCallback on_ENGAGE_ENGINE_STOPPED = () =>
    {
        lock (_engineNotificationSubscribers)
        {
            foreach (IEngineNotifications n in _engineNotificationSubscribers)
            {
                n.onEngineStopped();
            }
        }
    };

    private EngageStringCallback on_ENGAGE_RP_PAUSING_CONNECTION_ATTEMPT = (string id) =>
    {
        lock (_rallypointNotificationSubscribers)
        {
            foreach (IRallypointNotifications n in _rallypointNotificationSubscribers)
            {
                n.onRallypointPausingConnectionAttempt(id);
            }
        }
    };

    private EngageStringCallback on_ENGAGE_RP_CONNECTING = (string id) =>
    {
        lock (_rallypointNotificationSubscribers)
        {
            foreach (IRallypointNotifications n in _rallypointNotificationSubscribers)
            {
                n.onRallypointConnecting(id);
            }
        }
    };

    private EngageStringCallback on_ENGAGE_RP_CONNECTED = (string id) =>
    {
        lock (_rallypointNotificationSubscribers)
        {
            foreach (IRallypointNotifications n in _rallypointNotificationSubscribers)
            {
                n.onRallypointConnected(id);
            }
        }
    };

    private EngageStringCallback on_ENGAGE_RP_DISCONNECTED = (string id) =>
    {
        lock (_rallypointNotificationSubscribers)
        {
            foreach (IRallypointNotifications n in _rallypointNotificationSubscribers)
            {
                n.onRallypointDisconnected(id);
            }
        }
    };

    private EngageStringAndTwoIntCallback on_ENGAGE_RP_ROUNDTRIP_REPORT = (string id, int rtMs, int rtRating) =>
    {
        lock (_rallypointNotificationSubscribers)
        {
            foreach (IRallypointNotifications n in _rallypointNotificationSubscribers)
            {
                n.onRallypointRoundtripReport(id, rtMs, rtRating);
            }
        }
    };

    private EngageStringCallback on_ENGAGE_GROUP_CREATED = (string id) =>
    {
        lock (_groupNotificationSubscribers)
        {
            foreach (IGroupNotifications n in _groupNotificationSubscribers)
            {
                n.onGroupCreated(id);
            }
        }
    };

    private EngageStringCallback on_ENGAGE_GROUP_CREATE_FAILED = (string id) =>
    {
        lock (_groupNotificationSubscribers)
        {
            foreach (IGroupNotifications n in _groupNotificationSubscribers)
            {
                n.onGroupCreateFailed(id);
            }
        }
    };

    private EngageStringCallback on_ENGAGE_GROUP_DELETED = (string id) =>
    {
        lock (_groupNotificationSubscribers)
        {
            foreach (IGroupNotifications n in _groupNotificationSubscribers)
            {
                n.onGroupDeleted(id);
            }
        }
    };

    private EngageStringCallback on_ENGAGE_GROUP_CONNECTED = (string id) =>
    {
        lock (_groupNotificationSubscribers)
        {
            foreach (IGroupNotifications n in _groupNotificationSubscribers)
            {
                n.onGroupConnected(id);
            }
        }
    };

    private EngageStringCallback on_ENGAGE_GROUP_CONNECT_FAILED = (string id) =>
    {
        lock (_groupNotificationSubscribers)
        {
            foreach (IGroupNotifications n in _groupNotificationSubscribers)
            {
                n.onGroupConnectFailed(id);
            }
        }
    };

    private EngageStringCallback on_ENGAGE_GROUP_DISCONNECTED = (string id) =>
    {
        lock (_groupNotificationSubscribers)
        {
            foreach (IGroupNotifications n in _groupNotificationSubscribers)
            {
                n.onGroupDisconnected(id);
            }
        }
    };

    private EngageStringCallback on_ENGAGE_GROUP_JOINED = (string id) =>
    {
        lock (_groupNotificationSubscribers)
        {
            foreach (IGroupNotifications n in _groupNotificationSubscribers)
            {
                n.onGroupJoined(id);
            }
        }
    };

    private EngageStringCallback on_ENGAGE_GROUP_JOIN_FAILED = (string id) =>
    {
        lock (_groupNotificationSubscribers)
        {
            foreach (IGroupNotifications n in _groupNotificationSubscribers)
            {
                n.onGroupJoinFailed(id);
            }
        }
    };

    private EngageStringCallback on_ENGAGE_GROUP_LEFT = (string id) =>
    {
        lock (_groupNotificationSubscribers)
        {
            foreach (IGroupNotifications n in _groupNotificationSubscribers)
            {
                n.onGroupLeft(id);
            }
        }
    };

    private EngageStringAndIntCallback on_ENGAGE_GROUP_MEMBER_COUNT_CHANGED = (string id, int newCount) =>
    {
        lock (_groupNotificationSubscribers)
        {
            foreach (IGroupNotifications n in _groupNotificationSubscribers)
            {
                n.onGroupMemberCountChanged(id, newCount);
            }
        }
    };

    private EngageStringCallback on_ENGAGE_GROUP_RX_STARTED = (string id) =>
    {
        lock (_groupNotificationSubscribers)
        {
            foreach (IGroupNotifications n in _groupNotificationSubscribers)
            {
                n.onGroupRxStarted(id);
            }
        }
    };

    private EngageStringCallback on_ENGAGE_GROUP_RX_ENDED = (string id) =>
    {
        lock (_groupNotificationSubscribers)
        {
            foreach (IGroupNotifications n in _groupNotificationSubscribers)
            {
                n.onGroupRxEnded(id);
            }
        }
    };

    private EngageStringCallback on_ENGAGE_GROUP_RX_MUTED = (string id) =>
    {
        lock (_groupNotificationSubscribers)
        {
            foreach (IGroupNotifications n in _groupNotificationSubscribers)
            {
                n.onGroupRxMuted(id);
            }
        }
    };

    private EngageStringCallback on_ENGAGE_GROUP_RX_UNMUTED = (string id) =>
    {
        lock (_groupNotificationSubscribers)
        {
            foreach (IGroupNotifications n in _groupNotificationSubscribers)
            {
                n.onGroupRxUnmuted(id);
            }
        }
    };

    private EngageString2Callback on_ENGAGE_GROUP_RX_SPEAKERS_CHANGED = (string id, string speakerjson) =>
    {
        lock (_groupNotificationSubscribers)
        {
            foreach (IGroupNotifications n in _groupNotificationSubscribers)
            {
                n.onGroupRxSpeakersChanged(id, speakerjson);
            }
        }
    };

    private EngageStringCallback on_ENGAGE_GROUP_TX_STARTED = (string id) =>
    {
        lock (_groupNotificationSubscribers)
        {
            foreach (IGroupNotifications n in _groupNotificationSubscribers)
            {
                n.onGroupTxStarted(id);
            }
        }
    };

    private EngageStringCallback on_ENGAGE_GROUP_TX_ENDED = (string id) =>
    {
        lock (_groupNotificationSubscribers)
        {
            foreach (IGroupNotifications n in _groupNotificationSubscribers)
            {
                n.onGroupTxEnded(id);
            }
        }
    };

    private EngageStringCallback on_ENGAGE_GROUP_TX_FAILED = (string id) =>
    {
        lock (_groupNotificationSubscribers)
        {
            foreach (IGroupNotifications n in _groupNotificationSubscribers)
            {
                n.onGroupTxFailed(id);
            }
        }
    };

    private EngageStringCallback on_ENGAGE_GROUP_TX_USURPED_BY_PRIORITY = (string id) =>
    {
        lock (_groupNotificationSubscribers)
        {
            foreach (IGroupNotifications n in _groupNotificationSubscribers)
            {
                n.onGroupTxUsurpedByPriority(id);
            }
        }
    };

    private EngageStringCallback on_ENGAGE_GROUP_MAX_TX_TIME_EXCEEDED = (string id) =>
    {
        lock (_groupNotificationSubscribers)
        {
            foreach (IGroupNotifications n in _groupNotificationSubscribers)
            {
                n.onGroupMaxTxTimeExceeded(id);
            }
        }
    };

    private EngageString2Callback on_ENGAGE_GROUP_NODE_DISCOVERED = (string id, string nodeJson) =>
    {
        lock (_groupNotificationSubscribers)
        {
            foreach (IGroupNotifications n in _groupNotificationSubscribers)
            {
                n.onGroupNodeDiscovered(id, nodeJson);
            }
        }
    };

    private EngageString2Callback on_ENGAGE_GROUP_NODE_REDISCOVERED = (string id, string nodeJson) =>
    {
        lock (_groupNotificationSubscribers)
        {
            foreach (IGroupNotifications n in _groupNotificationSubscribers)
            {
                n.onGroupNodeRediscovered(id, nodeJson);
            }
        }
    };

    private EngageString2Callback on_ENGAGE_GROUP_NODE_UNDISCOVERED = (string id, string nodeJson) =>
    {
        lock (_groupNotificationSubscribers)
        {
            foreach (IGroupNotifications n in _groupNotificationSubscribers)
            {
                n.onGroupNodeUndiscovered(id, nodeJson);
            }
        }
    };

    private EngageString2Callback on_ENGAGE_GROUP_ASSET_DISCOVERED = (string id, string nodeJson) =>
    {
        lock (_groupNotificationSubscribers)
        {
            foreach (IGroupNotifications n in _groupNotificationSubscribers)
            {
                n.onGroupAssetDiscovered(id, nodeJson);
            }
        }
    };

    private EngageString2Callback on_ENGAGE_GROUP_ASSET_REDISCOVERED = (string id, string nodeJson) =>
    {
        lock (_groupNotificationSubscribers)
        {
            foreach (IGroupNotifications n in _groupNotificationSubscribers)
            {
                n.onGroupAssetRediscovered(id, nodeJson);
            }
        }
    };

    private EngageString2Callback on_ENGAGE_GROUP_ASSET_UNDISCOVERED = (string id, string nodeJson) =>
    {
        lock (_groupNotificationSubscribers)
        {
            foreach (IGroupNotifications n in _groupNotificationSubscribers)
            {
                n.onGroupAssetUndiscovered(id, nodeJson);
            }
        }
    };

    private EngageVoidCallback on_ENGAGE_LICENSE_CHANGED = () =>
    {
        lock (_licenseNotificationSubscribers)
        {
            foreach (ILicenseNotifications n in _licenseNotificationSubscribers)
            {
                n.onLicenseChanged();
            }
        }
    };

    private EngageVoidCallback on_ENGAGE_LICENSE_EXPIRED = () =>
    {
        lock (_licenseNotificationSubscribers)
        {
            foreach (ILicenseNotifications n in _licenseNotificationSubscribers)
            {
                n.onLicenseExpired();
            }
        }
    };
    
    private EngageStringCallback on_ENGAGE_LICENSE_EXPIRING = (string secondsLeft) =>
    {
        lock (_licenseNotificationSubscribers)
        {
            foreach (ILicenseNotifications n in _licenseNotificationSubscribers)
            {
                n.onLicenseExpiring(Int64.Parse(secondsLeft));
            }
        }
    };

    private EngageStringCallback on_ENGAGE_GROUP_BLOB_SENT = (string id) =>
    {
        lock (_groupNotificationSubscribers)
        {
            foreach (IGroupNotifications n in _groupNotificationSubscribers)
            {
                n.onGroupBlobSent(id);
            }
        }
    };

    private EngageStringCallback on_ENGAGE_GROUP_BLOB_SEND_FAILED = (string id) =>
    {
        lock (_groupNotificationSubscribers)
        {
            foreach (IGroupNotifications n in _groupNotificationSubscribers)
            {
                n.onGroupBlobSendFailed(id);
            }
        }
    };

    private EngageString2AndBlobCallback on_ENGAGE_GROUP_BLOB_RECEIVED = (string id, string blobInfoJson, IntPtr blob, int blobSize) =>
    {
        lock (_groupNotificationSubscribers)
        {
            byte[] csBlob = new byte[blobSize];
            Marshal.Copy(blob, csBlob, 0, blobSize);

            foreach (IGroupNotifications n in _groupNotificationSubscribers)
            {
                n.onGroupBlobReceived(id, blobInfoJson, csBlob, blobSize);
            }
        }
    };

    private EngageStringCallback on_ENGAGE_GROUP_RTP_SENT = (string id) =>
    {
        lock (_groupNotificationSubscribers)
        {
            foreach (IGroupNotifications n in _groupNotificationSubscribers)
            {
                n.onGroupRtpSent(id);
            }
        }
    };

    private EngageStringCallback on_ENGAGE_GROUP_RTP_SEND_FAILED = (string id) =>
    {
        lock (_groupNotificationSubscribers)
        {
            foreach (IGroupNotifications n in _groupNotificationSubscribers)
            {
                n.onGroupRtpSendFailed(id);
            }
        }
    };

    private EngageString2AndBlobCallback on_ENGAGE_GROUP_RTP_RECEIVED = (string id, string rtpHeaderJson, IntPtr payload, int payloadSize) =>
    {
        lock (_groupNotificationSubscribers)
        {
            byte[] csPayload = new byte[payloadSize];
            Marshal.Copy(payload, csPayload, 0, payloadSize);

            foreach (IGroupNotifications n in _groupNotificationSubscribers)
            {
                n.onGroupRtpReceived(id, rtpHeaderJson, csPayload, payloadSize);
            }
        }
    };

    private EngageStringCallback on_ENGAGE_GROUP_RAW_SENT = (string id) =>
    {
        lock (_groupNotificationSubscribers)
        {
            foreach (IGroupNotifications n in _groupNotificationSubscribers)
            {
                n.onGroupRawSent(id);
            }
        }
    };

    private EngageStringCallback on_ENGAGE_GROUP_RAW_SEND_FAILED = (string id) =>
    {
        lock (_groupNotificationSubscribers)
        {
            foreach (IGroupNotifications n in _groupNotificationSubscribers)
            {
                n.onGroupRawSendFailed(id);
            }
        }
    };

    private EngageStringAndBlobCallback on_ENGAGE_GROUP_RAW_RECEIVED = (string id, IntPtr raw, int rawSize) =>
    {
        lock (_groupNotificationSubscribers)
        {
            byte[] csRaw = new byte[rawSize];
            Marshal.Copy(raw, csRaw, 0, rawSize);

            foreach (IGroupNotifications n in _groupNotificationSubscribers)
            {
                n.onGroupRawReceived(id, csRaw, rawSize);
            }
        }
    };

    private EngageString2Callback on_ENGAGE_GROUP_TIMELINE_EVENT_STARTED = (string id, string eventJson) =>
    {
        lock (_groupNotificationSubscribers)
        {
            foreach (IGroupNotifications n in _groupNotificationSubscribers)
            {
                n.onGroupTimelineEventStarted(id, eventJson);
            }
        }
    };

    private EngageString2Callback on_ENGAGE_GROUP_TIMELINE_EVENT_UPDATED = (string id, string eventJson) =>
    {
        lock (_groupNotificationSubscribers)
        {
            foreach (IGroupNotifications n in _groupNotificationSubscribers)
            {
                n.onGroupTimelineEventUpdated(id, eventJson);
            }
        }
    };

    private EngageString2Callback on_ENGAGE_GROUP_TIMELINE_EVENT_ENDED = (string id, string eventJson) =>
    {
        lock (_groupNotificationSubscribers)
        {
            foreach (IGroupNotifications n in _groupNotificationSubscribers)
            {
                n.onGroupTimelineEventEnded(id, eventJson);
            }
        }
    };

    private EngageString2Callback on_ENGAGE_GROUP_TIMELINE_REPORT = (string id, string reportJson) =>
    {
        lock (_groupNotificationSubscribers)
        {
            foreach (IGroupNotifications n in _groupNotificationSubscribers)
            {
                n.onGroupTimelineReport(id, reportJson);
            }
        }
    };

    private EngageStringCallback on_ENGAGE_GROUP_TIMELINE_REPORT_FAILED = (string id) =>
    {
        lock (_groupNotificationSubscribers)
        {
            foreach (IGroupNotifications n in _groupNotificationSubscribers)
            {
                n.onGroupTimelineReportFailed(id);
            }
        }
    };
    #endregion

    #region Public functions
    public void subscribe(IEngineNotifications n)
    {
        lock(_engineNotificationSubscribers)
        {
            _engineNotificationSubscribers.Add(n);
        }
    }

    public void unsubscribe(IEngineNotifications n)
    {
        lock (_engineNotificationSubscribers)
        {
            _engineNotificationSubscribers.Remove(n);
        }
    }

    public void subscribe(IRallypointNotifications n)
    {
        lock (_rallypointNotificationSubscribers)
        {
            _rallypointNotificationSubscribers.Add(n);
        }
    }

    public void unsubscribe(IRallypointNotifications n)
    {
        lock (_rallypointNotificationSubscribers)
        {
            _rallypointNotificationSubscribers.Remove(n);
        }
    }

    public void subscribe(IGroupNotifications n)
    {
        lock (_groupNotificationSubscribers)
        {
            _groupNotificationSubscribers.Add(n);
        }
    }

    public void unsubscribe(IGroupNotifications n)
    {
        lock (_groupNotificationSubscribers)
        {
            _groupNotificationSubscribers.Remove(n);
        }
    }

    public void subscribe(ILicenseNotifications n)
    {
        lock (_licenseNotificationSubscribers)
        {
            _licenseNotificationSubscribers.Add(n);
        }
    }

    public void unsubscribe(ILicenseNotifications n)
    {
        lock (_licenseNotificationSubscribers)
        {
            _licenseNotificationSubscribers.Remove(n);
        }
    }

    public int initialize(string enginePolicyConfiguration, string userIdentity, string tempStoragePath)
    {
        int rc;

        rc = registerCallbacks();
        if(rc != ENGAGE_RESULT_OK)
        {
            return rc;
        }

        return engageInitialize(enginePolicyConfiguration, userIdentity, tempStoragePath);
    }

    public int shutdown()
    {
        return engageShutdown();
    }

    public int start()
    {
        return engageStart();
    }

    public int stop()
    {
        return engageStop();
    }

    public int createGroup(string jsonConfiguration)
    {
        return engageCreateGroup(jsonConfiguration);
    }

    public int deleteGroup(string id)
    {
        return engageDeleteGroup(id);
    }

    public int joinGroup(string id)
    {
        return engageJoinGroup(id);
    }

    public int leaveGroup(string id)
    {
        return engageLeaveGroup(id);
    }

    public int beginGroupTx(string id, int txPriority, int txFlags)
    {
        return engageBeginGroupTx(id, txPriority, txFlags);
    }

    public int beginGroupTxAdvanced(string id, string jsonParams)
    {
        return engageBeginGroupTxAdvanced(id, jsonParams);
    }

    public int endGroupTx(string id)
    {
        return engageEndGroupTx(id);
    }

    public int setGroupRxTag(string id, int tag)
    {
        return engageSetGroupRxTag(id, tag);
    }

    public int muteGroupRx(string id)
    {
        return engageMuteGroupRx(id);
    }    

    public int unmuteGroupRx(string id)
    {
        return engageUnmuteGroupRx(id);
    }    

    public int setGroupRxVolume(string id, int left, int right)
    {
        return engageSetGroupRxVolume(id, left, right);
    }

    public int queryGroupTimeline(string id, string jsonParams)
    {
        return engageQueryGroupTimeline(id, jsonParams);
    }

    public int logMsg(int level, string tag, string msg)
    {
        return engageLogMsg(level, tag, msg);
    }

    public String getVersion()
    {
        IntPtr ptr = engageGetVersion();

        if (ptr == IntPtr.Zero)
        {
            return null;
        }
        else
        {
            return Marshal.PtrToStringAnsi(ptr);
        }
    }

    public String getActiveLicenseDescriptor()
    {
        IntPtr ptr = engageGetActiveLicenseDescriptor();

        if (ptr == IntPtr.Zero)
        {
            return null;
        }
        else
        {
            return Marshal.PtrToStringAnsi(ptr);
        }
    }

    public String getLicenseDescriptor(string entitlement, string key, string activationCode)
    {
        IntPtr ptr = engageGetLicenseDescriptor(entitlement, key, activationCode);

        if (ptr == IntPtr.Zero)
        {
            return null;
        }
        else
        {
            return Marshal.PtrToStringAnsi(ptr);
        }
    }

    public int updateLicense(string entitlement, string key, string activationCode)
    {
        return engageUpdateLicense(entitlement, key, activationCode);
    }

    #endregion
}
