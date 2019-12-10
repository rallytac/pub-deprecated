//
//  Copyright (c) 2019 Rally Tactical Systems, Inc.
//  All rights reserved.
//

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

namespace ConfigurationObjects
{

    //-----------------------------------------------------------
    #pragma pack(push, 1)
        typedef struct
        {
            uint8_t     t;          // Type
            uint32_t    ts;         // Timestamp
            uint8_t     it;         // Increment type
            uint8_t     im;         // Increment multiplier
            uint8_t     vt;         // Value type
            uint8_t     ss;         // Series size (element count)
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
    class RtpHeader : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(RtpHeader)
        
    public:
        int         pt;
        bool        marker;
        uint16_t    seq;
        uint32_t    ssrc;
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
    class BlobInfo : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(BlobInfo)
        
    public:
        typedef enum
        {
            bptUndefined                    = 0,
            bptAppTextUtf8                  = 1,
            bptJsonTextUtf8                 = 2,
            bptAppBinary                    = 3,
            bptEngageBinaryHumanBiometrics  = 4
        } PayloadType_t;

        size_t          size;
        std::string     source;
        std::string     target;
        PayloadType_t   payloadType;
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
    class AdvancedTxParams : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(AdvancedTxParams)
        
    public:
        uint16_t        flags;
        uint8_t         priority;
        uint16_t        subchannelTag;
        bool            includeNodeId;
        std::string     alias;

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
            TOJSON_IMPL(alias)
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
    }

    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(Identity)
    class Identity : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(Identity)
        
    public:
        std::string     nodeId;
        std::string     userId;
        std::string     displayName;
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
    class Location : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(Location)

    public:
        constexpr static double INVALID_LOCATION_VALUE = -999.999;

        uint32_t                    ts;
        double                      latitude;
        double                      longitude;
        double                      altitude;
        double                      direction;
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
    class Power : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(Power)

    public:
        int     source;
        int     state;
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
    class Connectivity : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(Connectivity)

    public:
        int     type;
        int     strength;
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
    class GroupAlias : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(GroupAlias)

    public:
        std::string     groupId;
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
    class PresenceDescriptor : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(PresenceDescriptor)
        
    public:
        bool                        self;
        uint32_t                    ts;
        uint32_t                    nextUpdate;
        Identity                    identity;
        std::string                 comment;

        uint32_t                    disposition;

        // Emergency            = 0x00000001
        // Available            = 0x00000002
        // Busy                 = 0x00000004
        // etc, etc

        
        std::vector<GroupAlias>     groupAliases;
        Location                    location;
        std::string                 custom;
        bool                        announceOnReceive;
        Connectivity                connectivity;
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
    class NetworkTxOptions : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(NetworkTxOptions)
        
    public:
        typedef enum
        {
            priBestEffort   = 0,    // NET_SERVICE_TYPE_BE

            priSignaling    = 2,    // NET_SERVICE_TYPE_SIG
            priVideo        = 3,    // NET_SERVICE_TYPE_VI
            priVoice        = 4     // NET_SERVICE_TYPE_VO
        } TxPriority_t;

        TxPriority_t    priority;
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
    class NetworkAddress : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(NetworkAddress)
        
