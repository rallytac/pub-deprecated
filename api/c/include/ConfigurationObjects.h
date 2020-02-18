//
//  Copyright (c) 2019 Rally Tactical Systems, Inc.
//  All rights reserved.
//

/** \file ConfigurationObjects.h
    \brief Place to look for configuration helper
*/

/**
 * Detailed description of the ConfigurationObjects.h file
 *
 * This file contains all the configuration objects 
 *
 *  Copyright (c) 2018 Rally Tactical Systems, Inc.
 *  All rights reserved.
 *
 */

#ifndef ConfigurationObjects_h
#define ConfigurationObjects_h

#include "Platform.h"
#include <iostream>
#include <cstddef>
#include <cstdint>
#include <chrono>
#include <vector>
#include <string>

#include <nlohmann/json.hpp>

#ifndef WIN32
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-function"
#endif

#if !defined(ENGAGE_IGNORE_COMPILER_UNUSED_WARNING)
    #if defined(__GNUC__)
        #define ENGAGE_IGNORE_COMPILER_UNUSED_WARNING __attribute__((unused))
    #else
        #define ENGAGE_IGNORE_COMPILER_UNUSED_WARNING
    #endif
#endif  // ENGAGE_IGNORE_COMPILER_UNUSED_WARNING


namespace ConfigurationObjects
{

    //-----------------------------------------------------------
    #pragma pack(push, 1)
        typedef struct
        {
            /** @brief DataSeries Type.
             * Currently supported types
             * 
             * Type | Value | Range | JSON Object Name | Detail
             * --- | ---  | ---  | --- | ---
             * Heart Rate | 1 | 0-255 | heartRate | Beats per minute
             * Skin Temperature | 2 | 0-255 | skinTemp | Celsius
             * Core Temperature | 3 | 0-255 | coreTemp | Celsius
             * Hydration Percentage | 4 | 0-100 | hydration | Percentage
             * Blood Oxygenation Percentage | 5 | 0-100 | bloodOxygenation | Percentage
             * Fatigue Level | 6 | 0-10 | fatigueLevel | 0 = low fatigue, 10 = maximum fatigue
             * Task Effectiveness Level | 7 | 0-10 | taskEffectiveness | 0 = minimal effectiveness, 10 = maximum effectiveness
             *
             */
            uint8_t     t;          

            /** 
             * @brief Timestamp representing the number of seconds elapsed since January 1, 1970 -  based on traditional Unix time
             */
            uint32_t    ts;

            /** @brief Increment type.
             *  Valid Types:
             * 
             * Type  | Value
             * ------------- | -------------
             * 0 | Seconds 
             * 1 | Milliseconds
             * 2 | Minutes
             * 3 | Hours
             * 4 | Days 
             * 
             */
            uint8_t     it;         

            /** 
             * @brief Increment multiplier.
             * The increment multiplier is an additional field that allows you apply a multiplier of 1-255 for time offset increments. 
             * For example: let's assume that our data is being gathered at 20-millisecond intervals. We could certainly represent the 
             * data as [0,37,60,38,20,42,140,36] where the time offset "3" becomes "60", "1" becomes "20", and "7" becomes "140".
             * 
             * Valid range is 1 to 255
             */ 
            uint8_t     im;        
            
            /** 
             * @brief Value type
             */  
            uint8_t     vt;    
            
            /** 
             * @brief Series size (element count)
             */  
            uint8_t     ss; 
        } DataSeriesHeader_t;

        typedef struct 
        {
            uint8_t     ofs;
            uint8_t     val;
        } DataElementUint8_t;        

        typedef struct 
        {
            uint8_t     ofs;
            uint16_t    val;
        } DataElementUnint16_t;      

        typedef struct 
        {
            uint8_t     ofs;
            uint32_t    val;
        } DataElementUnint32_t;      

        typedef struct 
        {
            uint8_t     ofs;
            uint64_t    val;
        } DataElementUnint64_t;        
    #pragma pack(pop)

    typedef enum 
    {  
        invalid = 0,
        uint8 = 1,
        uint16 = 2,
        uint32 = 3,
        uint64 = 4
    } DataSeriesValueType_t;
    
    /** 
     * @brief Human Biometric Types.
     * 
     * TODO: More detailed TxCodec_t description. 
     */
    typedef enum 
    {  
        unknown = 0,
        heartRate = 1,
        skinTemp = 2,
        coreTemp = 3,
        hydration = 4,
        bloodOxygenation = 5,
        fatigueLevel = 6,
        taskEffectiveness = 7
    } HumanBiometricsTypes_t;
    
    //-----------------------------------------------------------

    static FILE *_internalFileOpener(const char *fn, const char *mode)
    {
        FILE *fp = nullptr;

        #ifndef WIN32
            fp = fopen(fn, mode);
        #else
            if(fopen_s(&fp, fn, mode) != 0)
            {
                fp = nullptr;
            }
        #endif

        return fp;
    }

    #define JSON_SERIALIZED_CLASS(_cn) \
        class _cn; \
        static void to_json(nlohmann::json& j, const _cn& p); \
        static void from_json(const nlohmann::json& j, _cn& p);

    #define IMPLEMENT_JSON_DOCUMENTATION(_cn) \
        public: \
        static void document(const char *path = nullptr) \
        { \
            _cn   example; \
            example.initForDocumenting(); \
            std::string theJson = example.serialize(3); \
            std::cout << "------------------------------------------------" << std::endl \
                      << #_cn << std::endl \
                      << theJson << std::endl \
                      << "------------------------------------------------" << std::endl; \
            \
            if(path != nullptr && path[0] != 0) \
            { \
                std::string fn = path; \
                fn.append("/"); \
                fn.append(#_cn); \
                fn.append(".json");  \
                \
                FILE *fp = _internalFileOpener(fn.c_str(), "wt");\
                \
                if(fp != nullptr) \
                { \
                    fputs(theJson.c_str(), fp); \
                    fclose(fp); \
                } \
                else \
                { \
                    std::cout << "ERROR: Cannot write to " << fn << std::endl; \
                } \
            } \
        }

    #define IMPLEMENT_JSON_SERIALIZATION() \
        public: \
        bool deserialize(const char *s) \
        { \
            try \
            { \
                if(s != nullptr && s[0] != 0) \
                { \
                    from_json(nlohmann::json::parse(s), *this); \
                } \
                else \
                { \
                    return false; \
                } \
            } \
            catch(...) \
            { \
                return false; \
            } \
            return true; \
        } \
        \
        std::string serialize(const int indent = -1) \
        { \
            nlohmann::json j; to_json(j, *this); \
            return j.dump(indent); \
        }

    #define TOJSON_IMPL(__var) \
        {#__var, p.__var}

    #define FROMJSON_IMPL_SIMPLE(__var) \
        getOptional(#__var, p.__var, j)

    #define FROMJSON_IMPL(__var, __type, __default) \
        getOptional<__type>(#__var, p.__var, j, __default)


    //-----------------------------------------------------------
    static std::string EMPTY_STRING;

    template<class T>
    static void getOptional(const char *name, T& v, const nlohmann::json& j, T def)
    {
        try
        {
            j.at(name).get_to(v);
        }
        catch(...)
        {
            v = def;
        }
    }
    
    template<class T>
    static void getOptional(const char *name, T& v, const nlohmann::json& j)
    {
        try
        {
            j.at(name).get_to(v);
        }
        catch(...)
        {
        }
    }

    static bool readTextFileIntoString(const char *fn, std::string& str)
    {
        bool rc = false;
		FILE *fp;

		#ifndef WIN32
				fp = fopen(fn, "rb");
		#else
				if (fopen_s(&fp, fn, "rb") != 0)
				{
					fp = nullptr;
				}
		#endif

        if(fp != nullptr)
        {
            fseek(fp, 0, SEEK_END);
            long sz = ftell(fp);
            fseek(fp, 0, SEEK_SET);

            if(sz > 0)
            {
                char *buff = new char[sz + 1];
                long read = (long)fread(buff, 1, sz, fp);

                if(read == sz)
                {
                    buff[sz] = 0;
                    str = buff;
                    rc = true;
                }
                
                delete []buff;
            }
            
            fclose(fp);
        }

        return rc;
    }

    class ConfigurationObjectBase
    {
    public:
        ConfigurationObjectBase()              
        {   
            _documenting = false;         
        }

        virtual ~ConfigurationObjectBase()
        {            
        }

        virtual void initForDocumenting()
        {
            _documenting = true;
        }

    protected:
        bool _documenting;
    };    

    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(NetworkInterfaceDevice)
    class NetworkInterfaceDevice : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(NetworkInterfaceDevice)
        
    public:
        std::string                 name;
        std::string                 friendlyName;
        std::string                 description;
        int                         family;
        std::string                 address;
        bool                        available;
        bool                        isLoopback;
        bool                        supportsMulticast;
        std::string                 hardwareAddress;

        NetworkInterfaceDevice()
        {
            clear();
        }

        void clear()
        {
            name.clear();
            friendlyName.clear();
            description.clear();
            family = -1;
            address.clear();
            available = false;
            isLoopback = false;
            supportsMulticast = false;
            hardwareAddress.clear();
        }

        virtual void initForDocumenting()
        {
            clear();
            name = "en0";
            friendlyName = "Wi-Fi";
            description = "A wi-fi adapter";
            family = 1;
            address = "127.0.0.1";
            available = true;
            isLoopback = true;
            supportsMulticast = false;
            hardwareAddress = "DE:AD:BE:EF:01:02:03";
        }
    };
    
    static void to_json(nlohmann::json& j, const NetworkInterfaceDevice& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(name),
            TOJSON_IMPL(friendlyName),
            TOJSON_IMPL(description),
            TOJSON_IMPL(family),
            TOJSON_IMPL(address),
            TOJSON_IMPL(available),
            TOJSON_IMPL(isLoopback),
            TOJSON_IMPL(supportsMulticast),
            TOJSON_IMPL(hardwareAddress)
        };
    }
    static void from_json(const nlohmann::json& j, NetworkInterfaceDevice& p)
    {
        p.clear();
        getOptional("name", p.name, j);
        getOptional("friendlyName", p.friendlyName, j);
        getOptional("description", p.description, j);        
        getOptional("family", p.family, j, -1);
        getOptional("address", p.address, j);
        getOptional("available", p.available, j, false);
        getOptional("isLoopback", p.isLoopback, j, false);
        getOptional("supportsMulticast", p.supportsMulticast, j, false);
        getOptional("hardwareAddress", p.hardwareAddress, j);
    }    

    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(ListOfNetworkInterfaceDevice)
    class ListOfNetworkInterfaceDevice : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(ListOfNetworkInterfaceDevice)
        
    public:
        std::vector<NetworkInterfaceDevice> list;

        ListOfNetworkInterfaceDevice()
        {
            clear();
        }

        void clear()
        {
            list.clear();
        }
    };
    
    static void to_json(nlohmann::json& j, const ListOfNetworkInterfaceDevice& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(list)
        };
    }
    static void from_json(const nlohmann::json& j, ListOfNetworkInterfaceDevice& p)
    {
        p.clear();
        getOptional<std::vector<NetworkInterfaceDevice>>("list", p.list, j);
    }


    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(RtpHeader)
    /** 
     * @brief RTP header information as per <a href="https://tools.ietf.org/html/rfc3550" target="_blank">RFC 3550</a>.
     * 
     * @see BlobInfo 
     * 
     * Example JSON: @include[lineno] examples/RtpHeader.json
     *  
     */    
    class RtpHeader : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(RtpHeader)
        
    public:

        /** @brief A valid RTP payload between 0 and 127 <a href="https://www.iana.org/assignments/rtp-parameters/rtp-parameters.xhtml" target="_blank">See IANA Real-Time Transport Protocol (RTP) Parameters</a>  */
        int         pt;

        /** @brief Indicates whether this is the start of the media stream burst. */
        bool        marker;

        /** @brief Packet sequence number. */
        uint16_t    seq;

        /** @brief Psuedo-random synchronization source. */
        uint32_t    ssrc;

        /** @brief Media sample timestamp. */
        uint32_t    ts;

        RtpHeader()
        {
            clear();
        }

        void clear()
        {
            pt = -1;
            marker = false;
            seq = 0;
            ssrc = 0;
            ts = 0;
        }

        virtual void initForDocumenting()
        {
            clear();
            pt = 0;
            marker = false;
            seq = 123;
            ssrc = 12345678;
            ts = 87654321;
        }
    };
    
    static void to_json(nlohmann::json& j, const RtpHeader& p)
    {
        if(p.pt != -1)
        {
            j = nlohmann::json{
                TOJSON_IMPL(pt),
                TOJSON_IMPL(marker),
                TOJSON_IMPL(seq),
                TOJSON_IMPL(ssrc),
                TOJSON_IMPL(ts)
            };
        }
    }
    static void from_json(const nlohmann::json& j, RtpHeader& p)
    {
        p.clear();
        getOptional<int>("pt", p.pt, j, -1);
        getOptional<bool>("marker", p.marker, j, false);
        getOptional<uint16_t>("seq", p.seq, j, 0);
        getOptional<uint32_t>("ssrc", p.ssrc, j, 0);
        getOptional<uint32_t>("ts", p.ts, j, 0);
    }

    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(BlobInfo)
    /** @brief Describes the Blob data being sent used in the @ref engageSendGroupBlob API
     *  
     *  TODO: Shaun, anything you want to add here 
     * 
     *  Helper C++ class to serialize and de-serialize BlobInfo JSON 
     * 
     *  Example JSON: @include[lineno] examples/BlobInfo.json 
     * 
     *  @see engageSendGroupBlob
     */        
    class BlobInfo : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(BlobInfo)
        
    public:
        /** @brief Payload type.
         * BlobInfo RTP supported Payload types. 
         */
        typedef enum
        {
            /** @brief Unknown type */
            bptUndefined                    = 0,

            /** @brief Plain UTF 8 text. This is typically used to send plain text messages */
            bptAppTextUtf8                  = 1,

            /** @brief JSON UFT 8 text. This is used to send JSON formatted messages */            
            bptJsonTextUtf8                 = 2, 

            /** @brief Binary payload. Used to send binary data */            
            bptAppBinary                    = 3,

            /** @brief Biometrics payload. Used to send biometric series data. <a href="https://github.com/rallytac/pub/wiki/Engage-Human-Biometrics" target="_blank">See WIKI post for more info</a> */            
            bptEngageBinaryHumanBiometrics  = 4
        } PayloadType_t;

        /** @brief [Optional, Default : 0] Size of the payload */
        size_t          size;

        /** @brief [Optional, Default: empty string] The nodeId of Engage Engine that sent the message. If this is empty the Blob info will be anonymous. */
        std::string     source;

        /** @brief [Optional, Default: empty string] The nodeId to which this message is targeted. If this is empty, the Blob will targeted at all endpoints. */
        std::string     target;

        /** @brief [Optional, Default: @ref bptUndefined] The payload type to send in the blob @see PayloadType_t */
        PayloadType_t   payloadType;

        /** @brief Custom RTP header */
        RtpHeader       rtpHeader;

        BlobInfo()
        {
            clear();
        }

        void clear()
        {
            size = 0;
            source.clear();
            target.clear();
            rtpHeader.clear();
            payloadType = PayloadType_t::bptUndefined;
        }

        virtual void initForDocumenting()
        {
            clear();
            rtpHeader.initForDocumenting();
        }
    };
    
    static void to_json(nlohmann::json& j, const BlobInfo& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(size),
            TOJSON_IMPL(source),
            TOJSON_IMPL(target),
            TOJSON_IMPL(rtpHeader),
            TOJSON_IMPL(payloadType)
        };
    }
    static void from_json(const nlohmann::json& j, BlobInfo& p)
    {
        p.clear();
        getOptional<size_t>("size", p.size, j, 0);
        getOptional<std::string>("source", p.source, j, EMPTY_STRING);
        getOptional<std::string>("target", p.target, j, EMPTY_STRING);
        getOptional<RtpHeader>("rtpHeader", p.rtpHeader, j);
        getOptional<BlobInfo::PayloadType_t>("payloadType", p.payloadType, j, BlobInfo::PayloadType_t::bptUndefined);
    }

    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(AdvancedTxParams)
    /** 
     * @brief Configuration when using the @ref engageBeginGroupTxAdvanced API   
     * 
     * Helper C++ class to serialize and de-serialize AdvancedTxParams JSON 
     * 
     * Example JSON: @include[lineno] examples/AdvancedTxParams.json 
     * 
     * TODO: Completed class properties 
     * 
     * @see engageBeginGroupTxAdvanced
     */            
    class AdvancedTxParams : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(AdvancedTxParams)
        
    public:

        /** @brief TODO: Shaun where are these flags defined? */
        uint16_t        flags;

        /** @brief [Optional, Default: 0] Transmit priority between 0 (lowest) and 255 (highest). */
        uint8_t         priority;

        /** @brief [Optional, Default: 0] Defines a sub channel within a group. Audio will be opaque to all other clients on the same group not on the same sub channel. */
        uint16_t        subchannelTag;

        /** @brief [Optional, Default: false] The Engage Engine should transmit the NodeId as part of the header extension in the RTP packet. */
        bool            includeNodeId;

        /** @brief [Optional, Default: empty string] The Engage Engine should transmit the user's alias as part of the header extension in the RTP packet. */
        std::string     alias;

        /** @brief [Optional, Default: empty string] The Engage Engine should play the audio specified by this URI while TX is pending. */
        std::string     pendingAudioUri;

        /** @brief [Optional, Default: empty string] The Engage Engine should play the audio specified by this URI when TX is granted. */
        std::string     grantAudioUri;

        /** @brief [Optional, Default: empty string] The Engage Engine should play the audio specified by this URI when TX is denied. */
        std::string     denyAudioUri;

        AdvancedTxParams()
        {
            clear();
        }

        void clear()
        {
            flags = 0;
            priority = 0;
            subchannelTag = 0;
            includeNodeId = false;
            alias.clear();
            pendingAudioUri.clear();
            grantAudioUri.clear();
            denyAudioUri.clear();
        }

        virtual void initForDocumenting()
        {            
        }
    };
    
    static void to_json(nlohmann::json& j, const AdvancedTxParams& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(flags),
            TOJSON_IMPL(priority),
            TOJSON_IMPL(subchannelTag),
            TOJSON_IMPL(includeNodeId),
            TOJSON_IMPL(alias),
            TOJSON_IMPL(pendingAudioUri),
            TOJSON_IMPL(grantAudioUri),
            TOJSON_IMPL(denyAudioUri)
        };
    }
    static void from_json(const nlohmann::json& j, AdvancedTxParams& p)
    {
        p.clear();
        getOptional<uint16_t>("flags", p.flags, j, 0);
        getOptional<uint8_t>("priority", p.priority, j, 0);
        getOptional<uint16_t>("subchannelTag", p.subchannelTag, j, 0);
        getOptional<bool>("includeNodeId", p.includeNodeId, j, false);
        getOptional<std::string>("alias", p.alias, j, EMPTY_STRING);
        getOptional<std::string>("pendingAudioUri", p.pendingAudioUri, j, EMPTY_STRING);
        getOptional<std::string>("grantAudioUri", p.grantAudioUri, j, EMPTY_STRING);
        getOptional<std::string>("denyAudioUri", p.denyAudioUri, j, EMPTY_STRING);
    }

    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(Identity)
    /** 
     * @brief Users Identity
     * 
     * Helper C++ class to serialize and de-serialize Identity JSON 
     * 
     * This configuration will be used by the Engine when transmitting presence data on behalf of the application. 
     * 
     * Example JSON: @include[lineno] examples/Identity.json 
     * 
     * @see engageInitialize, PresenceDescriptor
     */        
    class Identity : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(Identity)
        
    public:
        
        /** 
         * @brief [Optional, Default: Auto Generated] This is the Node ID to use to represent instance on the network.
         *
         * The application can generate and persist this Id or if an Id is not provided, the Engage Engine will generate 
            a random Id and will be advertised in the PresenceDescriptor JSON with the <b>self</b> attribute 
            set to <b>true</B> in the PFN_ENGAGE_GROUP_NODE_DISCOVERED event @see PresenceDescriptor.
         */     
        std::string     nodeId;

        /** @brief [Optional, Default: empty string] The user ID to be used to represent the user. */ 
        std::string     userId;         

        /** @brief [Optional, Default: empty string] The display name to be used for the user. */
        std::string     displayName;

        /** @brief [Optional, Default: empty string] This is a application defined field used to indicate a users avatar. The Engine doesn't not process this data and is a pass through. */ 
        std::string     avatar; 

        Identity()
        {
            clear();
        }

        void clear()
        {
            nodeId.clear();
            userId.clear();
            displayName.clear();
            avatar.clear();
        }

        virtual void initForDocumenting()
        {            
        }
    };
    
    static void to_json(nlohmann::json& j, const Identity& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(nodeId),
            TOJSON_IMPL(userId),
            TOJSON_IMPL(displayName),
            TOJSON_IMPL(avatar)
        };
    }
    static void from_json(const nlohmann::json& j, Identity& p)
    {
        p.clear();
        getOptional<std::string>("nodeId", p.nodeId, j);
        getOptional<std::string>("userId", p.userId, j);
        getOptional<std::string>("displayName", p.displayName, j);
        getOptional<std::string>("avatar", p.avatar, j);
    }


    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(Location)
    /** 
     * @brief Location information used as part of the @ref PresenceDescriptor
     * 
     * Helper C++ class to serialize and de-serialize Location JSON 
     * 
     * Location is presented by using the <a href="https://gisgeography.com/wgs84-world-geodetic-system/" target="_blank">World Geodetic System (WGS84)</a>
     *  
     * Example JSON: @include[lineno] examples/Location.json 
     * 
     *  @see PresenceDescriptor  
     */            
    class Location : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(Location)

    public:
        constexpr static double INVALID_LOCATION_VALUE = -999.999;

        /** @brief [Read Only: Unix timestamp - Zulu/UTC] Indicates the timestamp that the location was recorded. */
        uint32_t                    ts;             
        
        /** @brief Its the latitude position using the using the <b>Signed degrees format</b> (DDD.dddd). Valid range is -90 to 90. */ 
        double                      latitude;       
        
        /** @brief Its the longitudinal position using the <b>Signed degrees format</b> (DDD.dddd) format. Valid range is -180 to 180. */ 
        double                      longitude; 
        
        /** @brief [Optional, Default: INVALID_LOCATION_VALUE] The altitude above sea level in meters. */ 
        double                      altitude; 
        
        /** @brief [Optional, Default: INVALID_LOCATION_VALUE] Direction the endpoint is traveling in degrees. Valid range is 0 to 360 (NOTE: 0 and 360 are both the same heading). */ 
        double                      direction;
        
        /** @brief [Optional, Default: INVALID_LOCATION_VALUE] The speed the endpoint is traveling at in meters per second. */ 
        double                      speed;

        Location()
        {
            clear();
        }

        void clear()
        {
            ts = 0;
            latitude = INVALID_LOCATION_VALUE;
            longitude = INVALID_LOCATION_VALUE;
            altitude = INVALID_LOCATION_VALUE;
            direction = INVALID_LOCATION_VALUE;
            speed = INVALID_LOCATION_VALUE;
        }

        virtual void initForDocumenting()
        {
            clear();

            ts = 123456;
            latitude = 123.456;
            longitude = 456.789;
            altitude = 123;
            direction = 1;
            speed = 1234;
        }
    };
    
    static void to_json(nlohmann::json& j, const Location& p)
    {
        j = nlohmann::json{            
            TOJSON_IMPL(latitude),
            TOJSON_IMPL(longitude),
        };

        if(p.ts != 0) j["ts"] = p.ts;
        if(p.altitude != Location::INVALID_LOCATION_VALUE) j["altitude"] = p.altitude;
        if(p.speed != Location::INVALID_LOCATION_VALUE) j["speed"] = p.speed;
        if(p.direction != Location::INVALID_LOCATION_VALUE) j["direction"] = p.direction;
    }
    static void from_json(const nlohmann::json& j, Location& p)
    {
        p.clear();
        getOptional<uint32_t>("ts", p.ts, j, 0);
        j.at("latitude").get_to(p.latitude);
        j.at("longitude").get_to(p.longitude);
        getOptional<double>("altitude", p.altitude, j, Location::INVALID_LOCATION_VALUE);
        getOptional<double>("direction", p.direction, j, Location::INVALID_LOCATION_VALUE);
        getOptional<double>("speed", p.speed, j, Location::INVALID_LOCATION_VALUE);
    }        

    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(Power)
    /** 
     * @brief Device Power Information used as part of the @ref PresenceDescriptor
     * 
     *  Helper C++ class to serialize and de-serialize Power JSON 
     * 
     * Example JSON: @include[lineno] examples/Power.json 
     * 
     *  @see PresenceDescriptor  
     */                
    class Power : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(Power)

    public:

        /** @brief [Optional, Default: 0] Is the source the power is being delivered from
         *
         * Valid values are 
         *  
         * Value | Description 
         * --- | ---  
         *  0 | Undefined 
         *  1 | Battery 
         *  2 | Wired
         *  
         */    
        int     source;

        /** @brief [Optional, Default: 0] Is the current state that the power system is in.
         *
         * Valid values are 
         * 
         * Value | Description 
         * --- | ---  
         *  0 | Undefined 
         *  1 | Charging 
         *  2 | Discharging
         *  3 | Not Charging
         *  4 | Full
         */    
        int     state;

        /** @brief [Optional, Default: 0] Is the current level of the battery or power system as a percentage. Valid range is 0 to 100 TODO: Shaun, is this a percentage? */    
        int     level; 

        Power()
        {
            clear();
        }

        void clear()
        {
            source = 0;
            state = 0;
            level = 0;
        }

        virtual void initForDocumenting()
        {            
        }
    };
    
    static void to_json(nlohmann::json& j, const Power& p)
    {
        j = nlohmann::json{            
            TOJSON_IMPL(source),
            TOJSON_IMPL(state),
            TOJSON_IMPL(level)
        };
    }
    static void from_json(const nlohmann::json& j, Power& p)
    {
        p.clear();
        getOptional<int>("source", p.source, j, 0);
        getOptional<int>("state", p.state, j, 0);
        getOptional<int>("level", p.level, j, 0);
    }


    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(Connectivity)
    /** 
     * @brief Connectivity Information used as part of the @ref PresenceDescriptor
     * 
     *  Helper C++ class to serialize and de-serialize Connectivity JSON 
     * 
     * Example JSON: @include[lineno] examples/Connectivity.json  TODO: Shaun, Connectivity.json is null
     * 
     *  @see PresenceDescriptor  
     */                
    class Connectivity : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(Connectivity)

    public:
        /** 
         * @brief Is the type of connectivity the device has to the network 
         *
         * Valid values are 
         * 
         * Value | Description 
         * --- | ---  
         *  0 | Undefined 
         *  1 | Wired
         *  2 | Wireless WiFi
         *  3 | Wireless Cellular
         *  
         */    
        int     type;  

        /** @brief Is the strength of the connection connection. TODO: Shaun, what is this measured in? */
        int     strength; 

        /** @brief Is the round trip rating the Engine performs on the connection to a Rallypoint. TODO: Shaun, what are the valid ratings and what do they mean? */
        int     rating; 

        Connectivity()
        {
            clear();
        }

        void clear()
        {
            type = 0;
            strength = 0;
            rating = 0;
        }

        virtual void initForDocumenting()
        {
            clear();

            type = 1;
            strength = 2;
            rating = 3;
        }
    };
    
    static void to_json(nlohmann::json& j, const Connectivity& p)
    {
        if(p.type != 0)
        {
            j = nlohmann::json{            
                TOJSON_IMPL(type),
                TOJSON_IMPL(strength),
                TOJSON_IMPL(rating)
            };
        }
    }
    static void from_json(const nlohmann::json& j, Connectivity& p)
    {
        p.clear();
        getOptional<int>("type", p.type, j, 0);
        getOptional<int>("strength", p.strength, j, 0);
        getOptional<int>("rating", p.rating, j, 0);
    }


    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(GroupAlias)
    /** 
     * @brief Group Alias used as part of the @ref PresenceDescriptor 
     * 
     *  Helper C++ class to serialize and de-serialize GroupAlias JSON 
     * 
     * Example JSON: @include[lineno] examples/GroupAlias.json 
     * 
     *  @see PresenceDescriptor  
     */                
    class GroupAlias : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(GroupAlias)

    public:
        /** @brief Group Id the alias is associated with. */
        std::string     groupId;

        /** @brief Users alias for the group. */
        std::string     alias;

        GroupAlias()
        {
            clear();
        }

        void clear()
        {
            groupId.clear();
            alias.clear();
        }

        virtual void initForDocumenting()
        {            
        }
    };
    
    static void to_json(nlohmann::json& j, const GroupAlias& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(groupId),
            TOJSON_IMPL(alias)
        };
    }
    static void from_json(const nlohmann::json& j, GroupAlias& p)
    {
        p.clear();
        j.at("groupId").get_to(p.groupId);
        j.at("alias").get_to(p.alias);
    }    


    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(PresenceDescriptor)
    /** 
     * @brief Represents an endpoints presence properties. Used in @ref engageUpdatePresenceDescriptor API and PFN_ENGAGE_GROUP_NODE events.  
     *  
     *  Helper C++ class to serialize and de-serialize PresenceDescriptor JSON
     * 
     *  Example: @include[doc] examples/PresenceDescriptor.json 
     * 
     *  @see engageUpdatePresenceDescriptor, PFN_ENGAGE_GROUP_NODE_DISCOVERED, PFN_ENGAGE_GROUP_NODE_REDISCOVERED
     */    
    class PresenceDescriptor : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(PresenceDescriptor)
        
    public:

        /** 
         * @brief [Read Only] Indicates that this presence declaration was generated by the Engage Engine the application currently connected to. 
         *
         * NOTE: This is useful in identifying the nodeId that is automatically generated by the Engage Engine when the application does not provide the nodeId.  
         */      
        bool                        self;

        /** 
         * @brief [Read Only, Unix timestamp - Zulu/UTC] Indicates the timestamp that the message was originally sent. 
         *   
         * Use this property and the <b>@ref nextUpdate</b> attribute to determine if the endpoint is still on the network. 
         */      
        uint32_t                    ts; 

        /** 
         * @brief [Read Only, Unix timestamp - Zulu/UTC] Indicates the next time the presence descriptor will be sent. 
         *   
         *   This attributed together with the <b>@ref ts</b> attribute is used by the Engage Engage Engine to "timeout" the endpoint. (Unix timestamp - Zulu/UTC) 
         */      
        uint32_t                    nextUpdate;

        /** @brief [Optional, Default see @ref Identity] Endpoint's identity information. */      
        Identity                    identity;

        /** @brief [Optional] TODO: Shaun, whats the max length. */      
        std::string                 comment;

        /** 
         * @brief [Optional] Indicates the users disposition
         * 
         * This may be any value set bu the application and here is an example of values to use
         * 
         * Value | Description 
         * --- | ---  
         *  0x00000000 | Undefined 
         *  0x00000001 | Emergency 
         *  0x00000002 | Available 
         *  0x00000004 | Busy 
         *  etc | etc 
         */
        uint32_t                    disposition;         

        /** @brief [Read Only] List of group aliases associated with this presence descriptor. */
        std::vector<GroupAlias>     groupAliases;

        /** @brief [Optional, Default: see @ref Location] Location information @see Location for more info. */
        Location                    location;

        /** @brief [Optional, Default: empty string] Custom string application can use of presence descriptor. TODO: Whats the max length. */
        std::string                 custom;

        /** @brief [Read Only] Indicates that the Engine will announce its PresenceDescriptor in response to this message. */
        bool                        announceOnReceive;

        /** @brief [Optional, Default: see @ref Connectivity] Device connectivity information like wifi/cellular, strength etc. */
        Connectivity                connectivity;

        /** @brief [Optional, Default: see @ref Power] Device power information like charging state, battery level, etc. */
        Power                       power;

        PresenceDescriptor()
        {
            clear();
        }

        void clear()
        {
            self = false;
            ts = 0;
            nextUpdate = 0;
            identity.clear();
            comment.clear();
            disposition = 0;
            groupAliases.clear();
            location.clear();
            custom.clear();
            announceOnReceive = false;
            connectivity.clear();
            power.clear();
        }

        virtual void initForDocumenting()
        {
            clear();

            self = true;
            ts = 123;
            nextUpdate = 0;
            identity.initForDocumenting();
            comment = "This is a comment";
            disposition = 123;

            GroupAlias ga;
            ga.initForDocumenting();
            groupAliases.push_back(ga);

            location.initForDocumenting();
            custom = "{}";
            announceOnReceive = true;
            connectivity.initForDocumenting();
            power.initForDocumenting();
        }
    };

    static void to_json(nlohmann::json& j, const PresenceDescriptor& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(ts),
            TOJSON_IMPL(nextUpdate),
            TOJSON_IMPL(identity),
            TOJSON_IMPL(comment),
            TOJSON_IMPL(disposition),
            TOJSON_IMPL(groupAliases),
            TOJSON_IMPL(location),
            TOJSON_IMPL(custom),
            TOJSON_IMPL(announceOnReceive),
            TOJSON_IMPL(connectivity),
            TOJSON_IMPL(power)
        };

        if(p.self)
        {
            j["self"] = true;
        }
    }
    static void from_json(const nlohmann::json& j, PresenceDescriptor& p)
    {
        p.clear();
        getOptional<bool>("self", p.self, j);
        getOptional<uint32_t>("ts", p.ts, j);
        getOptional<uint32_t>("nextUpdate", p.nextUpdate, j);
        getOptional<ConfigurationObjects::Identity>("identity", p.identity, j);
        getOptional<std::string>("comment", p.comment, j);
        getOptional<uint32_t>("disposition", p.disposition, j);
        getOptional<std::vector<GroupAlias>>("groupAliases", p.groupAliases, j);
        getOptional<Location>("location", p.location, j);
        getOptional<std::string>("custom", p.custom, j);
        getOptional<bool>("announceOnReceive", p.announceOnReceive, j);
        getOptional<Connectivity>("connectivity", p.connectivity, j);
        getOptional<Power>("power", p.power, j);
    }
    
    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(NetworkTxOptions) 
    /** 
     * @brief Network Transmit Options used in @ref Group
     * 
     *  Helper C++ class to serialize and de-serialize NetworkTxOptions JSON
     * 
     *  TODO: Complete this Class
     * 
     *  Example: @include[doc] examples/NetworkTxOptions.json 
     *    
     *  @see Group
     */    
    class NetworkTxOptions : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(NetworkTxOptions)
        
    public:

        /** 
         * @brief Network Transmission Priority.
         *   
         *   Priority to set on the packets in order for the network to optimize routing of the packets 
         */
        typedef enum
        {
            /** @brief NET_SERVICE_TYPE_BE */
            priBestEffort   = 0, 


            /** @brief NET_SERVICE_TYPE_SIG */
            priSignaling    = 2,

            /** @brief NET_SERVICE_TYPE_VI */
            priVideo        = 3,

            /** @brief NET_SERVICE_TYPE_VO */
            priVoice        = 4
        } TxPriority_t;

        /** @brief [Optional, Default: @ref priVoice] Transmission priority at which to tag the packets with.  */
        TxPriority_t    priority;

        /** 
         * @brief [Optional, Default: 128] Time to live or hop limit is a mechanism that limits the lifespan or lifetime of data in a network. TTL prevents a data packet from circulating indefinitely. 
         *   
         *   E.g If you don't want multicast data to leave your local network, set the TTL to 1. 
         */
        int             ttl;   

        NetworkTxOptions()
        {
            clear();
        }

        void clear()
        {
            priority = priVoice;
            ttl = 128;
        }

        virtual void initForDocumenting()
        {            
        }
    };
    
    static void to_json(nlohmann::json& j, const NetworkTxOptions& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(priority),
            TOJSON_IMPL(ttl)
        };
    }
    static void from_json(const nlohmann::json& j, NetworkTxOptions& p)
    {
        p.clear();
        getOptional<NetworkTxOptions::TxPriority_t>("priority", p.priority, j, NetworkTxOptions::priVoice);
        getOptional<int>("ttl", p.ttl, j, 128);
    }

    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(NetworkAddress) 
    /** 
     * @brief NetworkAddress 
     * 
     * Helper C++ class to serialize and de-serialize NetworkAddress JSON
     * 
     * TODO: Complete this Class
     * 
     * Example: @include[doc] examples/NetworkAddress.json 
     *    
     */    
    class NetworkAddress : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(NetworkAddress)
        
    public:
        /** @brief IP address. */
        std::string     address;

        /** @brief Port. */
        int             port;

        NetworkAddress()
        {
            clear();
        }

        void clear()
        {
            address.clear();
            port = 0;
        }
    };
    
    static void to_json(nlohmann::json& j, const NetworkAddress& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(address),
            TOJSON_IMPL(port)
        };
    }
    static void from_json(const nlohmann::json& j, NetworkAddress& p)
    {
        p.clear();
        getOptional<std::string>("address", p.address, j);
        getOptional<int>("port", p.port, j);
    }


    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(NetworkAddressRxTx) 
    /** 
     * @brief NetworkAddressRxTx 
     * 
     * Helper C++ class to serialize and de-serialize NetworkAddressRxTx JSON
     * 
     * TODO: Complete this Class
     * 
     * Example: @include[doc] examples/NetworkAddressRxTx.json 
     *    
     */    
    class NetworkAddressRxTx : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(NetworkAddressRxTx)
        
    public:
        /** @brief RX. */
        NetworkAddress          rx;

        /** @brief TX. */
        NetworkAddress          tx;

        NetworkAddressRxTx()
        {
            clear();
        }

        void clear()
        {
            rx.clear();
            tx.clear();
        }
    };
    
    static void to_json(nlohmann::json& j, const NetworkAddressRxTx& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(rx),
            TOJSON_IMPL(tx)
        };
    }
    static void from_json(const nlohmann::json& j, NetworkAddressRxTx& p)
    {
        p.clear();
        getOptional<NetworkAddress>("rx", p.rx, j);
        getOptional<NetworkAddress>("tx", p.tx, j);
    }

    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(Rallypoint)
    /** 
     * @brief Rallypoint
     * 
     * Helper C++ class to serialize and de-serialize Rallypoint JSON
     *       
     * Example: @include[doc] examples/Rallypoint.json 
     */    
    class Rallypoint : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(Rallypoint)
        
    public:

        /**
         * @brief This is the host address for the Engine to connect to the RallyPoint service.  
         * 
         * TODO: Shaun, Is there anything else we have to explain here 
         * 
         */    
        NetworkAddress              host;

        /**
         * @brief This is the X509 certificate to use for mutual authentication.    
         * 
         * The full contents of the cert can be specified or the file path to where the cert is located can be specified by using an @@ replacement string e.g
         * 
         *  \code{.json}
             certificate="@c:\\privatestore\certs\rp_cert.pem"
         *  \endcode
         *  
         *  
         * TODO: Shaun, Is there anything else we have to explain here 
         * 
         */    
        std::string                 certificate;

        /**
         * @brief This is the private key used to generate the X509 certificate.    
         * 
         * The full contents of the key can be specified or the file path to where the key is located can be specified by using an @@ replacement string e.g
         * 
         *  \code{.json}
             certificate="@c:\\privatestore\certs\rp_private_key.key"
         *  \endcode
         *  
         *  
         * TODO: Shaun, Is there anything else we have to explain here 
         * 
         */          
        std::string                 certificateKey; 

        /** 
         * @brief [Optional] TODO: Shaun, whats this? This looks like its for RallyPoint config and not client?
         * 
         * Description 
         */
        bool                        verifyPeer;

        /** 
         * @brief [Optional] TODO: Shaun, whats this? This looks like its for RallyPoint config and not client?
         * 
         * Description 
         */
        bool                        allowSelfSignedCertificate;    

        /** 
         * @brief [Optional] TODO: Shaun, whats this? This looks like its for RallyPoint config and not client?
         * 
         * Description 
         */
        std::vector<std::string>    caCertificates; 

        /** 
         * @brief [Optional] TODO: Shaun, whats this? This looks like its for RallyPoint config and not client?
         * 
         * Description 
         */
        int                         transactionTimeoutMs;
        bool                        disableMessageSigning;


        Rallypoint()
        {
            clear();
        }

        void clear()
        {
            host.clear();
            certificate.clear();
            certificateKey.clear();
            caCertificates.clear();
            verifyPeer = false;
            transactionTimeoutMs = 5000;
            disableMessageSigning = false;
        }
    };
    
    static void to_json(nlohmann::json& j, const Rallypoint& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(host),
            TOJSON_IMPL(certificate),
            TOJSON_IMPL(certificateKey),                        
            TOJSON_IMPL(verifyPeer),
            TOJSON_IMPL(allowSelfSignedCertificate),
            TOJSON_IMPL(caCertificates),
            TOJSON_IMPL(transactionTimeoutMs),
            TOJSON_IMPL(disableMessageSigning)
        };
    }
    static void from_json(const nlohmann::json& j, Rallypoint& p)
    {
        p.clear();
        j.at("host").get_to(p.host);
        getOptional("certificate", p.certificate, j);
        getOptional("certificateKey", p.certificateKey, j);
        getOptional<bool>("verifyPeer", p.verifyPeer, j, false);
        getOptional<bool>("allowSelfSignedCertificate", p.allowSelfSignedCertificate, j, true);
        getOptional<std::vector<std::string>>("caCertificates", p.caCertificates, j);
        getOptional<int>("transactionTimeoutMs", p.transactionTimeoutMs, j, 5000);

        if(!p.certificate.empty() && p.certificate.c_str()[0] == '@')
        {
            if(!readTextFileIntoString(p.certificate.c_str() + 1, p.certificate))
            {
                p.certificate.clear();
            }
        }

        if(!p.certificateKey.empty() && p.certificateKey.c_str()[0] == '@')
        {
            if(!readTextFileIntoString(p.certificateKey.c_str() + 1, p.certificateKey))
            {
                p.certificateKey.clear();
            }
        }

        getOptional<bool>("disableMessageSigning", p.disableMessageSigning, j, false);
    }

    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(TxAudio)
    /**
     * @brief Configuration for the audio transmit properties for a group 
     *  
     *  Helper C++ class to serialize and de-serialize TxAudio JSON  @see Group
     *      
     *  This JSON is used as an object in the Group definition JSON. 
     * 
     *  <P> 
     *  Example: @include[doc] examples/TxAudio.json 
     *  </P>    
     *    
     */    
    class TxAudio : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(TxAudio)
        
    public:

        /** 
         * @brief Codec Types enum.
         * 
         * More detailed TxCodec_t description. 
         */
        typedef enum
        {

            /** @brief Unknown Codec type */
            ctUnknown       = 0,


            /** @brief G711 U-Law 64 (kbit/s) <a href="https://en.wikipedia.org/wiki/G.711" target="_blank">See for more info</a> */
            ctG711ulaw      = 1, 

            /** @brief G711 A-Law 64 (kbit/s) <a href="https://en.wikipedia.org/wiki/G.711" target="_blank">See for more info</a> */
            ctG711alaw      = 2,    /**<  */


            /** @brief GSM Full Rate 13.2 (kbit/s) <a href="https://en.wikipedia.org/wiki/Full_Rate" target="_blank">See for more info</a> */
            ctGsm610        = 3,

            /** @brief AMR Narrowband 4.75 (kbit/s) <a href="https://en.wikipedia.org/wiki/Adaptive_Multi-Rate_audio_codec" target="_blank">See for more info</a> */
            ctAmrNb4750     = 10,

            /** @brief AMR Narrowband 5.15 (kbit/s) <a href="https://en.wikipedia.org/wiki/Adaptive_Multi-Rate_audio_codec" target="_blank">See for more info</a> */
            ctAmrNb5150     = 11,

            /** @brief AMR Narrowband 5.9 (kbit/s) <a href="https://en.wikipedia.org/wiki/Adaptive_Multi-Rate_audio_codec" target="_blank">See for more info</a> */
            ctAmrNb5900     = 12, 

            /** @brief AMR Narrowband 6.7 (kbit/s) <a href="https://en.wikipedia.org/wiki/Adaptive_Multi-Rate_audio_codec" target="_blank">See for more info</a> */
            ctAmrNb6700     = 13, 

            /** @brief AMR Narrowband 7.4 (kbit/s) <a href="https://en.wikipedia.org/wiki/Adaptive_Multi-Rate_audio_codec" target="_blank">See for more info</a> */
            ctAmrNb7400     = 14,

            /** @brief AMR Narrowband 7.95 (kbit/s) <a href="https://en.wikipedia.org/wiki/Adaptive_Multi-Rate_audio_codec" target="_blank">See for more info</a> */
            ctAmrNb7950     = 15,

            /** @brief AMR Narrowband 10.2 (kbit/s) <a href="https://en.wikipedia.org/wiki/Adaptive_Multi-Rate_audio_codec" target="_blank">See for more info</a> */
            ctAmrNb10200    = 16,

            /** @brief AMR Narrowband 12.2 (kbit/s) <a href="https://en.wikipedia.org/wiki/Adaptive_Multi-Rate_audio_codec" target="_blank">See for more info</a> */
            ctAmrNb12200    = 17,


            /** @brief Opus 6 (kbit/s) <a href="http://opus-codec.org" target="_blank">See for more info</a> */
            ctOpus6000      = 20,

            /** @brief Opus 8 (kbit/s) <a href="http://opus-codec.org" target="_blank">See for more info</a> */
            ctOpus8000      = 21,

            /** @brief Opus 10 (kbit/s) <a href="http://opus-codec.org" target="_blank">See for more info</a> */
            ctOpus10000     = 22,

            /** @brief Opus 12 (kbit/s) <a href="http://opus-codec.org" target="_blank">See for more info</a> */
            ctOpus12000     = 23,

            /** @brief Opus 14 (kbit/s) <a href="http://opus-codec.org" target="_blank">See for more info</a> */
            ctOpus14000     = 24,

            /** @brief Opus 16 (kbit/s) <a href="http://opus-codec.org" target="_blank">See for more info</a> */
            ctOpus16000     = 25,

            /** @brief Opus 18 (kbit/s) <a href="http://opus-codec.org" target="_blank">See for more info</a> */
            ctOpus18000     = 26,

            /** @brief Opus 20 (kbit/s) <a href="http://opus-codec.org" target="_blank">See for more info</a> */
            ctOpus20000     = 27,

            /** @brief Opus 22 (kbit/s) <a href="http://opus-codec.org" target="_blank">See for more info</a> */
            ctOpus22000     = 28,

            /** @brief Opus 24 (kbit/s) <a href="http://opus-codec.org" target="_blank">See for more info</a> */
            ctOpus24000     = 29 
        } TxCodec_t;

        /** @brief [Optional, Default: @ref ctOpus8000] Specifies the Codec Type to use for the transmission. See @ref TxCodec_t for all codec types */
        TxCodec_t       encoder;

        /** @brief [Optional, Default: 60] Audio sample framing size in milliseconds. TODO: Shaun, can you please elaborate on this?  */
        int             framingMs;

        /** @brief [Optional, Default: false] Indicates if full duplex audio is supported.  TODO: Shaun, can you please elaborate on this? */
        bool            fdx;

        /** 
         * @brief [Optional, Default: false] Set to <b>true</b> whether to disable header extensions.
         *
         * Indicates whether the Engine should not add a header extension on the RTP packet. If the header extension is omitted then any of the information such as Alias, NodeID and Tx 
         * Priority will <B>not</B> be transmitted along with the audio.
         */   
        bool            noHdrExt; 
        
        /** 
         * @brief [Optional, Default: 0] Maximum number of seconds the Engine will transmit for.
         *
         * When the time limit is exceeded, the Engine will fire a PFN_ENGAGE_GROUP_MAX_TX_TIME_EXCEEDED event.
         */   
        int             maxTxSecs;
        
        /** 
         * @brief [Optional, Default: 10] The number pf packets when to periodically send the header extension.
         *
         * Eg, if its 3, then every third packed will contain the header extension. TODO: SHAUN please verify
         */   
        int             extensionSendInterval;  

        /** @brief [Optional, Default: false] TODO: SHAUN help. */
        bool            debug;

        /** @brief [Optional, Default: 0] A priority between 0 and 255.   */
        int             userTxPriority;

        /** @brief [Optional, Default: 0] TODO: SHAUN help. */
        int             userTxFlags;

        /** @brief [Optional, Default: 5] TODO: SHAUN help. */
        int             initialHeaderBurst; 

        /** @brief [Optional, Default: 5] TODO: SHAUN help. */
        int             trailingHeaderBurst;


        TxAudio()
        {
            clear();
        }

        void clear()
        {
            encoder = ctUnknown;
            framingMs = 60;
            fdx = false;
            noHdrExt = false;
            maxTxSecs = 0;
            extensionSendInterval = 10;
            debug = false;
            userTxPriority = 0;
            userTxFlags = 0;
            initialHeaderBurst = 5;
            trailingHeaderBurst = 5;
        }
    };
    
    static void to_json(nlohmann::json& j, const TxAudio& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(encoder),
            TOJSON_IMPL(framingMs),
            TOJSON_IMPL(fdx),
            TOJSON_IMPL(noHdrExt),
            TOJSON_IMPL(maxTxSecs),
            TOJSON_IMPL(extensionSendInterval),
            TOJSON_IMPL(debug),
            TOJSON_IMPL(userTxPriority),
            TOJSON_IMPL(userTxFlags),
            TOJSON_IMPL(initialHeaderBurst),
            TOJSON_IMPL(trailingHeaderBurst)
        };
    }
    static void from_json(const nlohmann::json& j, TxAudio& p)
    {
        p.clear();
        getOptional<TxAudio::TxCodec_t>("encoder", p.encoder, j, TxAudio::TxCodec_t::ctOpus8000);
        getOptional("framingMs", p.framingMs, j, 60);
        getOptional("fdx", p.fdx, j, false);
        getOptional("noHdrExt", p.noHdrExt, j, false);
        getOptional("maxTxSecs", p.maxTxSecs, j, 0);
        getOptional("debug", p.debug, j, false);
        getOptional("userTxPriority", p.userTxPriority, j, 0);
        getOptional("userTxFlags", p.userTxFlags, j, 0);
        getOptional("extensionSendInterval", p.extensionSendInterval, j, 10);
        getOptional("initialHeaderBurst", p.initialHeaderBurst, j, 5);
        getOptional("trailingHeaderBurst", p.trailingHeaderBurst, j, 5);
    }


    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(AudioDeviceDescriptor)
    /**
     * @brief Custom Audio Device Configuration  
     * 
     * Helper C++ class to serialize and de-serialize AudioDeviceDescriptor JSON used in @ref engageAudioDeviceRegister API.  
     *      
     * TODO: Shaun, I know very little about these settings, can you please verify and complete 
     * 
     * Example: @include[doc] examples/AudioDeviceDescriptor.json 
     * 
     * @see engageAudioDeviceRegister
     */           
    class AudioDeviceDescriptor : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(AudioDeviceDescriptor)
        
    public:

        /** @brief Audio Device Direction Enum. */    
        typedef enum 
        {
            /** @brief Direction unknown */
            dirUnknown = 0,

            /** @brief This is an input only device */
            dirInput,

            /** @brief This is an output only device */
            dirOutput,

            /** @brief This device supports both input and output */
            dirBoth
        } Direction_t;

        /** 
         * @brief [Read Only] Unique device identifier assigned by Engage Engine at time of device creation. 
         * 
         * TODO: SHAUN please verify 
         */
        int             deviceId;

        /** 
         * @brief This is the rate that the device will process the PCM audio data at. 
         * 
         * TODO: Shaun, do we need to add a note about the number of bytes etc based on Kamil experience 
         */ 
        int             samplingRate;       ///< TODO: Shaun help

        /** 
         * @brief Indicates the number of audio channels to process. 

         * TODO: Shaun. How many channels can we support and whats the default
         */  
        int             channels;           ///< TODO: Shaun help

        /** @brief Audio direction the device supports @see Direction_t */
        Direction_t     direction;

        /** 
         * @brief Is a percentage at which to gain the audio. 

         * TODO: Shaun help
         */
        int             boostPercentage;  
        int             msPerBuffer;

        bool            isAdad;
        std::string     name;
        std::string     manufacturer;
        std::string     model;
        std::string     hardwareId;
        std::string     serialNumber;
        bool            isDefault;
        std::string     type;
        std::string     extra;
        
        AudioDeviceDescriptor()
        {
            clear();
        }

        void clear()
        {
            deviceId = 0;
            samplingRate = 0;
            channels = 0;
            direction = dirUnknown;
            boostPercentage = 0;
            isAdad = false;
            isDefault = false;
            msPerBuffer = 0;

            name.clear();
            manufacturer.clear();
            model.clear();
            hardwareId.clear();
            serialNumber.clear();
            type.clear();
            extra.clear();
        }
    };
    
    static void to_json(nlohmann::json& j, const AudioDeviceDescriptor& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(deviceId),
            TOJSON_IMPL(samplingRate),
            TOJSON_IMPL(msPerBuffer),
            TOJSON_IMPL(channels),
            TOJSON_IMPL(direction),
            TOJSON_IMPL(boostPercentage),
            TOJSON_IMPL(isAdad),
            TOJSON_IMPL(name),
            TOJSON_IMPL(manufacturer),
            TOJSON_IMPL(model),
            TOJSON_IMPL(hardwareId),
            TOJSON_IMPL(serialNumber),
            TOJSON_IMPL(isDefault),
            TOJSON_IMPL(type),
            TOJSON_IMPL(extra)
        };
    }
    static void from_json(const nlohmann::json& j, AudioDeviceDescriptor& p)
    {
        p.clear();
        getOptional<int>("deviceId", p.deviceId, j, 0);
        getOptional<int>("samplingRate", p.samplingRate, j, 0);
        getOptional<int>("msPerBuffer", p.msPerBuffer, j, 0);
        getOptional<int>("channels", p.channels, j, 0);
        getOptional<AudioDeviceDescriptor::Direction_t>("direction", p.direction, j, 
                        AudioDeviceDescriptor::Direction_t::dirUnknown);
        getOptional<int>("boostPercentage", p.boostPercentage, j, 0);

        getOptional<bool>("isAdad", p.isAdad, j, false);
        getOptional("name", p.name, j);
        getOptional("manufacturer", p.manufacturer, j);
        getOptional("model", p.model, j);
        getOptional("hardwareId", p.hardwareId, j);
        getOptional("serialNumber", p.serialNumber, j);
        getOptional("isDefault", p.isDefault, j);
        getOptional("type", p.type, j);
        getOptional("extra", p.extra, j);
    }

    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(ListOfAudioDeviceDescriptor)
    class ListOfAudioDeviceDescriptor : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(ListOfAudioDeviceDescriptor)
        
    public:
        std::vector<AudioDeviceDescriptor> list;

        ListOfAudioDeviceDescriptor()
        {
            clear();
        }

        void clear()
        {
            list.clear();
        }
    };
    
    static void to_json(nlohmann::json& j, const ListOfAudioDeviceDescriptor& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(list)
        };
    }
    static void from_json(const nlohmann::json& j, ListOfAudioDeviceDescriptor& p)
    {
        p.clear();
        getOptional<std::vector<AudioDeviceDescriptor>>("list", p.list, j);
    }

    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(Audio)
    /**
     * @brief Used to configure the Audio properties for a group @see Group  
     * 
     * Helper C++ class to serialize and de-serialize Audio JSON 
     *      
     * Example: @include[doc] examples/Audio.json 
     */           
    class Audio : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(Audio)
        
    public:

        /** @brief [Optional, Default: first audio device] Id for the input audio device to use for this group. TODO: Shaun, please verify */
        int     inputId;

        /** @brief [Optional, Default: 0] The percentage at which to gain the input audio. TODO: Shaun, what is the range? */
        int     inputGain;

        /** @brief [Optional, Default: first audio device] Id for the output audio device to use for this group. TODO: Shaun, please verify. */
        int     outputId;

        /** @brief [Optional, Default: 0] The percentage at which to gain the output audio. TODO: Shaun, what is the range? */
        int     outputGain;

        /** @brief [Optional, Default: 100] The percentage at which to set the <b>left</b> audio at.  */
        int     outputLevelLeft;

        /** @brief [Optional, Default: 100] The percentage at which to set the <b>right</b> audio at.  */
        int     outputLevelRight;

        /** @brief [Optional, Default: false] Mutes output audio.  */
        bool    outputMuted;

        Audio()
        {
            clear();
        }

        void clear()
        {
            inputId = 0;
            inputGain = 0;
            outputId = 0;
            outputGain = 0;
            outputLevelLeft = 100;
            outputLevelRight = 100;
            outputMuted = false;
        }
    };
    
    static void to_json(nlohmann::json& j, const Audio& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(inputId),
            TOJSON_IMPL(inputGain),
            TOJSON_IMPL(outputId),
            TOJSON_IMPL(outputLevelLeft),
            TOJSON_IMPL(outputLevelRight),
            TOJSON_IMPL(outputMuted)
        };
    }
    static void from_json(const nlohmann::json& j, Audio& p)
    {
        p.clear();
        getOptional<int>("inputId", p.inputId, j, 0);
        getOptional<int>("inputGain", p.inputGain, j, 0);
        getOptional<int>("outputId", p.outputId, j, 0);
        getOptional<int>("outputGain", p.outputGain, j, 0);
        getOptional<int>("outputLevelLeft", p.outputLevelLeft, j, 100);
        getOptional<int>("outputLevelRight", p.outputLevelRight, j, 100);
        getOptional<bool>("outputMuted", p.outputMuted, j, false);
    }
    
    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(TalkerInformation)
    /**
     * @brief Contains talker information used in providing a list in GroupTalkers  
     * 
     * Helper C++ class to serialize and de-serialize TalkerInformation JSON 
     * 
     * Example: @include[doc] examples/TalkerInformation.json 
     *    
     * @see GroupTalkers 
     */   
    class TalkerInformation : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(TalkerInformation)
        
    public:

        /** @brief The user alias to represent as a "talker". */ 
        std::string alias;

        /** @brief The nodeId the talker is originating from. */ 
        std::string nodeId;

        TalkerInformation()
        {
            clear();
        }

        void clear()
        {
            alias.clear();
            nodeId.clear();
        }
    };
    
    static void to_json(nlohmann::json& j, const TalkerInformation& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(alias),
            TOJSON_IMPL(nodeId)
        };
    }
    static void from_json(const nlohmann::json& j, TalkerInformation& p)
    {
        p.clear();
        getOptional<std::string>("alias", p.alias, j, EMPTY_STRING);
        getOptional<std::string>("nodeId", p.nodeId, j, EMPTY_STRING);
    }

    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(GroupTalkers)
    /**
     * @brief List of @ref TalkerInformation objects.  
     * 
     * Helper C++ class to serialize and de-serialize GroupTalkers JSON 
     *      
     * This is used as the groupTalkerJson parameter in the PFN_ENGAGE_GROUP_RX_SPEAKERS_CHANGED event to represent a list of active talkers on the group. 
     * 
     * Example: @include[doc] examples/GroupTalkers.json 
     *    
     * @see PFN_ENGAGE_GROUP_RX_SPEAKERS_CHANGED
     */   
    class GroupTalkers : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(GroupTalkers)
        
    public:
        /** @brief List of @ref TalkerInformation objects.*/
        std::vector<TalkerInformation> list;

        GroupTalkers()
        {
            clear();
        }

        void clear()
        {
            list.clear();
        }
    };
    
    static void to_json(nlohmann::json& j, const GroupTalkers& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(list)
        };
    }
    static void from_json(const nlohmann::json& j, GroupTalkers& p)
    {
        p.clear();
        getOptional<std::vector<TalkerInformation>>("list", p.list, j);
    }
    
    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(Presence)
    /**
     * @brief Describes how the Presence is configured for a group of type @ref Group::gtPresence in @ref Group::Type_t
     * 
     * Helper C++ class to serialize and de-serialize Presence JSON 
     *           
     * Example: @include[doc] examples/Presence.json 
     *    
     * @see Group
     */   
    class Presence : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(Presence)
        
    public:
        /**
         * @brief Presence format types enum.
         */
        typedef enum
        {
            /** @brief Unknown */
            pfUnknown       = 0,

            /** @brief Engage propriety format */
            pfEngage        = 1,

            /** 
             * @brief Cursor On Target format. 
             * 
             * This is a simple XML based exchange standard that is used to share information about targets. 
             * Cursor on Target was originally developed by MITRE in 2002 in support of the U.S. Air Force Electronic Systems Center (ESC) 
             */
            pfCot           = 2
        } Format_t;

        /** @brief Format to be used to represent presence information. */
        Format_t        format;

        /** @brief [Optional, Default: 30] The interval in seconds at which to send the presence descriptor on the presence group */
        int             intervalSecs;

        /** @brief [Optional, Default: false] Will force the presence to be transmitted every time audio is transmitted. */ 
        bool            forceOnAudioTransmit;

        /** @brief Instructs the Engage Engine to not transmit presence descriptor */ 
        bool            listenOnly;

        Presence()
        {
            clear();
        }

        void clear()
        {
            format = pfUnknown;
            intervalSecs = 30;
            forceOnAudioTransmit = false;
            listenOnly = false;
        }
    };
    
    static void to_json(nlohmann::json& j, const Presence& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(format),
            TOJSON_IMPL(intervalSecs),
            TOJSON_IMPL(forceOnAudioTransmit),
            TOJSON_IMPL(listenOnly)
        };
    }
    static void from_json(const nlohmann::json& j, Presence& p)
    {
        p.clear();
        getOptional<Presence::Format_t>("format", p.format, j, Presence::Format_t::pfEngage);
        getOptional<int>("intervalSecs", p.intervalSecs, j, 60);
        getOptional<bool>("forceOnAudioTransmit", p.forceOnAudioTransmit, j, false);
        getOptional<bool>("listenOnly", p.listenOnly, j, false);
    }


    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(Advertising)
    /** 
     * @brief TODO: Shaun, not sure what this does, please verify all info.  Presnce
     * 
     * Helper C++ class to serialize and de-serialize Advertising JSON 
     * 
     * Example: @include[doc] examples/Advertising.json 
     *    
     * @see DiscoverySsdp, DiscoverySap
     */       
    class Advertising : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(Advertising)
        
    public:
        /** @brief [Optional, Default: false] Enabled advertising */
        bool        enabled;

        /** @brief [Optional, Default: 20000] Interval at which the advertisement should be sent in milliseconds. */
        int         intervalMs;

        /** @brief [Optional, Default: false] TODO: Shaun please help */
        bool        alwaysAdvertise;

        Advertising()
        {
            clear();
        }

        void clear()
        {
            enabled = false;
            intervalMs = 20000;
            alwaysAdvertise = false;
        }
    };

    static void to_json(nlohmann::json& j, const Advertising& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(enabled),
            TOJSON_IMPL(intervalMs),
            TOJSON_IMPL(alwaysAdvertise)
        };
    }
    static void from_json(const nlohmann::json& j, Advertising& p)
    {
        p.clear();
        getOptional("enabled", p.enabled, j, false);
        getOptional<int>("intervalMs", p.intervalMs, j, 20000);
        getOptional<bool>("alwaysAdvertise", p.alwaysAdvertise, j, false);
    }

    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(GroupTimeline)
    /**
     * @brief Configuration for Timeline functionality for Group. 
     * 
     * If a GroupTimeline is not specified, then the configursation in the @ref ConfigurationObjects::EnginePolicyTimelines will used as default.
     * 
     * Helper C++ class to serialize and de-serialize GroupTimeline JSON 
     * 
     * Example: @include[doc] examples/GroupTimeline.json 
     *    
     * @see ConfigurationObjects::Group, ConfigurationObjects::EnginePolicyTimeLines 
     */       
    class GroupTimeline : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(GroupTimeline)
        
    public:
        /** @brief [Optional, Default: true] Enables timeline feature. */
        bool        enabled;

        /** @brief [Optional, Default: 30000] Maximum audio block size to record in milliseconds. */
        int         maxAudioTimeMs;
        bool        recordAudio;

        GroupTimeline()
        {
            clear();
        }

        void clear()
        {
            enabled = true;
            maxAudioTimeMs = 30000;
            recordAudio = true;
        }
    };

    static void to_json(nlohmann::json& j, const GroupTimeline& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(enabled),
            TOJSON_IMPL(maxAudioTimeMs),
            TOJSON_IMPL(recordAudio)
        };
    }
    static void from_json(const nlohmann::json& j, GroupTimeline& p)
    {
        p.clear();
        getOptional("enabled", p.enabled, j, true);
        getOptional<int>("maxAudioTimeMs", p.maxAudioTimeMs, j, 30000);
        getOptional("recordAudio", p.recordAudio, j, true);
    }

    //-----------------------------------------------------------
    ENGAGE_IGNORE_COMPILER_UNUSED_WARNING static const char *GROUP_SOURCE_ENGAGE_INTERNAL = "com.rallytac.engage.internal";
    ENGAGE_IGNORE_COMPILER_UNUSED_WARNING static const char *GROUP_SOURCE_ENGAGE_MAGELLAN_CISTECH = "com.rallytac.engage.magellan.cistech" ;
    ENGAGE_IGNORE_COMPILER_UNUSED_WARNING static const char *GROUP_SOURCE_ENGAGE_MAGELLAN_TRELLISWARE = "com.rallytac.engage.magellan.trellisware";

    JSON_SERIALIZED_CLASS(Group)
    /** 
     * @brief Group Configuration
     * 
     * This describes all the group properties for all supported group types
     *
     * Example: @include[doc] examples/Group.json    
     * 
     * @see engageCreateGroup 
     *     
     */        
    class Group : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(Group)
        
    public:

        /** @brief Enum describing all the different group types. */
        typedef enum 
        {   

            /** @brief Unknown group type */
            gtUnknown, 

            /** @brief Audio group type. This group is used to transmit Audio. */
            gtAudio, 

            /** @brief Presence group type. This group is use to relay presence data to all nodes that are configured for the same presence group. */
            gtPresence,

            /** @brief Raw group type. TODO: Shaun, can you elaborate  */
            gtRaw
        } Type_t;

        /** @brief Specifies the group type (see @ref Type_t). */
        Type_t                                  type;

        /**
         * @brief Unique identity for the group.
         * 
         * NOTE: Groups configured with the same multicast addresses but with different 
         * id's will NOT be routed correctly via RallyPoints as they are considered different streams.  
         */
        std::string                             id;

        /** @brief The human readable name for the group. */
        std::string                             name;

        /** @brief TODO: Shaun, what is this? */
        std::string                             interfaceName;

        /** @brief The network address for receiving network traffic on. */
        NetworkAddress                          rx;

        /** @brief The network address for transmitting network traffic to. */
        NetworkAddress                          tx;

        /** @brief Transmit options fro the group (see @ref NetworkTxOptions). */
        NetworkTxOptions                        txOptions;

        /** @brief Audio transmit options such as codec, framing size etc (see @ref TxAudio). */
        TxAudio                                 txAudio;

        /** @brief Presence configuration (see @ref Presence). */
        Presence                                presence;

        /** @brief Password to be used for encryption. Note that this is not the encryption key. TODO: Shaun, can you please elaborate */
        std::string                             cryptoPassword;

        /** @brief TODO: Shaun, no clue what this does */
        bool                                    debugAudio;

        /** @brief List of @ref Rallypoint (s) the Group should use to connect to a RallyPoint router. */
        std::vector<Rallypoint>                 rallypoints;

        /** @brief Sets audio properties like which audio device to use, audio gain etc (see @ref Audio). */
        Audio                                   audio;

        /** 
         * @brief Audio timeline is configuration. 
         * 
         * Specifies how the Engine should record times lines. Timelines are used for instant replay and audio archival.  
         * 
         * @see engageQueryGroupTimeline, PFN_ENGAGE_GROUP_TIMELINE_EVENT_??? events
         */
        GroupTimeline                           timeline;
     
        /** @brief User alias to transmit as part part of the realtime audio stream when using the @ref engageBeginGroupTx API.  */
        std::string                             alias;

         /** @brief [Optional, Default: false] Set this to true if you do not want the Engine to advertise this Group on the Presence group. TODO: Shaun, please verify */
        bool                                    blockAdvertising;

        /** @brief [Optional, Default: null] Indicates the source of this configuration - e.g. from the application or discovered via Magellan */
        std::string                             source;

        /** 
         * @brief [Optional, Default: 0] Maximum number of seconds the Engine will receive for on this group.
         *
         * When the time limit is exceeded, the Engine will fire a PFN_ENGAGE_GROUP_MAX_RX_TIME_EXCEEDED event.
         */   
        int             maxRxSecs;


        Group()
        {
            clear();
        }

        void clear()
        {
            type = gtUnknown;
            id.clear();
            name.clear();
            interfaceName.clear();
            rx.clear();
            tx.clear();
            txOptions.clear();
            txAudio.clear();
            presence.clear();
            cryptoPassword.clear();

            alias.clear();
            
            rallypoints.clear();

            debugAudio = false;
            audio.clear();
            timeline.clear();

            blockAdvertising = false;

            source.clear();

            maxRxSecs = 0;
        }
    };

    static void to_json(nlohmann::json& j, const Group& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(type),
            TOJSON_IMPL(id),
            TOJSON_IMPL(name),
            TOJSON_IMPL(interfaceName),
            TOJSON_IMPL(rx),
            TOJSON_IMPL(tx),
            TOJSON_IMPL(txOptions),
            TOJSON_IMPL(txAudio),
            TOJSON_IMPL(presence),
            TOJSON_IMPL(cryptoPassword),
            TOJSON_IMPL(alias),
            TOJSON_IMPL(rallypoints),
            TOJSON_IMPL(alias),
            TOJSON_IMPL(audio),
            TOJSON_IMPL(timeline),
            TOJSON_IMPL(blockAdvertising),
            TOJSON_IMPL(source),
            TOJSON_IMPL(maxRxSecs)
        };
    }
    static void from_json(const nlohmann::json& j, Group& p)
    {
        p.clear();
        j.at("type").get_to(p.type);
        j.at("id").get_to(p.id);
        getOptional<std::string>("name", p.name, j);
        getOptional<std::string>("interfaceName", p.interfaceName, j);
        getOptional<NetworkAddress>("rx", p.rx, j);
        getOptional<NetworkAddress>("tx", p.tx, j);
        getOptional<NetworkTxOptions>("txOptions", p.txOptions, j);
        getOptional<std::string>("cryptoPassword", p.cryptoPassword, j);
        getOptional<std::string>("alias", p.alias, j);
        getOptional<TxAudio>("txAudio", p.txAudio, j);
        getOptional<Presence>("presence", p.presence, j);
        getOptional<std::vector<Rallypoint>>("rallypoints", p.rallypoints, j);
        getOptional<Audio>("audio", p.audio, j);
        getOptional<GroupTimeline>("timeline", p.timeline, j);
        getOptional<bool>("blockAdvertising", p.blockAdvertising, j, false);
        getOptional<std::string>("source", p.source, j);
        getOptional<int>("maxRxSecs", p.maxRxSecs, j, 0);
    }
    
    
    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(Mission)
    class Mission : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(Mission)
        
    public:
        std::string                             id;
        std::string                             name;
        std::vector<Group>                      groups;        
        std::chrono::system_clock::time_point   begins;
        std::chrono::system_clock::time_point   ends;

        void clear()
        {
            id.clear();
            name.clear();
            groups.clear();
        }
    };

    static void to_json(nlohmann::json& j, const Mission& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(id),
            TOJSON_IMPL(name),
            TOJSON_IMPL(groups)
        };        
    }

    static void from_json(const nlohmann::json& j, Mission& p)
    {
        p.clear();
        j.at("id").get_to(p.id);
        j.at("name").get_to(p.name);

        // Groups are optional
        try
        {
            j.at("groups").get_to(p.groups);
        }
        catch(...)
        {
            p.groups.clear();
        }

        std::string s;
    }
    
    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(LicenseDescriptor)
    /**
    * @brief Helper class for serializing and deserializing the LicenseDescriptor JSON 
    * 
    * Helper C++ class to serialize and de-serialize LicenseDescriptor JSON 
    * 
    * Example: @include[doc] examples/LicenseDescriptor.json 
    *    
    * @see TODO: engageGetActiveLicenseDescriptor, engageGetLicenseDescriptor 
    */       
    class LicenseDescriptor : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(LicenseDescriptor)

    /** @addtogroup licensingStatusCodes Licensing Codes
     *  
     * Status codes used to determine licensing status
     *  @{
     */        
    public:
        static const int STATUS_OK = 0;
        static const int ERR_NULL_ENTITLEMENT_KEY = -1;
        static const int ERR_NULL_LICENSE_KEY = -2;
        static const int ERR_INVALID_LICENSE_KEY_LEN = -3;
        static const int ERR_LICENSE_KEY_VERIFICATION_FAILURE = -4;
        static const int ERR_ACTIVATION_CODE_VERIFICATION_FAILURE = -5;
        static const int ERR_INVALID_EXPIRATION_DATE = -6;
        static const int ERR_GENERAL_FAILURE = -7;
        static const int ERR_NOT_INITIALIZED = -8;
        static const int ERR_REQUIRES_ACTIVATION = -9;
        /** @} */ 

        /** 
         * @brief Entitlement key to use for the product.
         *
         * The entitlement key is a unique key generated by an application developer so that any license keys generated by 
         * Rally Tactical's licensing system can only be used on a product with the same matching entitlement key. E.g If you develop two 
         * products and you would like to issue license keys independently and would like the Engage Engine and licensing system to handle the 
         * entitlement, then you should generate and separate entitlement key for each application.  
         */
        std::string                             entitlement;

        /** 
         * @brief License Key to be used for the application. 
         * 
         * This key is generated by the Rally Tactical Licensing systems and requires the entitlement key for creation. This key is then 
         * locked to the entitlement key and can never be changed.  
        */
        std::string                             key;

        /** @brief If the key required activation, this is the activation code generated using the entitlement, key and deviceId. */
        std::string                             activationCode;

        /** @brief [Read only] Unique device identifier generated by the Engine. */
        std::string                             deviceId;

        /** @brief [Read only]  TODO: Shaun, what does this mean?. */
        int                                     type;

        /** @brief [Read only] The time that the license key or activation code expires in Unix timestamp - Zulu/UTC. */
        time_t                                  expires;                

        /** @brief [Read only] The time that the license key or activation code expires formatted in ISO 8601 format, Zulu/UTC. */
        std::string                             expiresFormatted; 
        
        /** @brief TODO: Shaun, I assume that these are the bit flags or whatever the app wants to do with them */
        int                                     flags;

        /** @brief [Deprecated] TODO: Shaun, verify? */
        std::string                             refreshUri;

        /** @brief [Deprecated] TODO: Shaun, verify? */
        std::string                             cargo;

        /** @brief [Deprecated] TODO: Shaun, verify? */
        int                                     refreshIntervalDays;

        /** @brief The current licensing status.
         *  
         * @see licensingStatusCodes
         */
        int                                     status;

        /** @brief [Read only] Manufacturer ID. */
        std::string                             manufacturerId;

        LicenseDescriptor()
        {
            clear();
        }

        void clear()
        {
            entitlement.clear();
            key.clear();
            activationCode.clear();
            type = 0;
            expires = 0;
            expiresFormatted.clear();
            flags = 0;
            refreshUri.clear();
            cargo.clear();
            deviceId.clear();
            refreshIntervalDays = 0;
            status = ERR_NOT_INITIALIZED;
            manufacturerId.clear();
        }
    };

    static void to_json(nlohmann::json& j, const LicenseDescriptor& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(entitlement),
            TOJSON_IMPL(key),
            TOJSON_IMPL(activationCode),
            TOJSON_IMPL(type),
            TOJSON_IMPL(expires),
            TOJSON_IMPL(expiresFormatted),
            TOJSON_IMPL(flags),
            TOJSON_IMPL(refreshUri),
            TOJSON_IMPL(cargo),
            TOJSON_IMPL(deviceId),
            TOJSON_IMPL(refreshIntervalDays),
            TOJSON_IMPL(status),
            TOJSON_IMPL(manufacturerId)
        };        
    }

    static void from_json(const nlohmann::json& j, LicenseDescriptor& p)
    {
        p.clear();
        FROMJSON_IMPL(entitlement, std::string, EMPTY_STRING);
        FROMJSON_IMPL(key, std::string, EMPTY_STRING);
        FROMJSON_IMPL(activationCode, std::string, EMPTY_STRING);
        FROMJSON_IMPL(type, int, 0);
        FROMJSON_IMPL(expires, time_t, 0);
        FROMJSON_IMPL(expiresFormatted, std::string, EMPTY_STRING);
        FROMJSON_IMPL(flags, int, 0);
        FROMJSON_IMPL(refreshUri, std::string, EMPTY_STRING);
        FROMJSON_IMPL(cargo, std::string, EMPTY_STRING);
        FROMJSON_IMPL(deviceId, std::string, EMPTY_STRING);
        FROMJSON_IMPL(refreshIntervalDays, int, 0);
        FROMJSON_IMPL(status, int, LicenseDescriptor::ERR_NOT_INITIALIZED);
        FROMJSON_IMPL(manufacturerId, std::string, EMPTY_STRING);
    }


    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(EnginePolicyNetworking)
    /**
    * @brief TODO: Shaun, can you document this class please 
    * 
    * Helper C++ class to serialize and de-serialize EnginePolicyNetworking JSON 
    * 
    * Example: @include[doc] examples/EnginePolicyNetworking.json 
    *    
    * @see TODO: Add references 
    */       
    class EnginePolicyNetworking : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(EnginePolicyNetworking)
        
    public:

        /** @brief This is the default netork interface card the Engage Engine should bind to. TODO: Shaun */ 
        std::string         defaultNic;

        /** @brief [Optional, Default: 100] TODO: Shaun. */ 
        int                 maxOutputQueuePackets;

        /** @brief [Optional, Default: 100] TODO: Shaun. */ 
        int                 rtpJitterMinMs;

        int                 rtpJitterMaxFactor;

        /** @brief [Optional, Default: 1000] TODO: Shaun. */ 
        int                 rtpJitterMaxMs;

        /** @brief [Optional, Default: 5] TODO: Shaun. */ 
        int                 rtpLatePacketSequenceRange;

        int                 rtpJitterOverflowTrimPercentage;
        int                 rtpJitterUnderrunReductionThresholdMs;

        /** @brief [Optional, Default: 2000] TODO: Shaun. */ 
        int                 rtpLatePacketTimestampRangeMs;

        /** @brief [Optional, Default: 500] TODO: Shaun. */ 
        int                 rtpInboundProcessorInactivityMs;

        /** @brief [Optional, Default: 8] TODO: Shaun. */ 
        int                 multicastRejoinSecs;

        /** @brief [Optional, Default: 10] TODO: Shaun. */ 
        int                 rpLeafConnectTimeoutSecs;

        /** @brief [Optional, Default: 5000] TODO: Shaun. */ 
        int                 maxReconnectPauseMs;

        /** @brief [Optional, Default: 500] TODO: Shaun. */ 
        int                 reconnectFailurePauseIncrementMs;

        /** @brief [Optional, Default: 1000] TODO: Shaun. */ 
        int                 sendFailurePauseMs;

        /** @brief [Optional, Default: 6000] TODO: Shaun. */ 
        int                 rallypointRtTestIntervalMs;
        bool                logRtpJitterBufferStats;

        EnginePolicyNetworking()
        {
            clear();
        }

        void clear()
        {
            defaultNic.clear();
            maxOutputQueuePackets = 100;
            rtpJitterMinMs = 100;
            rtpJitterMaxFactor = 8;
            rtpJitterOverflowTrimPercentage = 10;
            rtpJitterUnderrunReductionThresholdMs = 1500;
            rtpLatePacketSequenceRange = 5;
            rtpLatePacketTimestampRangeMs = 2000;
            rtpInboundProcessorInactivityMs = 500;
            multicastRejoinSecs = 8;
            rpLeafConnectTimeoutSecs = 10;
            maxReconnectPauseMs = 5000;
            reconnectFailurePauseIncrementMs = 500;
            sendFailurePauseMs = 1000;
            rallypointRtTestIntervalMs = 60000;
            logRtpJitterBufferStats = false;
        }
    };

    static void to_json(nlohmann::json& j, const EnginePolicyNetworking& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(defaultNic),
            TOJSON_IMPL(maxOutputQueuePackets),
            TOJSON_IMPL(rtpJitterMinMs),
            TOJSON_IMPL(rtpJitterMaxFactor),
            TOJSON_IMPL(rtpJitterOverflowTrimPercentage),
            TOJSON_IMPL(rtpJitterUnderrunReductionThresholdMs),            
            TOJSON_IMPL(rtpLatePacketSequenceRange),
            TOJSON_IMPL(rtpLatePacketTimestampRangeMs),
            TOJSON_IMPL(rtpInboundProcessorInactivityMs),
            TOJSON_IMPL(multicastRejoinSecs),
            TOJSON_IMPL(rpLeafConnectTimeoutSecs),
            TOJSON_IMPL(maxReconnectPauseMs),
            TOJSON_IMPL(reconnectFailurePauseIncrementMs),
            TOJSON_IMPL(sendFailurePauseMs),
            TOJSON_IMPL(rallypointRtTestIntervalMs),
            TOJSON_IMPL(logRtpJitterBufferStats)
        };
    }
    static void from_json(const nlohmann::json& j, EnginePolicyNetworking& p)
    {
        p.clear();
        FROMJSON_IMPL(defaultNic, std::string, EMPTY_STRING);
        FROMJSON_IMPL(maxOutputQueuePackets, int, 100);
        FROMJSON_IMPL(rtpJitterMinMs, int, 8);
        FROMJSON_IMPL(rtpJitterMaxFactor, int, 8);
        FROMJSON_IMPL(rtpJitterOverflowTrimPercentage, int, 10);
        FROMJSON_IMPL(rtpJitterUnderrunReductionThresholdMs, int, 1500);
        FROMJSON_IMPL(rtpLatePacketSequenceRange, int, 5);
        FROMJSON_IMPL(rtpLatePacketTimestampRangeMs, int, 2000);
        FROMJSON_IMPL(rtpInboundProcessorInactivityMs, int, 500);
        FROMJSON_IMPL(multicastRejoinSecs, int, 8);
        FROMJSON_IMPL(rpLeafConnectTimeoutSecs, int, 10);
        FROMJSON_IMPL(maxReconnectPauseMs, int, 5000);
        FROMJSON_IMPL(reconnectFailurePauseIncrementMs, int, 500);
        FROMJSON_IMPL(sendFailurePauseMs, int, 1000);
        FROMJSON_IMPL(rallypointRtTestIntervalMs, int, 60000);
        FROMJSON_IMPL(logRtpJitterBufferStats, bool, false);
    }           


    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(EnginePolicyAudio)
    /**
    * @brief Default audio settings for Engage Engine policy. TODO: Shaun, can you document this class please 
    * 
    * Helper C++ class to serialize and de-serialize EnginePolicyAudio JSON 
    * 
    * Example: @include[doc] examples/EnginePolicyAudio.json 
    *    
    * @see TODO: ConfigurationObjects::EnginePolicy
    */
    class EnginePolicyAudio : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(EnginePolicyAudio)
        
    public:
        int                 internalRate;
        int                 internalChannels;

        int                 inputBufferMs;
        int                 inputChannels;
        int                 inputRate;

        int                 outputBufferMs;

        /** @brief [Optional, Default: 2] TODO: Shaun. */ 
        int                 outputChannels;

        /** @brief [Optional, Default: ?] TODO: Shaun. */ 
        int                 outputRate;

        /** @brief [Optional, Default: 0] TODO: Shaun. */ 
        int                 outputGainPercentage;

        /** @brief [Optional, Default: false] TODO: Shaun. */ 
        bool                allowOutputOnTransmit;


        EnginePolicyAudio()
        {
            clear();
        }

        void clear()
        {
            internalRate = 16000;
            internalChannels = 1;

            inputBufferMs = 60;
            inputChannels = 1;
            inputRate = 16000;

            outputBufferMs = 60;
            outputChannels = 2;
            outputRate = 16000;

            outputGainPercentage = 0;
            allowOutputOnTransmit = false;
        }
    };

    static void to_json(nlohmann::json& j, const EnginePolicyAudio& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(internalRate),
            TOJSON_IMPL(internalChannels),            
            TOJSON_IMPL(inputBufferMs),
            TOJSON_IMPL(inputChannels),
            TOJSON_IMPL(inputRate),
            TOJSON_IMPL(outputBufferMs),
            TOJSON_IMPL(outputChannels),
            TOJSON_IMPL(outputRate),
            TOJSON_IMPL(outputChannels),
            TOJSON_IMPL(outputGainPercentage),
            TOJSON_IMPL(allowOutputOnTransmit)
        };
    }
    static void from_json(const nlohmann::json& j, EnginePolicyAudio& p)
    {
        p.clear();
        FROMJSON_IMPL(internalRate, int, 16000);
        FROMJSON_IMPL(internalChannels, int, 1);

        FROMJSON_IMPL(inputBufferMs, int, 60);
        FROMJSON_IMPL(inputChannels, int, 1);
        FROMJSON_IMPL(inputRate, int, 16000);

        FROMJSON_IMPL(outputBufferMs, int, 60);
        FROMJSON_IMPL(outputChannels, int, 2);
        FROMJSON_IMPL(outputRate, int, 16000);

        FROMJSON_IMPL(outputGainPercentage, int, 0);
        FROMJSON_IMPL(allowOutputOnTransmit, bool, false);
    }           

    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(SecurityCertificate)
    /**
    * @brief Configuration for a Security Certificate used in various configurations. TODO: Shaun please review
    * 
    * Helper C++ class to serialize and de-serialize SecurityCertificate JSON 
    * 
    * Example: @include[doc] examples/SecurityCertificate.json
    *    
    * @see ConfigurationObjects::EnginePolicySecurity, ConfigurationObjects::RallypointServer
    */       
    class SecurityCertificate : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(SecurityCertificate)
        
    public:

        /** 
         * @brief x509 certificate text.
         * 
         * TODO: Shaun, do we need to expand on this 
         */ 
        std::string         certificate;

        /** @brief Private key for the certificate. TODO: Shaun, please verify. */ 
        std::string         key;

        SecurityCertificate()
        {
            clear();
        }

        void clear()
        {
            certificate.clear();
            key.clear();
        }
    };

    static void to_json(nlohmann::json& j, const SecurityCertificate& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(certificate),
            TOJSON_IMPL(key)
        };
    }
    static void from_json(const nlohmann::json& j, SecurityCertificate& p)
    {
        p.clear();
        FROMJSON_IMPL(certificate, std::string, EMPTY_STRING);
        FROMJSON_IMPL(key, std::string, EMPTY_STRING);

        if(!p.certificate.empty() && p.certificate.c_str()[0] == '@')
        {
            if(!readTextFileIntoString(p.certificate.c_str() + 1, p.certificate))
            {
                p.certificate.clear();
            }
        }

        if(!p.key.empty() && p.key.c_str()[0] == '@')
        {
            if(!readTextFileIntoString(p.key.c_str() + 1, p.key))
            {
                p.key.clear();
            }
        }      
    }           
    
    // This is where spell checking stops
    //----------------------------------------------------------- 
    JSON_SERIALIZED_CLASS(EnginePolicySecurity)

    /**
    * @brief Default certificate to use for security operation in the Engage Engine.  
    * 
    * For example .... TODO: Shaun, can you please provide examples
    * 
    * Helper C++ class to serialize and de-serialize EnginePolicySecurity JSON 
    * 
    * Example: @include[doc] examples/EnginePolicySecurity.json 
    *    
    * @see ConfigurationObjects::EnginePolicy
    */       
    class EnginePolicySecurity : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(EnginePolicySecurity)
        
    public:

        /** @brief TODO: Shaun, is this optional as I recall there is a built in CERT the engine uses ... should we document that?? */ 
        SecurityCertificate     certificate;

        EnginePolicySecurity()
        {
            clear();
        }

        void clear()
        {
            certificate.clear();
        }
    };

    static void to_json(nlohmann::json& j, const EnginePolicySecurity& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(certificate),
        };
    }
    static void from_json(const nlohmann::json& j, EnginePolicySecurity& p)
    {
        p.clear();
        getOptional("certificate", p.certificate, j);
    }           

    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(EnginePolicyLogging)
    /**
    * @brief Engine logging settings. TODO: Shaun, please verify this.  
    * 
    * Helper C++ class to serialize and de-serialize EnginePolicyLogging JSON 
    * 
    * Example: @include[doc] examples/EnginePolicyLogging.json 
    *    
    * @see ConfigurationObjects::EnginePolicy
    */       
    class EnginePolicyLogging : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(EnginePolicyLogging)
        
    public:

        /** 
         * @brief [Optional, Default: 4, Range: 0-7] This is the maximum logging level to display in other words, any logging with levels equal or lower than this level will be logged. TODO: Shaun, can you please verify this?
         *
         * Logging levels 
         * Value    | Severity      | Description 
         * ---      | ---           | ---
         * 0        | Emergency     | System is unusable
         * 1        | Alert         | Action must be taken immediately
         * 2        | Critical      | Critical conditions
         * 3        | Error         | Error conditions
         * 4        | Warning       | Warning conditions
         * 5        | Notice        | Normal but significant conditions
         * 6        | Informational | Informational messages
         * 7        | debug         | Debug-level messages
         *          
         */
        int     maxLevel;

        /** [Optional, Default: false] When enabled, the Engage Engine will output logging to Sys log as well. */
        bool    enableSyslog;

        EnginePolicyLogging()
        {
            clear();
        }

        void clear()
        {
            maxLevel = 4;       // ILogger::Level::debug
            enableSyslog = false;
        }
    };

    static void to_json(nlohmann::json& j, const EnginePolicyLogging& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(maxLevel),
            TOJSON_IMPL(enableSyslog)
        };
    }
    static void from_json(const nlohmann::json& j, EnginePolicyLogging& p)
    {
        p.clear();
        getOptional("maxLevel", p.maxLevel, j, 4);          // ILogger::Level::debug
        getOptional("enableSyslog", p.enableSyslog, j);
    }           


    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(EnginePolicyDatabase)
    class EnginePolicyDatabase : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(EnginePolicyDatabase)
        
    public:
        typedef enum
        {
            dbtFixedMemory          = 0,
            dbtPagedMemory          = 1,
            dbtFixedFile            = 2
        } DatabaseType_t;

        bool                enabled;
        DatabaseType_t      type;
        std::string         fixedFileName;
        bool                forceMaintenance;
        bool                reclaimSpace;

        EnginePolicyDatabase()
        {
            clear();
        }

        void clear()
        {
            enabled = true;
            type = DatabaseType_t::dbtFixedMemory;
            fixedFileName.clear();
            forceMaintenance = false;
            reclaimSpace = false;
        }
    };

    static void to_json(nlohmann::json& j, const EnginePolicyDatabase& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(enabled),
            TOJSON_IMPL(type),
            TOJSON_IMPL(fixedFileName),
            TOJSON_IMPL(forceMaintenance),
            TOJSON_IMPL(reclaimSpace)
        };
    }
    static void from_json(const nlohmann::json& j, EnginePolicyDatabase& p)
    {
        p.clear();
        FROMJSON_IMPL(enabled, bool, true);
        FROMJSON_IMPL(type, EnginePolicyDatabase::DatabaseType_t, EnginePolicyDatabase::DatabaseType_t::dbtFixedMemory);
        FROMJSON_IMPL(fixedFileName, std::string, EMPTY_STRING);
        FROMJSON_IMPL(forceMaintenance, bool, false);
        FROMJSON_IMPL(reclaimSpace, bool, false);
    }  

    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(EnginePolicyLicensing)
    /**
    * @brief Licensing settings
    * 
    * Used to enable the Engage Engine for production features 
    * 
    * Helper C++ class to serialize and de-serialize EnginePolicyLicensing JSON 
    * 
    * Example: @include[doc] examples/EnginePolicyLicensing.json 
    *    
    * @see engageInitialize, engageGetActiveLicenseDescriptor, engageGetLicenseDescriptor, engageUpdateLicense, ConfigurationObjects::EnginePolicy,  
    */       
    class EnginePolicyLicensing : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(EnginePolicyLicensing)
        
    public:

        /** @brief Entitlement key to use for the product. See @ref LicenseDescriptor::entitlement for details */
        std::string         entitlement;

        /** @brief License key. See @ref LicenseDescriptor::key for details */
        std::string         key;

        /** @brief Activation Code issued for the license key. See @ref LicenseDescriptor::activationCode for details */
        std::string         activationCode;

        /** @brief Device Identifier. See @ref LicenseDescriptor::deviceId for details */
        std::string         deviceId;

        /** @brief Manufacturer ID to use for the product. See @ref LicenseDescriptor::manufacturerId for details */
        std::string         manufacturerId;

        EnginePolicyLicensing()
        {
            clear();
        }

        void clear()
        {
            entitlement.clear();
            key.clear();
            activationCode.clear();
            deviceId.clear();
            manufacturerId.clear();
        }
    };

    static void to_json(nlohmann::json& j, const EnginePolicyLicensing& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(entitlement),
            TOJSON_IMPL(key),
            TOJSON_IMPL(activationCode),
            TOJSON_IMPL(deviceId),
            TOJSON_IMPL(manufacturerId)
        };
    }
    static void from_json(const nlohmann::json& j, EnginePolicyLicensing& p)
    {
        p.clear();
        FROMJSON_IMPL(entitlement, std::string, EMPTY_STRING);
        FROMJSON_IMPL(key, std::string, EMPTY_STRING);
        FROMJSON_IMPL(activationCode, std::string, EMPTY_STRING);
        FROMJSON_IMPL(deviceId, std::string, EMPTY_STRING);
        FROMJSON_IMPL(manufacturerId, std::string, EMPTY_STRING);
    }           

    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(DiscoverySsdp)
    /**
    * @brief <a href="https://en.wikipedia.org/wiki/Simple_Service_Discovery_Protocol" target="_blank">Simple Service Discovery Protocol</a>  settings.
    * TODO: Shaun please verify
    * 
    * Helper C++ class to serialize and de-serialize DiscoverySsdp JSON 
    * 
    * Example: @include[doc] examples/DiscoverySsdp.json 
    *    
    * @see engageInitialize, ConfigurationObjects::DiscoveryConfiguration 
    */       
    class DiscoverySsdp : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(DiscoverySsdp)
        
    public:

        /** @brief [Optional, Default: false] Enables the Engage Engine to use SSDP for asset discovery. TODO: Shaun, is this correct? */ 
        bool                                    enabled;

        /** @brief [Optional, Default: default system interface] The network interface to bind to receiving discovery packets.  */ 
        std::string                             interfaceName;

        /** @brief TODO: Shaun, can you please complete? */ 
        NetworkAddress                          address;

        /** @brief TODO: Shaun, can you please complete? */ 
        std::vector<std::string>                searchTerms;

        /** @brief TODO: Shaun, can you please complete? */ 
        int                                     ageTimeoutMs;

        /** @brief TODO: Shaun, can you please complete? */ 
        Advertising                             advertising;

        DiscoverySsdp()
        {
            clear();
        }

        void clear()
        {
            enabled = false;
            interfaceName.clear();
            address.clear();
            searchTerms.clear();
            ageTimeoutMs = 30000;
            advertising.clear();
        }
    };

    static void to_json(nlohmann::json& j, const DiscoverySsdp& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(enabled),
            TOJSON_IMPL(interfaceName),
            TOJSON_IMPL(address),
            TOJSON_IMPL(searchTerms),
            TOJSON_IMPL(ageTimeoutMs),
            TOJSON_IMPL(advertising)
        };
    }
    static void from_json(const nlohmann::json& j, DiscoverySsdp& p)
    {
        p.clear();
        getOptional("enabled", p.enabled, j, false);
        getOptional<std::string>("interfaceName", p.interfaceName, j);
        getOptional<NetworkAddress>("address", p.address, j);
        getOptional<std::vector<std::string>>("searchTerms", p.searchTerms, j);
        getOptional<int>("ageTimeoutMs", p.ageTimeoutMs, j, 30000);
        getOptional<Advertising>("advertising", p.advertising, j);
    }

    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(DiscoverySap)
    /**
    * @brief <a href="https://en.wikipedia.org/wiki/Session_Announcement_Protocol" target="_blank">Session Announcement Discovery settings</a>  settings. 
    * TODO: Shaun please verify
    * 
    * Helper C++ class to serialize and de-serialize DiscoverySap JSON 
    * 
    * Example: @include[doc] examples/DiscoverySap.json 
    *    
    * @see engageInitialize, ConfigurationObjects::DiscoveryConfiguration 
    */       
    class DiscoverySap : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(DiscoverySap)
        
    public:
        /** @brief [Optional, Default: false] Enables the Engage Engine to use SAP for asset discovery. TODO: Shaun, is this correct? */ 
        bool                                    enabled;

        /** @brief [Optional, Default: default system interface] The network interface to bind to receiving discovery packets.  */ 
        std::string                             interfaceName;

        /** @brief TODO: Shaun, can you please complete? */ 
        NetworkAddress                          address;

        /** @brief TODO: Shaun, can you please complete? */ 
        int                                     ageTimeoutMs;

        /** @brief TODO: Shaun, can you please complete? */ 
        Advertising                             advertising;

        DiscoverySap()
        {
            clear();
        }

        void clear()
        {
            enabled = false;
            interfaceName.clear();
            address.clear();
            ageTimeoutMs = 30000;
            advertising.clear();
        }
    };

    static void to_json(nlohmann::json& j, const DiscoverySap& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(enabled),
            TOJSON_IMPL(interfaceName),
            TOJSON_IMPL(address),
            TOJSON_IMPL(ageTimeoutMs),
            TOJSON_IMPL(advertising)
        };
    }
    static void from_json(const nlohmann::json& j, DiscoverySap& p)
    {
        p.clear();
        getOptional("enabled", p.enabled, j, false);
        getOptional<std::string>("interfaceName", p.interfaceName, j);
        getOptional<NetworkAddress>("address", p.address, j);
        getOptional<int>("ageTimeoutMs", p.ageTimeoutMs, j, 30000);
        getOptional<Advertising>("advertising", p.advertising, j);
    }

    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(DiscoveryCistech)
    /**
    * @brief Cistech Discovery settings 
    * 
    * Used to configure the Engage Engine to discovery Cistech Gateways on the network using a propriety Citech protocol
    * 
    * Helper C++ class to serialize and de-serialize DiscoveryCistech JSON 
    * 
    * Example: @include[doc] examples/DiscoveryCistech.json 
    *    
    * @see engageInitialize, ConfigurationObjects::DiscoveryConfiguration     
    */       
    class DiscoveryCistech : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(DiscoveryCistech)
        
    public:
        bool                                    enabled;
        std::string                             interfaceName;
        NetworkAddress                          address;
        int                                     ageTimeoutMs;

        DiscoveryCistech()
        {
            clear();
        }

        void clear()
        {
            enabled = false;
            interfaceName.clear();
            address.clear();
            ageTimeoutMs = 30000;
        }
    };

    static void to_json(nlohmann::json& j, const DiscoveryCistech& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(enabled),
            TOJSON_IMPL(interfaceName),
            TOJSON_IMPL(address),
            TOJSON_IMPL(ageTimeoutMs)
        };
    }
    static void from_json(const nlohmann::json& j, DiscoveryCistech& p)
    {
        p.clear();
        getOptional("enabled", p.enabled, j, false);
        getOptional<std::string>("interfaceName", p.interfaceName, j);
        getOptional<NetworkAddress>("address", p.address, j);
        getOptional<int>("ageTimeoutMs", p.ageTimeoutMs, j, 30000);
    }


    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(DiscoveryTrellisware)
    /**
    * @brief Trellisware Discovery settings
    *     
    * Helper C++ class to serialize and de-serialize DiscoveryTrellisware JSON 
    * 
    * Example: @include[doc] examples/DiscoveryTrellisware.json 
    *    
    * @see engageInitialize, ConfigurationObjects::DiscoveryConfiguration 
    */       
    class DiscoveryTrellisware : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(DiscoveryTrellisware)
        
    public:

        /** Enable Discovery */
        bool                                    enabled;

        DiscoveryTrellisware()
        {
            clear();
        }

        void clear()
        {
            enabled = false;
        }
    };

    static void to_json(nlohmann::json& j, const DiscoveryTrellisware& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(enabled)
        };
    }
    static void from_json(const nlohmann::json& j, DiscoveryTrellisware& p)
    {
        p.clear();
        getOptional("enabled", p.enabled, j, false);
    }

    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(DiscoveryConfiguration)
    /**
    * @brief Configuration for the Discovery features 
    * 
    * Helper C++ class to serialize and de-serialize DiscoveryConfiguration JSON 
    * 
    * Example: @include[doc] examples/DiscoveryConfiguration.json 
    *    
    * @see engageInitialize, ConfigurationObjects::EnginePolicy 
    */       
    class DiscoveryConfiguration : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(DiscoveryConfiguration)
        
    public:
        /** [Optional] SSDP (Simple Session Discovery Protocol)  configuration. */
        DiscoverySsdp                           ssdp;

        /** [Optional] SAP (Simple Session Discovery Protocol) configuration. */
        DiscoverySap                            sap;

        /** [Optional] SSDP (Simple Session Discovery Protocol) configuration. */
        DiscoveryCistech                        cistech;

        /** [Optional] SSDP (Simple Session Discovery Protocol) configuration. */
        DiscoveryTrellisware                    trellisware;

        DiscoveryConfiguration()
        {
            clear();
        }

        void clear()
        {
            ssdp.clear();
            sap.clear();
            cistech.clear();
        }
    };

    static void to_json(nlohmann::json& j, const DiscoveryConfiguration& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(ssdp),
            TOJSON_IMPL(sap),
            TOJSON_IMPL(cistech),
            TOJSON_IMPL(trellisware)
        };
    }
    static void from_json(const nlohmann::json& j, DiscoveryConfiguration& p)
    {
        p.clear();
        getOptional<DiscoverySsdp>("ssdp", p.ssdp, j);
        getOptional<DiscoverySap>("sap", p.sap, j);
        getOptional<DiscoveryCistech>("cistech", p.cistech, j);
        getOptional<DiscoveryTrellisware>("trellisware", p.trellisware, j);
    }


    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(EnginePolicyInternals)
    /**
    * @brief Internal Engage Engine settings
    * 
    * These settings are used to configure internal parameters.   
    * 
    * Helper C++ class to serialize and de-serialize EnginePolicyInternals JSON 
    * 
    * Example: @include[doc] examples/EnginePolicyInternals.json 
    *    
    * @see engageInitialize, ConfigurationObjects::EnginePolicy 
    */       
    class EnginePolicyInternals : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(EnginePolicyInternals)
        
    public:

        /** @brief [Optional, Default: false] The watchdog will monitor internal message queues and if it detects an issue, it will abort the process. TODO: Shaun, is this correct*/
        bool                disableWatchdog;

        /** @brief [Optional, Default: 5000] The interval that the watchdog will periodically run at. TODO: Shaun, is this correct? */
        int                 watchdogIntervalMs;

        /** @brief [Optional, Default: 2000] The duration the watchdog thread will wait for a message to be processed before it declares that the process has hung. TODO: Shaun, is this correct? */
        int                 watchdogHangDetectionMs;

        /** @brief [Optional, Default: 1000] TODO: Shaun, no idea what this is */
        int                 housekeeperIntervalMs;

        /** @brief [Optional, Default: 30] The default duration the @ref engageBeginGroupTx and @ref engageBeginGroupTxAdvanced function will transmit for. */
        int                 maxTxSecs;
        int                 maxRxSecs;
        int                 logTaskQueueStatsIntervalMs;
        bool                enableLazySpeakerClosure;

        EnginePolicyInternals()
        {
            clear();
        }

        void clear()
        {
            disableWatchdog = false;
            watchdogIntervalMs = 5000;
            watchdogHangDetectionMs = 2000;
            housekeeperIntervalMs = 1000;
            logTaskQueueStatsIntervalMs = 0;
            maxTxSecs = 30;
            maxRxSecs = 0;
            enableLazySpeakerClosure = false;
        }
    };

    static void to_json(nlohmann::json& j, const EnginePolicyInternals& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(disableWatchdog),
            TOJSON_IMPL(watchdogIntervalMs),
            TOJSON_IMPL(watchdogHangDetectionMs),
            TOJSON_IMPL(housekeeperIntervalMs),
            TOJSON_IMPL(logTaskQueueStatsIntervalMs),
            TOJSON_IMPL(maxTxSecs),
            TOJSON_IMPL(maxRxSecs),
            TOJSON_IMPL(enableLazySpeakerClosure)
        };
    }
    static void from_json(const nlohmann::json& j, EnginePolicyInternals& p)
    {
        p.clear();
        getOptional<bool>("disableWatchdog", p.disableWatchdog, j, false);
        getOptional<int>("watchdogIntervalMs", p.watchdogIntervalMs, j, 5000);
        getOptional<int>("watchdogHangDetectionMs", p.watchdogHangDetectionMs, j, 2000);
        getOptional<int>("housekeeperIntervalMs", p.housekeeperIntervalMs, j, 1000);
        getOptional<int>("logTaskQueueStatsIntervalMs", p.logTaskQueueStatsIntervalMs, j, 0);        
        getOptional<int>("maxTxSecs", p.maxTxSecs, j, 30);
        getOptional<int>("maxRxSecs", p.maxRxSecs, j, 0);
        getOptional<bool>("enableLazySpeakerClosure", p.enableLazySpeakerClosure, j, false);
        
    }

    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(EnginePolicyTimelines)
    /**
    * @brief Engine Policy Timeline configuration.
    * 
    * Timelines are used to record audio for instant replay and archival purposes. The audio files contain "anti tampering" features.  
    * 
    * Helper C++ class to serialize and de-serialize EnginePolicyTimelines JSON 
    * 
    * Example: @include[doc] examples/EnginePolicyTimelines.json 
    *    
    * @see engageInitialize, engageQueryGroupTimeline, EnginePolicy 
    */       
    class EnginePolicyTimelines : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(EnginePolicyTimelines)
        
    public:

        /** 
         * @brief [Optional, Default: true] Specifies if Time Lines are enabled by default. 
         * 
         * Certain time line settings can be overridden at a per group level using the @ref ConfigurationObjects::GroupTimeline configuration. 
         */
        bool                                    enabled;

        /** @brief Specifies where the timeline recordings will be stored physically. */ 
        std::string                             storageRoot;

        /** Specifies the maximum storage to use for recordings. TODO: Shaun, is this memory or disk space? */
        int                                     maxStorageMb;

        /** @brief TODO: Shaun, no idea what this is */
        long                                    maxEventAgeSecs;

        /** @brief TODO: Shaun, can you document this please */
        int                                     maxEvents;

        /** @brief TODO: Shaun, can you document this please */
        long                                    groomingIntervalSecs;

        /** 
         * @brief The certificate to use for signing the recording with. 
         * 
         * This is part of the anti tampering feature. TODO: Shaun, can you expand on this please 
         * 
         */ 
        SecurityCertificate                     security;
        long                                    autosaveIntervalSecs;
        bool                                    disableSigningAndVerification;

        EnginePolicyTimelines()
        {
            clear();
        }

        void clear()
        {
            enabled = true;
            storageRoot.clear();
            maxStorageMb = 128;
            maxEventAgeSecs = (86400 * 30);         // 30 days
            groomingIntervalSecs = (60 * 30);       // 30 minutes
            maxEvents = 1000;
            autosaveIntervalSecs = 5;               
            security.clear();
            disableSigningAndVerification = false;
        }
    };

    static void to_json(nlohmann::json& j, const EnginePolicyTimelines& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(enabled),
            TOJSON_IMPL(storageRoot),
            TOJSON_IMPL(maxStorageMb),
            TOJSON_IMPL(maxEventAgeSecs),
            TOJSON_IMPL(maxEvents),
            TOJSON_IMPL(groomingIntervalSecs),
            TOJSON_IMPL(autosaveIntervalSecs),                        
            TOJSON_IMPL(security),
            TOJSON_IMPL(disableSigningAndVerification)
        };
    }
    static void from_json(const nlohmann::json& j, EnginePolicyTimelines& p)
    {
        p.clear();
        getOptional<bool>("enabled", p.enabled, j, true);
        getOptional<std::string>("storageRoot", p.storageRoot, j, EMPTY_STRING);
        getOptional<int>("maxStorageMb", p.maxStorageMb, j, 128);
        getOptional<long>("maxEventAgeSecs", p.maxEventAgeSecs, j, (86400 * 30));       
        getOptional<long>("groomingIntervalSecs", p.groomingIntervalSecs, j, (60 * 30));     
        getOptional<long>("autosaveIntervalSecs", p.autosaveIntervalSecs, j, 5);     
        getOptional<int>("maxEvents", p.maxEvents, j, 1000);
        getOptional<SecurityCertificate>("security", p.security, j);
        getOptional<bool>("disableSigningAndVerification", p.disableSigningAndVerification, j, false);
    }

    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(EnginePolicy)
    /**
    * @brief Provides Engage Engine policy configuration.
    * 
    * Helper C++ class to serialize and de-serialize EnginePolicy JSON 
    * 
    * Provided at Engage Engine initialization time to specify all the policy settings for the Engage Engine.
    *  
    * Example JSON: @include[doc] examples/EnginePolicy.json 
    * 
    * @see engageInitialize 
    */    
    class EnginePolicy : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(EnginePolicy)
        
    public:

        /** @brief Specifies the physical path to store data. TODO: Shaun, what is this used for? */
        std::string                 dataDirectory;  

        /** @brief Licensing settings */ 
        EnginePolicyLicensing       licensing;      

        /** @brief Security settings */ 
        EnginePolicySecurity        security;
                      
        /** @brief Security settings */ 
        EnginePolicyNetworking      networking;
                      
        /** @brief Audio settings */ 
        EnginePolicyAudio           audio;
                      
        /** @brief Discovery settings */ 
        DiscoveryConfiguration      discovery;
                      
        /** @brief Logging settings */ 
        EnginePolicyLogging         logging;
                      
        /** @brief Internal settings */ 
        EnginePolicyInternals       internals;
                      
        /** @brief Timelines settings */ 
        EnginePolicyTimelines       timelines;
        EnginePolicyDatabase        database;

        EnginePolicy()
        {
            clear();
        }

        void clear()
        {
            dataDirectory.clear();
            licensing.clear();
            security.clear();
            networking.clear();
            audio.clear();
            discovery.clear();
            logging.clear();
            internals.clear();
            timelines.clear();
            database.clear();
        }
    };

    static void to_json(nlohmann::json& j, const EnginePolicy& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(dataDirectory),
            TOJSON_IMPL(licensing),
            TOJSON_IMPL(security),
            TOJSON_IMPL(networking),
            TOJSON_IMPL(audio),
            TOJSON_IMPL(discovery),
            TOJSON_IMPL(logging),
            TOJSON_IMPL(internals),
            TOJSON_IMPL(timelines),
            TOJSON_IMPL(database)
        };
    }
    static void from_json(const nlohmann::json& j, EnginePolicy& p)
    {
        p.clear();
        FROMJSON_IMPL_SIMPLE(dataDirectory);
        FROMJSON_IMPL_SIMPLE(licensing);
        FROMJSON_IMPL_SIMPLE(security);
        FROMJSON_IMPL_SIMPLE(networking);
        FROMJSON_IMPL_SIMPLE(audio);
        FROMJSON_IMPL_SIMPLE(discovery);
        FROMJSON_IMPL_SIMPLE(logging);
        FROMJSON_IMPL_SIMPLE(internals);
        FROMJSON_IMPL_SIMPLE(timelines);
        FROMJSON_IMPL_SIMPLE(database);
    } 


    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(TalkgroupAsset)
    /**
    * @brief TODO: Complete class 
    * TODO: Shaun, can you please complete
    * 
    * Helper C++ class to serialize and de-serialize TalkgroupAsset JSON 
    * 
    * Example: @include[doc] examples/TalkgroupAsset.json 
    *    
    * @see TODO: Add references 
    */       
    class TalkgroupAsset : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(TalkgroupAsset)
        
    public:

        /** @brief TODO: Shaun, can you please complete */
        std::string     nodeId;

        /** @brief TODO: Shaun, can you please complete */
        Group           group;

        TalkgroupAsset()
        {
            clear();
        }

        void clear()
        {
            nodeId.clear();
            group.clear();
        }
    };
    
    static void to_json(nlohmann::json& j, const TalkgroupAsset& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(nodeId),
            TOJSON_IMPL(group)
        };
    }
    static void from_json(const nlohmann::json& j, TalkgroupAsset& p)
    {
        p.clear();
        getOptional<std::string>("nodeId", p.nodeId, j);
        getOptional<Group>("group", p.group, j);
    }

    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(EngageDiscoveredGroup)
    /**
    * @brief TODO: Complete class 
    * TODO: Shaun, can you please complete
    * 
    * Helper C++ class to serialize and de-serialize EngageDiscoveredGroup JSON 
    * 
    * Example: @include[doc] examples/EngageDiscoveredGroup.json 
    *    
    * @see TODO: Add references 
    */       
    class EngageDiscoveredGroup : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(EngageDiscoveredGroup)
        
    public:
        std::string     id;
        int             type;
        NetworkAddress  rx;
        NetworkAddress  tx;

        EngageDiscoveredGroup()
        {
            clear();
        }

        void clear()
        {
            id.clear();
            type = 0;
            rx.clear();
            tx.clear();
        }
    };
    
    static void to_json(nlohmann::json& j, const EngageDiscoveredGroup& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(id),
            TOJSON_IMPL(type),
            TOJSON_IMPL(rx),
            TOJSON_IMPL(tx)
        };
    }
    static void from_json(const nlohmann::json& j, EngageDiscoveredGroup& p)
    {
        p.clear();
        getOptional<std::string>("id", p.id, j);
        getOptional<int>("type", p.type, j, 0);
        getOptional<NetworkAddress>("rx", p.rx, j);
        getOptional<NetworkAddress>("tx", p.tx, j);
    }

    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(RallypointPeer)
    /**
    * @brief TODO: Shaun, is this even needed to be documented? 
    * 
    * Helper C++ class to serialize and de-serialize RallypointPeer JSON 
    * 
    * Example: @include[doc] examples/RallypointPeer.json 
    *    
    * @see RallypointServer  
    */       
    class RallypointPeer : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(RallypointPeer)
        
    public:
        std::string             id;
        bool                    enabled;
        NetworkAddress          host;
        SecurityCertificate     certificate;

        RallypointPeer()
        {
            clear();
        }

        void clear()
        {
            id.clear();
            enabled = true;
            host.clear();
            certificate.clear();
        }
    };

    static void to_json(nlohmann::json& j, const RallypointPeer& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(id),
            TOJSON_IMPL(enabled),
            TOJSON_IMPL(host),
            TOJSON_IMPL(certificate)
        };
    }
    static void from_json(const nlohmann::json& j, RallypointPeer& p)
    {
        p.clear();
        j.at("id").get_to(p.id);
        getOptional<bool>("enabled", p.enabled, j, true);
        getOptional<NetworkAddress>("host", p.host, j);
        getOptional<SecurityCertificate>("certificate", p.certificate, j);
    }

    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(RallypointServerStatusReport)
    /**
    * @brief TODO: Configuration for the Rallypoint status report file
    * 
    * Helper C++ class to serialize and de-serialize RallypointServerStatusReport JSON 
    * 
    * Example: @include[doc] examples/RallypointServerStatusReport.json 
    *    
    * @see RallypointServer  
    */       
    class RallypointServerStatusReport : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(RallypointServerStatusReport)

    public:
        /** File name to use for the Rally Point report. */
        std::string                     fileName;

        /** [Optional, Default: 30] The interval at which to write out the status report to file. */
        int                             intervalSecs;
        bool                            enabled;
        bool                            includeLinks;
        bool                            includePeerLinkDetails;
        bool                            includeClientLinkDetails;

        RallypointServerStatusReport()
        {
            clear();
        }

        void clear()
        {
            fileName.clear();
            intervalSecs = 60;
            enabled = false;
            includeLinks = false;
            includePeerLinkDetails = false;
            includeClientLinkDetails = false;
        }
    };
    
    static void to_json(nlohmann::json& j, const RallypointServerStatusReport& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(fileName),
            TOJSON_IMPL(intervalSecs),
            TOJSON_IMPL(enabled),
            TOJSON_IMPL(includeLinks),
            TOJSON_IMPL(includePeerLinkDetails),
            TOJSON_IMPL(includeClientLinkDetails)
        };
    }
    static void from_json(const nlohmann::json& j, RallypointServerStatusReport& p)
    {
        p.clear();
        getOptional<std::string>("fileName", p.fileName, j);
        getOptional<int>("intervalSecs", p.intervalSecs, j, 60);
        getOptional<bool>("enabled", p.enabled, j, false);
        getOptional<bool>("includeLinks", p.includeLinks, j, false);
        getOptional<bool>("includePeerLinkDetails", p.includePeerLinkDetails, j, false);
        getOptional<bool>("includeClientLinkDetails", p.includeClientLinkDetails, j, false);
    }    

    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(RallypointServerLinkGraph)
    class RallypointServerLinkGraph : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(RallypointServerLinkGraph)

    public:
        std::string                     fileName;
        int                             minRefreshSecs;
        bool                            enabled;
        bool                            includeDigraphEnclosure;
        bool                            includeClients;
        std::string                     coreRpStyling;
        std::string                     leafRpStyling;
        std::string                     clientStyling;

        RallypointServerLinkGraph()
        {
            clear();
        }

        void clear()
        {
            fileName.clear();
            minRefreshSecs = 5;
            enabled = false;
            includeDigraphEnclosure = true;
            includeClients = true;
            coreRpStyling = "[shape=hexagon color=firebrick style=filled]";
            leafRpStyling = "[shape=box color=gray style=filled]";
            clientStyling.clear();
        }
    };
    
    static void to_json(nlohmann::json& j, const RallypointServerLinkGraph& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(fileName),
            TOJSON_IMPL(minRefreshSecs),
            TOJSON_IMPL(enabled),
            TOJSON_IMPL(includeDigraphEnclosure),
            TOJSON_IMPL(includeClients),
            TOJSON_IMPL(coreRpStyling),
            TOJSON_IMPL(leafRpStyling),
            TOJSON_IMPL(clientStyling)
        };
    }
    static void from_json(const nlohmann::json& j, RallypointServerLinkGraph& p)
    {
        p.clear();
        getOptional<std::string>("fileName", p.fileName, j);
        getOptional<int>("minRefreshSecs", p.minRefreshSecs, j, 5);
        getOptional<bool>("enabled", p.enabled, j, false);
        getOptional<bool>("includeDigraphEnclosure", p.includeDigraphEnclosure, j, true);
        getOptional<bool>("includeClients", p.includeClients, j, true);
        getOptional<std::string>("coreRpStyling", p.coreRpStyling, j, "[shape=hexagon color=firebrick style=filled]");
        getOptional<std::string>("leafRpStyling", p.leafRpStyling, j, "[shape=box color=gray style=filled]");
        getOptional<std::string>("clientStyling", p.clientStyling, j);        
    }    

    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(RallypointExternalHealthCheckResponder)
    /**
    * @brief TODO: Configuration to enable external systems to use to check if the service is still running. TODO: Shaun, can you please expand. 
    * 
    * Helper C++ class to serialize and de-serialize RallypointExternalHealthCheckResponder JSON 
    * 
    * Example: @include[doc] examples/RallypointExternalHealthCheckResponder.json 
    *    
    * @see RallypointServer 
    */       
    class RallypointExternalHealthCheckResponder : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(RallypointExternalHealthCheckResponder)

    public:

        /** The network port to listen on for connections */
        int                             listenPort;

        /** TODO: Shaun, please complete */
        bool                            immediateClose;

        RallypointExternalHealthCheckResponder()
        {
            clear();
        }

        void clear()
        {
            listenPort = 0;
            immediateClose = true;
        }
    };
    
    static void to_json(nlohmann::json& j, const RallypointExternalHealthCheckResponder& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(listenPort),
            TOJSON_IMPL(immediateClose)
        };
    }
    static void from_json(const nlohmann::json& j, RallypointExternalHealthCheckResponder& p)
    {
        p.clear();
        getOptional<int>("listenPort", p.listenPort, j, 0);
        getOptional<bool>("immediateClose", p.immediateClose, j, true);
    }    


    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(Tls)
    /**
    * @brief TODO: Transport Security Layer (TLS) settings
    * TODO: Shaun, can you please complete ..
    * 
    * Helper C++ class to serialize and de-serialize Tls JSON 
    * 
    * Example: @include[doc] examples/Tls.json 
    *    
    * @see RallypointServer 
    */       
    class Tls : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(Tls)

    public:

        /** @brief TODO: */ 
        bool                        verifyPeers;

        /** @brief TODO: */ 
        bool                        allowSelfSignedCertificates;

        /** @brief TODO: */ 
        std::vector<std::string>    caCertificates;

        /** @brief TODO: */ 
        std::vector<std::string>    whitelistedSubjects;

        /** @brief TODO: */ 
        std::vector<std::string>    whitelistedIssuers;

        /** @brief TODO: */ 
        std::vector<std::string>    blacklistedSubjects;

        /** @brief TODO: */ 
        std::vector<std::string>    blacklistedIssuers;

        Tls()
        {
            clear();
        }

        void clear()
        {
            verifyPeers = false;
            allowSelfSignedCertificates = true;
            caCertificates.clear();
            whitelistedSubjects.clear();
            whitelistedIssuers.clear();
            blacklistedSubjects.clear();
            blacklistedIssuers.clear();
        }
    };
    
    static void to_json(nlohmann::json& j, const Tls& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(verifyPeers),
            TOJSON_IMPL(allowSelfSignedCertificates),
            TOJSON_IMPL(caCertificates),            
            TOJSON_IMPL(whitelistedSubjects),
            TOJSON_IMPL(whitelistedIssuers),
            TOJSON_IMPL(blacklistedSubjects),
            TOJSON_IMPL(blacklistedIssuers)
        };
    }
    static void from_json(const nlohmann::json& j, Tls& p)
    {
        p.clear();
        getOptional<bool>("verifyPeers", p.verifyPeers, j, false);
        getOptional<bool>("allowSelfSignedCertificates", p.allowSelfSignedCertificates, j, true);
        getOptional<std::vector<std::string>>("caCertificates", p.caCertificates, j);
        getOptional<std::vector<std::string>>("whitelistedSubjects", p.whitelistedSubjects, j);
        getOptional<std::vector<std::string>>("whitelistedIssuers", p.whitelistedIssuers, j);
        getOptional<std::vector<std::string>>("blacklistedSubjects", p.blacklistedSubjects, j);
        getOptional<std::vector<std::string>>("blacklistedIssuers", p.blacklistedIssuers, j);
    }    

    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(PeeringConfiguration)
    /**
    * @brief Configuration for Rallypoint peers 
    * 
    * Helper C++ class to serialize and de-serialize PeeringConfiguration JSON 
    *         
    * @see RallypointServer 
    */       
    class PeeringConfiguration : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(PeeringConfiguration)
        
    public:

        /** @brief TODO: Shaun, not sure what this Id is */
        std::string                     id;

        /** @brief TODO: Shaun, help */
        int                             version;

        /** @brief Comments */ 
        std::string                     comments;

        /** @brief X509 Certificate to use for peer connectivity. */
        SecurityCertificate             certificate;

        /** @brief List of Rallypoint peers to connect to */
        std::vector<RallypointPeer>     peers;

        /** @brief Transport Layer Security(TLS) settings for the peer connection. */
        Tls                             tls;

        PeeringConfiguration()
        {
            clear();
        }

        void clear()
        {
            id.clear();
            version = 0;
            comments.clear();
            certificate.clear();
            peers.clear();
            tls.clear();
        }
    };

    static void to_json(nlohmann::json& j, const PeeringConfiguration& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(id),
            TOJSON_IMPL(version),
            TOJSON_IMPL(comments),
            TOJSON_IMPL(certificate),
            TOJSON_IMPL(peers),
            TOJSON_IMPL(tls)
        };
    }
    static void from_json(const nlohmann::json& j, PeeringConfiguration& p)
    {
        p.clear();
        getOptional<std::string>("id", p.id, j);
        getOptional<int>("version", p.version, j, 0);
        getOptional<std::string>("comments", p.comments, j);
        getOptional<SecurityCertificate>("certificate", p.certificate, j);
        getOptional<std::vector<RallypointPeer>>("peers", p.peers, j);
        getOptional<Tls>("tls", p.tls, j);
    }

    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(IgmpSnooping)
    /**
    * @brief Configuration for IGMP snooping
    * 
    * Helper C++ class to serialize and de-serialize IgmpSnooping JSON 
    *         
    * @see RallypointServer 
    */       
    class IgmpSnooping : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(IgmpSnooping)
        
    public:

        /** @brief Enables IGMP.  Default is false. */
        bool                            enabled;

        /** @brief TODO */ 
        int                             queryIntervalMs;

        /** @brief TODO */ 
        int                             lastMemberQueryIntervalMs;

        /** @brief TODO */ 
        int                             lastMemberQueryCount;

        IgmpSnooping()
        {
            clear();
        }

        void clear()
        {
            enabled = false;
            queryIntervalMs = 60000;
            lastMemberQueryIntervalMs = 1000;
            lastMemberQueryCount = 1;
        }
    };

    static void to_json(nlohmann::json& j, const IgmpSnooping& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(enabled),
            TOJSON_IMPL(queryIntervalMs),
            TOJSON_IMPL(lastMemberQueryIntervalMs),
            TOJSON_IMPL(lastMemberQueryCount)
        };
    }
    static void from_json(const nlohmann::json& j, IgmpSnooping& p)
    {
        p.clear();
        getOptional<bool>("enabled", p.enabled, j);
        getOptional<int>("queryIntervalMs", p.queryIntervalMs, j, 60000);
        getOptional<int>("lastMemberQueryIntervalMs", p.lastMemberQueryIntervalMs, j, 1000);
        getOptional<int>("lastMemberQueryCount", p.lastMemberQueryCount, j, 1);
    }

    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(RallypointServer)
    /**
    * @brief Configuration for the Rallypoint server
    * 
    * TODO: Shaun, can you please document this?
    * 
    * Helper C++ class to serialize and de-serialize RallypointServer JSON 
    * 
    * Example: @include[doc] examples/RallypointServer.json 
    *     
    */       
    class RallypointServer : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(RallypointServer)
        
    public:

        /** @brief A unqiue identifier for the Rallpoint */
        std::string                                 id;

        /** @brief TCP port to listen on.  Default is 7443. */
        int                                         listenPort;

        /** @brief Name of the NIC to bind to for listening for incoming TCP connections. */
        std::string                                 interfaceName;

        /** @brief Indicate whether FIPS140-2 mode is required for security. */
        bool                                        requireFips;

        /** @brief X.509 certificate and private key that identifies the Rallypoint.  @see SecurityCertificate*/
        SecurityCertificate                         certificate;

        /** @brief Name of a file containing a JSON array of Rallypoint peers to connect to. */
        std::string                                 peeringConfigurationFileName;

        /** @brief Number of seconds between checks to see if the peering configuration has been updated.  Default is 60.*/
        int                                         peeringConfigurationFileCheckSecs;

        /** @brief Allows traffic received on unicast links to be forwarded to the multicast network. */
        bool                                        allowMulticastForwarding;

        /** @brief Number of threading pools to create for network I/O.  Default is -1 which creates 1 I/O pool per CPU core. */
        int                                         ioPools;

        /** @brief Details for producing a status report. @see RallypointServerStatusReport */
        RallypointServerStatusReport                statusReport;

        /** @brief Details for producing a Graphviz-compatible link graph. @see RallypointServerLinkGraph */
        RallypointServerLinkGraph                   linkGraph;

        /** @brief Details concerning the Rallypoint's interaction with an external heal-checker such as a load-balancer.  @see RallypointExternalHealthCheckResponder */
        RallypointExternalHealthCheckResponder      externalHealthCheckResponder;

        /** @brief Set to true to allow forwarding of packets received from other Rallypoints to all other Rallypoints.  *WARNING* Be exceptionally careful when enabling this capability! */
        bool                                        allowPeerForwarding;

        /** @brief The name of the NIC on which to send and receive multicast traffic. */
        std::string                                 multicastInterfaceName;

        /** @brief Details concerning Transport Layer Security.  @see Tls */
        Tls                                         tls;

        /** @brief Details discovery capabilities.  @see DiscoveryConfiguration */
        DiscoveryConfiguration                      discovery;

        /** @brief Enables automatic forwarding of discovered multicast traffic to peer Rallypoints. */
        bool                                        forwardDiscoveredGroups;

        /** @brief Internal - not serialized. */
        PeeringConfiguration                        peeringConfiguration;       // NOTE: This is NOT serialized

        /** @brief Indicates whether this Rallypoint is part of a core mesh or hangs off the periphery as a leaf node. */
        bool                                        isMeshLeaf;

        /** @brief Disables the watchdog logic that protects against a hung process becoming non-functional.  Only use for troubleshooting purposes. */
        bool                                        disableWatchdog;

        /** @brief The watchdog's check interval in milliseconds.  Only use for troubleshooting purposes. */
        int                                         watchdogIntervalMs;

        /** @brief The watchdog's hung process timeout in milliseconds.  Only use for troubleshooting purposes. */
        int                                         watchdogHangDetectionMs;

        /** @brief Set to true to forgo DSA signing of messages.  Doing so is is a security risk but can be useful on CPU-constrained systems on already-secure environments. */
        bool                                        disableMessageSigning;

        /** @brief Indicates whether inbound peer registrations should be reciprocated.  Only applicable for a mesh leaf that has multicat forwarding enabled. */
        bool                                        reciprocateRegistrations;

        /** @brief Vector of multicasts that may be reflected. */
        std::vector<NetworkAddressRxTx>             multicastWhitelist;

        /** @brief Vector of multicasts that may not be reflected. */
        std::vector<NetworkAddressRxTx>             multicastBlacklist;

        /** @brief IGMP snooping configuration. */
        IgmpSnooping                                igmpSnooping;

        /** @brief Vector of pre-configured multicasts that must always be reflected. */
        std::vector<NetworkAddressRxTx>             requiredMulticasts;

        RallypointServer()
        {
            clear();
        }

        void clear()
        {
            id.clear();
            listenPort = 7443;
            interfaceName.clear();
            requireFips = false;
            certificate.clear();
            allowMulticastForwarding = false;
            peeringConfiguration.clear();
            peeringConfigurationFileName.clear();
            peeringConfigurationFileCheckSecs = 60;
            ioPools = -1;
            statusReport.clear();
            linkGraph.clear();
            externalHealthCheckResponder.clear();
            allowPeerForwarding = false;
            multicastInterfaceName.clear();
            tls.clear();
            discovery.clear();
            forwardDiscoveredGroups = false;
            isMeshLeaf = false;
            disableWatchdog = false;
            watchdogIntervalMs = 5000;
            watchdogHangDetectionMs = 2000;
            disableMessageSigning = false;
            reciprocateRegistrations = false;
            multicastWhitelist.clear();
            multicastBlacklist.clear();
            igmpSnooping.clear();
            requiredMulticasts.clear();
        }
    };
    
    static void to_json(nlohmann::json& j, const RallypointServer& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(id),
            TOJSON_IMPL(listenPort),
            TOJSON_IMPL(interfaceName),
            TOJSON_IMPL(requireFips),
            TOJSON_IMPL(certificate),
            TOJSON_IMPL(allowMulticastForwarding),
            // TOJSON_IMPL(peeringConfiguration),               // NOTE: Not serialized!
            TOJSON_IMPL(peeringConfigurationFileName),
            TOJSON_IMPL(peeringConfigurationFileCheckSecs),
            TOJSON_IMPL(ioPools),
            TOJSON_IMPL(statusReport),
            TOJSON_IMPL(linkGraph),
            TOJSON_IMPL(externalHealthCheckResponder),
            TOJSON_IMPL(allowPeerForwarding),
            TOJSON_IMPL(multicastInterfaceName),
            TOJSON_IMPL(tls),
            TOJSON_IMPL(discovery),
            TOJSON_IMPL(forwardDiscoveredGroups),
            TOJSON_IMPL(isMeshLeaf),
            TOJSON_IMPL(disableWatchdog),
            TOJSON_IMPL(watchdogIntervalMs),
            TOJSON_IMPL(watchdogHangDetectionMs),
            TOJSON_IMPL(disableMessageSigning),
            TOJSON_IMPL(reciprocateRegistrations),
            TOJSON_IMPL(multicastWhitelist),
            TOJSON_IMPL(multicastBlacklist),
            TOJSON_IMPL(multicastBlacklist),
            TOJSON_IMPL(igmpSnooping),
            TOJSON_IMPL(requiredMulticasts)
        };
    }
    static void from_json(const nlohmann::json& j, RallypointServer& p)
    {
        p.clear();
        getOptional<std::string>("id", p.id, j);
        j.at("certificate").get_to(p.certificate);
        getOptional<bool>("requireFips", p.requireFips, j, false);
        getOptional<std::string>("interfaceName", p.interfaceName, j);
        getOptional<int>("listenPort", p.listenPort, j, 7443);
        getOptional<bool>("allowMulticastForwarding", p.allowMulticastForwarding, j, false);
        //getOptional<PeeringConfiguration>("peeringConfiguration", p.peeringConfiguration, j);         // NOTE: Not serialized!    
        getOptional<std::string>("peeringConfigurationFileName", p.peeringConfigurationFileName, j);
        getOptional<int>("peeringConfigurationFileCheckSecs", p.peeringConfigurationFileCheckSecs, j, 60);
        getOptional<int>("ioPools", p.ioPools, j, -1);
        getOptional<RallypointServerStatusReport>("statusReport", p.statusReport, j);
        getOptional<RallypointServerLinkGraph>("linkGraph", p.linkGraph, j);
        getOptional<RallypointExternalHealthCheckResponder>("externalHealthCheckResponder", p.externalHealthCheckResponder, j);
        getOptional<bool>("allowPeerForwarding", p.allowPeerForwarding, j, false);        
        getOptional<std::string>("multicastInterfaceName", p.multicastInterfaceName, j);
        getOptional<Tls>("clientTls", p.tls, j);
        getOptional<DiscoveryConfiguration>("discovery", p.discovery, j);   
        getOptional<bool>("forwardDiscoveredGroups", p.forwardDiscoveredGroups, j, false);
        getOptional<bool>("isMeshLeaf", p.isMeshLeaf, j, false);
        getOptional<bool>("disableWatchdog", p.disableWatchdog, j, false);        
        getOptional<int>("watchdogIntervalMs", p.watchdogIntervalMs, j, 5000);
        getOptional<int>("watchdogHangDetectionMs", p.watchdogHangDetectionMs, j, 2000);
        getOptional<bool>("disableMessageSigning", p.disableMessageSigning, j, false);
        getOptional<bool>("reciprocateRegistrations", p.reciprocateRegistrations, j, false);
        getOptional<std::vector<NetworkAddressRxTx>>("multicastWhitelist", p.multicastWhitelist, j);
        getOptional<std::vector<NetworkAddressRxTx>>("multicastBlacklist", p.multicastBlacklist, j);
        getOptional<IgmpSnooping>("igmpSnooping", p.igmpSnooping, j);
        getOptional<std::vector<NetworkAddressRxTx>>("requiredMulticasts", p.requiredMulticasts, j);
    }    

    
    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(PlatformDiscoveredService)
    /**
    * @brief TODO: Shaun, not sure what this is 
    * 
    * Helper C++ class to serialize and de-serialize PlatformDiscoveredService JSON 
    * 
    * Example: @include[doc] examples/PlatformDiscoveredService.json 
    *    
    * @see TODO: Add references 
    */       
    class PlatformDiscoveredService : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(PlatformDiscoveredService)
        
    public:

        /** @brief TODO: */
        std::string                             id;

        /** @brief TODO: */
        std::string                             type;

        /** @brief TODO: */
        std::string                             name;

        /** @brief TODO: */
        NetworkAddress                          address;

        PlatformDiscoveredService()
        {
            clear();
        }

        void clear()
        {
            id.clear();
            type.clear();
            name.clear();
            address.clear();
        }
    };

    static void to_json(nlohmann::json& j, const PlatformDiscoveredService& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(id),
            TOJSON_IMPL(type),
            TOJSON_IMPL(name),
            TOJSON_IMPL(address)
        };
    }
    static void from_json(const nlohmann::json& j, PlatformDiscoveredService& p)
    {
        p.clear();
        getOptional<std::string>("id", p.id, j);
        getOptional<std::string>("type", p.type, j);
        getOptional<std::string>("name", p.name, j);
        getOptional<NetworkAddress>("address", p.address, j);
    }

    //-----------------------------------------------------------
    class TimelineEvent
    {
    public:
        typedef enum
        {
            etUndefined           = 0,
            etAudio               = 1,
            etLocation            = 2
        } EventType_t;

        typedef enum 
        {
            dNone        = 0,
            dInbound     = 1,
            dOutbound    = 2,
            dBoth        = 3,
            dUndefined   = 4,
        } Direction_t;
    };    


    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(TimelineQueryParameters)
    /**
    * @brief TODO: Shaun, can you please document this class  
    * 
    * Helper C++ class to serialize and de-serialize TimelineQueryParameters JSON 
    * 
    * Example: @include[doc] examples/TimelineQueryParameters.json 
    *    
    * @see TODO: Add references 
    */       
    class TimelineQueryParameters : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(TimelineQueryParameters)
        
    public:

        /** @brief TODO: */
        long                    maxCount;

        /** @brief TODO: */
        bool                    mostRecentFirst;

        /** @brief TODO: */
        uint64_t                startedOnOrAfter;

        /** @brief TODO: */
        uint64_t                endedOnOrBefore;

        /** @brief TODO: */
        int                     onlyDirection;

        /** @brief TODO: */
        int                     onlyType;

        /** @brief TODO: */
        bool                    onlyCommitted;

        /** @brief TODO: */
        std::string             onlyAlias;

        /** @brief TODO: */
        std::string             onlyNodeId;
        std::string             sql;

        TimelineQueryParameters()
        {
            clear();
        }

        void clear()
        {
            maxCount = 50;
            mostRecentFirst = true;
            startedOnOrAfter = 0;
            endedOnOrBefore = 0;
            onlyDirection = 0;
            onlyType = 0;
            onlyCommitted = true;
            onlyAlias.clear();
            onlyNodeId.clear();
            sql.clear();
        }
    };

    static void to_json(nlohmann::json& j, const TimelineQueryParameters& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(maxCount),
            TOJSON_IMPL(mostRecentFirst),
            TOJSON_IMPL(startedOnOrAfter),
            TOJSON_IMPL(endedOnOrBefore),
            TOJSON_IMPL(onlyDirection),
            TOJSON_IMPL(onlyType),
            TOJSON_IMPL(onlyCommitted),
            TOJSON_IMPL(onlyAlias),
            TOJSON_IMPL(onlyNodeId),
            TOJSON_IMPL(sql)
        };
    }
    static void from_json(const nlohmann::json& j, TimelineQueryParameters& p)
    {
        p.clear();
        getOptional<long>("maxCount", p.maxCount, j, 50);
        getOptional<bool>("mostRecentFirst", p.mostRecentFirst, j, false);
        getOptional<uint64_t>("startedOnOrAfter", p.startedOnOrAfter, j, 0);
        getOptional<uint64_t>("endedOnOrBefore", p.endedOnOrBefore, j, 0);
        getOptional<int>("onlyDirection", p.onlyDirection, j, 0);
        getOptional<int>("onlyType", p.onlyType, j, 0);
        getOptional<bool>("onlyCommitted", p.onlyCommitted, j, true);
        getOptional<std::string>("onlyAlias", p.onlyAlias, j, EMPTY_STRING);        
        getOptional<std::string>("onlyNodeId", p.onlyNodeId, j, EMPTY_STRING);
        getOptional<std::string>("sql", p.sql, j, EMPTY_STRING);
    }

    //-----------------------------------------------------------    
    static inline void dumpExampleConfigurations(const char *path)
    {
        RtpHeader::document(path);
        BlobInfo::document(path);
        AdvancedTxParams::document(path);
        Identity::document(path);
        Location::document(path);
        Power::document(path);
        Connectivity::document(path);
        GroupAlias::document(path);
        PresenceDescriptor::document(path);
        NetworkTxOptions::document(path);
        NetworkAddress::document(path);
        NetworkAddressRxTx::document(path);
        IgmpSnooping::document(path);
        Rallypoint::document(path);
        TxAudio::document(path);
        AudioDeviceDescriptor::document(path);
        Audio::document(path);
        TalkerInformation::document(path);
        GroupTalkers::document(path);
        Presence::document(path);
        Advertising::document(path);
        GroupTimeline::document(path);
        Group::document(path);
        Mission::document(path);
        LicenseDescriptor::document(path);
        EnginePolicyNetworking::document(path);
        EnginePolicyAudio::document(path);
        SecurityCertificate::document(path);
        EnginePolicySecurity::document(path);
        EnginePolicyLogging::document(path);
        EnginePolicyLicensing::document(path);
        DiscoverySsdp::document(path);
        DiscoverySap::document(path);
        DiscoveryCistech::document(path);
        DiscoveryTrellisware::document(path);
        DiscoveryConfiguration::document(path);
        EnginePolicyInternals::document(path);
        EnginePolicyTimelines::document(path);
        EnginePolicy::document(path);
        TalkgroupAsset::document(path);
        EngageDiscoveredGroup::document(path);
        PeeringConfiguration::document(path);
        RallypointPeer::document(path);
        RallypointServerStatusReport::document(path);
        RallypointExternalHealthCheckResponder::document(path);
        Tls::document(path);
        RallypointServer::document(path);
        PlatformDiscoveredService::document(path);
        TimelineQueryParameters::document(path);
    }
}

#ifndef WIN32    
    #pragma GCC diagnostic pop
#endif

#endif /* ConfigurationObjects_h */