    public:
        std::string     address;
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
        j.at("address").get_to(p.address);
        j.at("port").get_to(p.port);
    }



    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(Rallypoint)
    class Rallypoint : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(Rallypoint)
        
    public:
        NetworkAddress              host;
        std::string                 certificate;
        std::string                 certificateKey;
        bool                        verifyPeer;
        bool                        allowSelfSignedCertificate;        
        std::vector<std::string>    caCertificates;
        int                         transactionTimeoutMs;

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
            TOJSON_IMPL(transactionTimeoutMs)
        };
    }
    static void from_json(const nlohmann::json& j, Rallypoint& p)
    {
        p.clear();
        j.at("host").get_to(p.host);
        j.at("certificate").get_to(p.certificate);
        j.at("certificateKey").get_to(p.certificateKey);
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
    }

    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(TxAudio)
    class TxAudio : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(TxAudio)
        
    public:
        typedef enum
        {
            ctUnknown       = 0,

            ctG711ulaw      = 1,
            ctG711alaw      = 2,

            ctGsm610        = 3,

            ctAmrNb4750     = 10,
            ctAmrNb5150     = 11,
            ctAmrNb5900     = 12,
            ctAmrNb6700     = 13,
            ctAmrNb7400     = 14,
            ctAmrNb7950     = 15,
            ctAmrNb10200    = 16,
            ctAmrNb12200    = 17,

            ctOpus6000      = 20,
            ctOpus8000      = 21,
            ctOpus10000     = 22,
            ctOpus12000     = 23,
            ctOpus14000     = 24,
            ctOpus16000     = 25,
            ctOpus18000     = 26,
            ctOpus20000     = 27,
            ctOpus22000     = 28,
            ctOpus24000     = 29
        } TxCodec_t;

        TxCodec_t       encoder;
        int             framingMs;
        bool            fdx;
        bool            noHdrExt;
        int             maxTxSecs;
        int             extensionSendInterval;
        bool            debug;
        int             userTxPriority;
        int             userTxFlags;
        int             initialHeaderBurst;
        int             trailingHeaderBurst;


        TxAudio()
        {
            clear();
        }

        void clear()
        {
            encoder = ctUnknown;
            framingMs = 20;
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
    class AudioDeviceDescriptor : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(AudioDeviceDescriptor)
        
    public:
        typedef enum 
        {
            dirUnknown = 0, 
            dirInput, 
            dirOutput
        } Direction_t;

        int             deviceId;
        int             samplingRate;
        int             channels;
        Direction_t     direction;
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
    class Audio : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(Audio)
        
    public:
        int     inputId;
        int     inputGain;
        int     outputId;
        int     outputGain;
        int     outputLevelLeft;
        int     outputLevelRight;

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
        }
    };
    
    static void to_json(nlohmann::json& j, const Audio& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(inputId),
            TOJSON_IMPL(inputGain),
            TOJSON_IMPL(outputId),
            TOJSON_IMPL(outputLevelLeft),
            TOJSON_IMPL(outputLevelRight)
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
    }
    
    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(TalkerInformation)
    class TalkerInformation : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(TalkerInformation)
        
    public:
        std::string alias;
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
    class GroupTalkers : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(GroupTalkers)
        
    public:
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
    class Presence : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(Presence)
        
    public:
        typedef enum
        {
            pfUnknown       = 0,

            pfEngage        = 1,
            pfCot           = 2
        } Format_t;

        Format_t        format;
        int             intervalSecs;
        bool            forceOnAudioTransmit;
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
    class Advertising : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(Advertising)
        
    public:
        bool        enabled;
        int         intervalMs;
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
    class GroupTimeline : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(GroupTimeline)
        
    public:
        bool        enabled;
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
    JSON_SERIALIZED_CLASS(Group)
    class Group : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(Group)
        
    public:
        typedef enum 
        {   
            gtUnknown, 
            gtAudio, 
            gtPresence,
            gtRaw
        } Type_t;

        Type_t                                  type;
        std::string                             id;
        std::string                             name;
        std::string                             interfaceName;
        NetworkAddress                          rx;
        NetworkAddress                          tx;
        NetworkTxOptions                        txOptions;
        TxAudio                                 txAudio;
        Presence                                presence;
        std::string                             cryptoPassword;
        bool                                    debugAudio;
        std::vector<Rallypoint>                 rallypoints;
        Audio                                   audio;
        GroupTimeline                           timeline;

        std::string                             alias;
        bool                                    blockAdvertising;

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
            TOJSON_IMPL(blockAdvertising)
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
    class LicenseDescriptor : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(LicenseDescriptor)
        
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

        std::string                             entitlement;
        std::string                             key;
        std::string                             activationCode;
        std::string                             deviceId;
        int                                     type;
        time_t                                  expires;                // Unix timestamp - Zulu/UTC 
        std::string                             expiresFormatted;       // ISO 8601 format, Zulu/UTC
        int                                     flags;
        std::string                             refreshUri;
        std::string                             cargo;
        int                                     refreshIntervalDays;
        int                                     status;

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
            TOJSON_IMPL(status)
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
    }


    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(EnginePolicyNetworking)
    class EnginePolicyNetworking : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(EnginePolicyNetworking)
        
    public:
        std::string         defaultNic;
        int                 maxOutputQueuePackets;
        int                 rtpJitterMinMs;
        int                 rtpJitterMaxFactor;
        int                 rtpJitterOverflowTrimPercentage;
        int                 rtpJitterUnderrunReductionThresholdMs;
        int                 rtpLatePacketSequenceRange;
        int                 rtpLatePacketTimestampRangeMs;
        int                 rtpInboundProcessorInactivityMs;
        int                 multicastRejoinSecs;
        int                 rpLeafConnectTimeoutSecs;
        int                 maxReconnectPauseMs;
        int                 reconnectFailurePauseIncrementMs;
        int                 sendFailurePauseMs;
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
        int                 outputChannels;
        int                 outputRate;

        int                 outputGainPercentage;
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
    class SecurityCertificate : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(SecurityCertificate)
        
    public:
        std::string         certificate;
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

    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(EnginePolicySecurity)
    class EnginePolicySecurity : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(EnginePolicySecurity)
        
    public:
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
    class EnginePolicyLogging : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(EnginePolicyLogging)
        
    public:
        int     maxLevel;
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
    class EnginePolicyLicensing : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(EnginePolicyLicensing)
        
    public:
        std::string         entitlement;
        std::string         key;
        std::string         activationCode;
        std::string         deviceId;

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
        }
    };

    static void to_json(nlohmann::json& j, const EnginePolicyLicensing& p)
    {
        j = nlohmann::json{
            TOJSON_IMPL(entitlement),
            TOJSON_IMPL(key),
            TOJSON_IMPL(activationCode),
            TOJSON_IMPL(deviceId)
        };
    }
    static void from_json(const nlohmann::json& j, EnginePolicyLicensing& p)
    {
        p.clear();
        FROMJSON_IMPL(entitlement, std::string, EMPTY_STRING);
        FROMJSON_IMPL(key, std::string, EMPTY_STRING);
        FROMJSON_IMPL(activationCode, std::string, EMPTY_STRING);
        FROMJSON_IMPL(deviceId, std::string, EMPTY_STRING);
    }           

    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(DiscoverySsdp)
    class DiscoverySsdp : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(DiscoverySsdp)
        
    public:
        bool                                    enabled;
        std::string                             interfaceName;
        NetworkAddress                          address;
        std::vector<std::string>                searchTerms;
        int                                     ageTimeoutMs;
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
    class DiscoverySap : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(DiscoverySap)
        
    public:
        bool                                    enabled;
        std::string                             interfaceName;
        NetworkAddress                          address;
        int                                     ageTimeoutMs;
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
    class DiscoveryTrellisware : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(DiscoveryTrellisware)
        
    public:
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
    class DiscoveryConfiguration : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(DiscoveryConfiguration)
        
    public:
        DiscoverySsdp                           ssdp;
        DiscoverySap                            sap;
        DiscoveryCistech                        cistech;
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
    class EnginePolicyInternals : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(EnginePolicyInternals)
        
    public:
        bool                disableWatchdog;
        int                 watchdogIntervalMs;
        int                 watchdogHangDetectionMs;
        int                 housekeeperIntervalMs;
        int                 maxTxSecs;
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
        getOptional<bool>("enableLazySpeakerClosure", p.enableLazySpeakerClosure, j, false);
        
    }

    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(EnginePolicyTimelines)
    class EnginePolicyTimelines : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(EnginePolicyTimelines)
        
    public:
        bool                                    enabled;
        std::string                             storageRoot;
        int                                     maxStorageMb;
        long                                    maxEventAgeSecs;
        int                                     maxEvents;
        long                                    groomingIntervalSecs;
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
    class EnginePolicy : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(EnginePolicy)
        
    public:
        std::string                 dataDirectory;
        EnginePolicyLicensing       licensing;
        EnginePolicySecurity        security;
        EnginePolicyNetworking      networking;
        EnginePolicyAudio           audio;
        DiscoveryConfiguration      discovery;
        EnginePolicyLogging         logging;
        EnginePolicyInternals       internals;
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
    class TalkgroupAsset : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(TalkgroupAsset)
        
    public:
        std::string     nodeId;
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
    class RallypointServerStatusReport : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(RallypointServerStatusReport)

    public:
        std::string                     fileName;
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
    class RallypointExternalHealthCheckResponder : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(RallypointExternalHealthCheckResponder)

    public:
        int                             listenPort;
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
    class Tls : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(Tls)

    public:
        bool                        verifyPeers;
        bool                        allowSelfSignedCertificates;
        std::vector<std::string>    caCertificates;
        std::vector<std::string>    whitelistedSubjects;
        std::vector<std::string>    whitelistedIssuers;
        std::vector<std::string>    blacklistedSubjects;
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
    class PeeringConfiguration : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(PeeringConfiguration)
        
    public:
        std::string                     id;
        int                             version;
        std::string                     comments;
        SecurityCertificate             certificate;
        std::vector<RallypointPeer>     peers;
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
    JSON_SERIALIZED_CLASS(RallypointServer)
    class RallypointServer : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(RallypointServer)
        
    public:
        std::string                                 id;
        int                                         listenPort;
        std::string                                 interfaceName;
        bool                                        requireFips;
        SecurityCertificate                         certificate;
        std::string                                 peeringConfigurationFileName;
        int                                         peeringConfigurationFileCheckSecs;
        bool                                        allowMulticastForwarding;
        int                                         ioPools;
        uint64_t                                    msdToken;
        RallypointServerStatusReport                statusReport;
        RallypointServerLinkGraph                   linkGraph;
        RallypointExternalHealthCheckResponder      externalHealthCheckResponder;
        bool                                        allowPeerForwarding;
        std::string                                 multicastInterfaceName;
        Tls                                         tls;
        DiscoveryConfiguration                      discovery;
        bool                                        forwardDiscoveredGroups;

        PeeringConfiguration                        peeringConfiguration;       // NOTE: This is NOT serialized

        bool                                        isMeshLeaf;

        bool                                        disableWatchdog;
        int                                         watchdogIntervalMs;
        int                                         watchdogHangDetectionMs;

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
            msdToken = 0;
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
            TOJSON_IMPL(msdToken),
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
            TOJSON_IMPL(watchdogHangDetectionMs)
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
        getOptional<uint64_t>("msdToken", p.msdToken, j, 0);
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
    }    

    
    //-----------------------------------------------------------
    JSON_SERIALIZED_CLASS(PlatformDiscoveredService)
    class PlatformDiscoveredService : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(PlatformDiscoveredService)
        
    public:
        std::string                             id;
        std::string                             type;
        std::string                             name;
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
    class TimelineQueryParameters : public ConfigurationObjectBase
    {
        IMPLEMENT_JSON_SERIALIZATION()
        IMPLEMENT_JSON_DOCUMENTATION(TimelineQueryParameters)
        
    public:
        long                    maxCount;
        bool                    mostRecentFirst;
        uint64_t                startedOnOrAfter;
        uint64_t                endedOnOrBefore;
        int                     onlyDirection;
        int                     onlyType;
        bool                    onlyCommitted;
        std::string             onlyAlias;
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
