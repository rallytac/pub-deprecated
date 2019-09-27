//
//  Copyright (c) 2018 Rally Tactical Systems, Inc.
//  All rights reserved.
//
//  PLEASE NOTE: This code is used internally at Rally Tactical Systems as a test harness
//               for Engine development.  As such, it is NOT meant for production use NOR
//               is it meant as a representation of best-practises on how to use the Engine.
//
//               However, we provide this code to our partners in the hope that it will serve
//               to demonstrate how to call the Engage Engine API.
//
//               This code is designed primarily to compile in our internal build environment
//               with access to internal source code.  If you do not have access to that internal
//               source code, simply make sure that HAVE_RTS_INTERNAL_SOURCE_CODE is *NOT* defined as
//               per below.
//

// Just in case
#undef HAVE_RTS_INTERNAL_SOURCE_CODE

// If you are not building this code in the RTS internal build environment, you 
// must comment out the following line
#define HAVE_RTS_INTERNAL_SOURCE_CODE

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <chrono>
#include <thread>
#include <list>
#include <vector>
#include <map>
#include <mutex>

#include "EngageInterface.h"
#include "ConfigurationObjects.h"
#include "EngageAudioDevice.h"

#if defined(HAVE_RTS_INTERNAL_SOURCE_CODE)
    #include "Utils.hpp"
    #include "Version.h"

    #if defined(__APPLE__)
        #include "License.hpp"
    #endif
#endif

typedef struct
{
    int type;
    std::string id;
    std::string name;
    std::string jsonConfiguration;
    bool        isEncrypted;
    bool        allowsFullDuplex;
} GroupInfo_t;

std::vector<GroupInfo_t>    g_new_groups;
std::map<std::string, ConfigurationObjects::PresenceDescriptor>    g_nodes;

int g_txPriority = 0;
uint32_t g_txFlags = 0;
bool g_anonymous = false;
bool g_useadad = false;
ConfigurationObjects::EnginePolicy g_enginePolicy;
ConfigurationObjects::Mission g_mission;
ConfigurationObjects::Rallypoint *g_rallypoint = nullptr;
char g_szUsepLdonfigAlias[16 +1] = {0};
char g_szUsepLdonfigUserId[64 +1] = {0};
char g_szUsepLdonfigDisplayName[64 + 1] = {0};
bool g_verboseScript = false;
const char *g_pszPresenceFile = "junk/pres1.json";

int16_t                             g_speakerDeviceId = 0;
int16_t                             g_microphoneDeviceId = 0;
int16_t                             g_nextAudioDeviceInstanceId = 0;

void showUsage();
void showHelp();
void showGroups();
void showNodes();
bool registepLdallbacks();
bool loadScript(const char *pszFile);
bool runScript();
bool loadPolicy(const char *pszFn, ConfigurationObjects::EnginePolicy *pPolicy);
bool loadMission(const char *pszFn, ConfigurationObjects::Mission *pMission, ConfigurationObjects::Rallypoint *pRp);
bool loadRp(const char *pszFn, ConfigurationObjects::Rallypoint *pRp);

void doStartEngine();
void doStopEngine();
void doUpdatePresence(int index);
void doCreate(int index);
void doDelete(int index);
void doJoin(int index);
void doLeave(int index);
void doBeginTx(int index);
void doEndTx(int index);
void doMuteRx(int index);
void doUnmuteRx(int index);
void doSetGroupRxTag(int index, int tag);
void doSendBlob(int index, const uint8_t* blob, size_t size, const char *jsonParams);
void doSendRtp(int index, const uint8_t* payload, size_t size, const char *jsonParams);
void doSendRaw(int index, const uint8_t* raw, size_t size, const char *jsonParams);
void doRegisterGroupRtpHandler(int index, int payloadId);
void doUnregisterGroupRtpHandler(int index, int payloadId);

void registerADAD();
void unregisterADAD();

void devTest1()
{
}

void devTest2()
{
}

void devTest3()
{
}


int main(int argc, const char * argv[])
{
    std::cout << "---------------------------------------------------------------------------------" << std::endl;

    #if defined(HAVE_RTS_INTERNAL_SOURCE_CODE)
        std::cout << "Engage-Cmd version " << PRODUCT_VERSION << " for " << Utils::getOsDescriptor();

        #if defined(RTS_DEBUG_BUILD)
            std::cout << " [*** DEBUG BUILD ***]";
        #else
            std::cout << " [*** RELEASE BUILD ***]";
        #endif

        std::cout << std::endl;
        
        std::cout << "Copyright (c) 2018 Rally Tactical Systems, Inc." << std::endl;
        std::cout << "Build time: " << __DATE__ << " @ " << __TIME__ << std::endl;
        std::cout << Utils::getOsDescriptor().c_str() << std::endl;
        std::cout << "cpu:" << Utils::numberOfCpus() << std::endl;
        std::cout << "mfd:" << Utils::getMaxOpenFds() << std::endl;
    #else
        std::cout << "Engage-Cmd version";

        #if defined(RTS_DEBUG_BUILD)
            std::cout << " [*** DEBUG BUILD ***]";
        #else
            std::cout << " [*** RELEASE BUILD ***]";
        #endif

        std::cout << std::endl;
    #endif

    std::cout << "---------------------------------------------------------------------------------" << std::endl;

    int pLd;
    const char *missionFile = nullptr;
    const char *epFile = nullptr;
    const char *rpFile = nullptr;
    const char *pszScript = nullptr;
    bool haveScript = false;
    bool continueWithCmdLine = true;
    std::string nicName;

    #if defined(HAVE_RTS_INTERNAL_SOURCE_CODE)
        Utils::NetworkInterfaceInfo nic;
    #endif

    std::string enginePolicyJson;
    std::string userIdentityJson;

    // Check out the command line
    for(int x = 1; x < argc; x++)
    {        
        if(strncmp(argv[x], "-jsonobjects", 12) == 0)
        {
            const char *path = nullptr;
            const char *p = strchr(argv[x], ':');
            if(p != nullptr)
            {
                path = (p + 1);
            }

            ConfigurationObjects::dumpExampleConfigurations(path);
            exit(0);
        }
        else if(strncmp(argv[x], "-mission:", 9) == 0)
        {
            missionFile = argv[x] + 9;
        }        
        else if(strncmp(argv[x], "-nic:", 5) == 0)
        {
            nicName = argv[x] + 5;
        }
        else if(strncmp(argv[x], "-ep:", 4) == 0)
        {
            epFile = argv[x] + 4;
        }        
        else if(strncmp(argv[x], "-rp:", 4) == 0)
        {
            rpFile = argv[x] + 4;
        }        
        else if(strcmp(argv[x], "-anon") == 0)
        {
            g_anonymous = true;
        }
        else if(strcmp(argv[x], "-useadad") == 0)
        {
            g_useadad = true;
        }
        else if(strncmp(argv[x], "-script:", 8) == 0)
        {
            pszScript = (argv[x] + 8);
        }
        else if(strcmp(argv[x], "-verbose") == 0)
        {
            g_verboseScript = true;
        }        
        else if(strncmp(argv[x], "-ua:", 4) == 0)
        {
            memset(g_szUsepLdonfigAlias, 0, sizeof(g_szUsepLdonfigAlias));
            strncpy_s(g_szUsepLdonfigAlias, sizeof(g_szUsepLdonfigAlias), argv[x] + 4, sizeof(g_szUsepLdonfigAlias)-1);
        }        
        else if(strncmp(argv[x], "-ui:", 4) == 0)
        {
            memset(g_szUsepLdonfigUserId, 0, sizeof(g_szUsepLdonfigUserId));
			strncpy_s(g_szUsepLdonfigUserId, sizeof(g_szUsepLdonfigUserId), argv[x] + 4, sizeof(g_szUsepLdonfigUserId)-1);
        }
        else if(strncmp(argv[x], "-ud:", 4) == 0)
        {
            memset(g_szUsepLdonfigDisplayName, 0, sizeof(g_szUsepLdonfigDisplayName));
			strncpy_s(g_szUsepLdonfigDisplayName, sizeof(g_szUsepLdonfigDisplayName), argv[x] + 4, sizeof(g_szUsepLdonfigDisplayName)-1);
        }
        else if(strncmp(argv[x], "-ut:", 4) == 0)
        {
            g_txPriority = atoi(argv[x] + 4);
        }
        else if(strncmp(argv[x], "-uf:", 4) == 0)
        {
            g_txFlags = atoi(argv[x] + 4);
        }
        else if(strncmp(argv[x], "-putenv:", 8) == 0)
        {
            putenv((char*)(argv[x] + 8));
        }
        else
        {
            std::cout << "unknown option '" << argv[x] << "'" << std::endl;
            showUsage();
            goto end_function;
        }
    }

    // We're just going to use a simple random number generator for this app
    srand((unsigned int)time(NULL));

    if(pszScript != nullptr && pszScript[0])
    {
        haveScript = loadScript(pszScript);
        if(!haveScript)
        {
            goto end_function;
        }
    }

    // If we're not going to be anonymous then create our user configuration json
    if(!g_anonymous)
    {
        // Make a random alias if we don't have one
        if(!g_szUsepLdonfigAlias[0])
        {
            #if defined(__APPLE__)
                const char *prefix = "APL-";
            #elif defined(__ANDROID__)
                const char *prefix = "AND-";
            #elif defined(__linux__)
                const char *prefix = "LNX-";
            #elif defined(WIN32)
                const char *prefix = "WIN-";
            #else
                const char *prefix = "UNK-";    
            #endif

            sprintf_s(g_szUsepLdonfigAlias, sizeof(g_szUsepLdonfigAlias), "%s%012X", prefix, rand());
        }
        
        // Cook up a user id if we don't have one
        if(!g_szUsepLdonfigUserId[0])
        {
            sprintf_s(g_szUsepLdonfigUserId, sizeof(g_szUsepLdonfigUserId), "%s@engagedev.rallytac.com", g_szUsepLdonfigAlias);
        }

        // Cook up a display name if we don't have one
        if(!g_szUsepLdonfigDisplayName[0])
        {
            sprintf_s(g_szUsepLdonfigDisplayName, sizeof(g_szUsepLdonfigDisplayName), "User %s", g_szUsepLdonfigAlias);
        }

        // Make sure our transmit priority is a valid value
        if(g_txPriority < 0 || g_txPriority > 255)
        {
            g_txPriority = 0;
        }
    }

    if(missionFile == nullptr || missionFile[0] == 0)
    {
        std::cerr << "no mission file specified" << std::endl;
        showUsage();
        goto end_function;
    }

    // Register our callbacks
    if(!registepLdallbacks())
    {
        std::cerr << "callback registration failed" << std::endl;
        goto end_function;
    }

    // Load our engine policy (if any)
    if(epFile != nullptr)
    {
        if(!loadPolicy(epFile, &g_enginePolicy))
        {
            std::cerr << "could not load engine policy '" << epFile << "'" << std::endl;
            goto end_function;
        }        
    }

    // Load our mission
    g_rallypoint = new ConfigurationObjects::Rallypoint();
    if(!loadMission(missionFile, &g_mission, g_rallypoint))
    {
        std::cerr << "could not load mission '" << missionFile << "'" << std::endl;
        goto end_function;
    }

    // Load our RP (if any)
    if(rpFile != nullptr)
    {
        if(!loadRp(rpFile, g_rallypoint))
        {
            std::cerr << "could not load rallypoint '" << rpFile << "'" << std::endl;
            goto end_function;
        }        
    }

    if(!nicName.empty())
    {
        g_enginePolicy.networking.defaultNic = nicName;
    }

#ifdef WIN32
	if (g_enginePolicy.networking.defaultNic.empty())
	{
		std::cerr << "WARNING : The Windows version of engage-cmd does not pass the network interface name to the Engage Engine!" << std::endl;
	}
#else
    #if defined(HAVE_RTS_INTERNAL_SOURCE_CODE)
        if(g_enginePolicy.networking.defaultNic.empty())
        {
            if(!Utils::getFirstViableNic(nic, AF_INET))
            {
                std::cerr << "no viable nics found - following nics are present:" << std::endl;

                std::vector<Utils::NetworkInterfaceInfo> presentNics = Utils::getNics();
                for(size_t x = 0; x < presentNics.size(); x++)
                {
                    std::cerr << "name='" << presentNics[x]._name << "'"
                            << ", family=" << presentNics[x]._family
                            << ", address='" << presentNics[x]._address << "'"
                            << ", available=" << presentNics[x]._isAvailable
                            << ", loopBack=" << presentNics[x]._isLoopback
                            << ", supportsMulticast=" << presentNics[x]._supportsMulticast << std::endl;
                }

                return -1;
            }        
        }    
        else
        {
            if(!Utils::getNicByName(g_enginePolicy.networking.defaultNic.c_str(), AF_INET, nic))
            {
                std::cerr << "WARNING: '" << nicName.c_str() << "' not found found - following nics are present:" << std::endl;

                std::vector<Utils::NetworkInterfaceInfo> presentNics = Utils::getNics();
                for(size_t x = 0; x < presentNics.size(); x++)
                {
                    std::cerr << "name='" << presentNics[x]._name << "'"
                            << ", family=" << presentNics[x]._family
                            << ", address='" << presentNics[x]._address << "'"
                            << ", available=" << presentNics[x]._isAvailable
                            << ", loopBack=" << presentNics[x]._isLoopback
                            << ", supportsMulticast=" << presentNics[x]._supportsMulticast << std::endl;
                }

                strcpy_s(nic._name, sizeof(nic._name), nicName.c_str());
            }
        }

        std::cout << "name='" << nic._name << "'"
                    << ", family=" << nic._family
                    << ", address='" << nic._address << "'"
                    << ", available=" << nic._isAvailable
                    << ", loopBack=" << nic._isLoopback
                    << ", supportsMulticast=" << nic._supportsMulticast << std::endl;
    #endif
#endif

    // Build the information we'll need for our UI
    for(std::vector<ConfigurationObjects::Group>::iterator itr = g_mission.groups.begin();
        itr != g_mission.groups.end();
        itr++)
    {
        GroupInfo_t gi;

        gi.id = (int)itr->type;
        gi.id = itr->id;
        gi.name = itr->name;
        gi.isEncrypted = (!itr->cryptoPassword.empty());
        gi.allowsFullDuplex = itr->txAudio.fdx;

        // Get the JSON representation - it'll be needed when we create the group
        gi.jsonConfiguration = itr->serialize();

        g_new_groups.push_back(gi);
    }

    // At this point we have our group list, show them
    showGroups();

    // Create a user identity
    userIdentityJson = "{";
    if(!g_anonymous)
    {
        userIdentityJson += "\"userId\":\"";
        userIdentityJson += g_szUsepLdonfigUserId;
        userIdentityJson += "\"";

        userIdentityJson += ",\"displayName\":\"";
        userIdentityJson += g_szUsepLdonfigDisplayName;
        userIdentityJson += "\"";
    }
    userIdentityJson += "}";

    // Build our engine policy
    enginePolicyJson = g_enginePolicy.serialize();

    // Initialize the library
    pLd = engageInitialize(enginePolicyJson.c_str(), userIdentityJson.c_str(), nullptr);
    if(pLd != ENGAGE_RESULT_OK)
    {
        std::cerr << "engageInitialize failed" << std::endl;
        goto end_function;
    }

    // Start the Engine
    pLd = engageStart();
    if(pLd != ENGAGE_RESULT_OK)
    {
        std::cerr << "engageStart failed" << std::endl;
        goto end_function;
    }

    if(g_useadad)
    {
        registerADAD();
    }

    if(haveScript)
    {
        if( !runScript() )
        {
            continueWithCmdLine = false;
        }
    }

    // Go round and round getting commands from our console user
    while( continueWithCmdLine )
    {
        char buff[256];
        char *p;
        std::cout << "............running >";

        if( !fgets(buff, sizeof(buff), stdin) )
        {
            continue;
        }

        p = (buff + strlen(buff) - 1);
        while((p >= buff) && (*p == '\n'))
        {
            *p = 0;
            p--;
        }

        if(buff[0] == 0)
        {
            continue;
        }
        
        else if(strcmp(buff, "q") == 0)
        {
            break;
        }

        else if(buff[0] == '!')
        {
            if(system(buff + 1) < 0)
            {
                std::cout << "Error executing '" << (buff + 1) << "'" << std::endl;
            }
        }

        else if(strncmp(buff, "tl", 2) == 0)
        {
            const char *p = (buff + 2);
            while( *p == ' ')
            {
                p++;
            }

            if( *p == 0 )
            {
                p = "filters/default-timeline-query.json";
            }

            if(*p != 0)
            {
                std::string filter;
                ConfigurationObjects::readTextFileIntoString(p, filter);
                if(!filter.empty())
                {
                    engageQueryGroupTimeline(g_new_groups[1].id.c_str(), filter.c_str());
                }
                else
                {
                    std::cout << "ERROR: Cannot load specified filter file" << std::endl;
                }                
            }
            else
            {
                std::cout << "NOTE: No filter file specified" << std::endl;
                engageQueryGroupTimeline(g_new_groups[1].id.c_str(), "{}");
            }

            /*
            "((started >= 123 && ended <= 999) || duration >= 1000) && (alias matches '[*]TEST[*]')"            

            "started >= 123"
            "ended <= 999"
            "duration >= 1000"
            "alias like '*TEST*'"
            "nodeId = '{bb805ced-3d01-420d-9a2a-8cfdf4eded0b}'"
            "direction = 2"
            "audio.ms >= 500"
            "audio.samples >= 2000"
            "id = '{ef34ec8e-8a18-456e-9c14-073373830e31}'"
            "type = 2"
            "inProgress = false"
            */
        }

        else if(buff[0] == '1')
        {
            devTest1();
        }
        else if(buff[0] == '2')
        {
            devTest2();
        }
        else if(buff[0] == '3')
        {
            devTest3();
        }

        /*
        else if(buff[0] == 'l')
        {            
            const char *msg = "hello world blob";
            size_t size = strlen(msg + 1);

            uint8_t *blob = new uint8_t[size];
            memcpy(blob, msg, size);
            std::string jsonParams;

            ConfigurationObjects::BlobInfo bi;
            bi.rtpHeader.pt = 114;
            jsonParams = bi.serialize();

            doSendBlob(buff[1] == 'a' ? -1 : atoi(buff + 1), blob, size, jsonParams.c_str());

            delete[] blob;
        }
        else if(buff[0] == 'r')
        {
            const char *msg = "hello world rtp";
            size_t size = strlen(msg + 1);

            uint8_t *payload = new uint8_t[size];
            memcpy(payload, msg, size);
            std::string jsonParams;

            ConfigurationObjects::RtpHeader rtpHeader;
            rtpHeader.pt = 109;
            jsonParams = rtpHeader.serialize();

            doSendRtp(buff[1] == 'a' ? -1 : atoi(buff + 1), payload, size, jsonParams.c_str());

            delete[] payload;
        }
        else if(buff[0] == 'w')
        {
            const char *msg = "hello world raw";
            size_t size = strlen(msg + 1);

            uint8_t *payload = new uint8_t[size];
            memcpy(payload, msg, size);
            std::string jsonParams;

            doSendRaw(buff[1] == 'a' ? -1 : atoi(buff + 1), payload, size, jsonParams.c_str());

            delete[] payload;
        }
        */
       
        else if(strcmp(buff, "?") == 0)
        {
            showHelp();
        }
        else if(strcmp(buff, "sg") == 0)
        {
            showGroups();
        }
        else if(strcmp(buff, "sn") == 0)
        {
            showNodes();
        }
        else if(buff[0] == 'z')
        {
            doUpdatePresence(buff[1] == 'a' ? -1 : atoi(buff + 1));
        }
        else if(buff[0] == 'c')
        {
            doCreate(buff[1] == 'a' ? -1 : atoi(buff + 1));
        }
        else if(buff[0] == 'd')
        {
            doDelete(buff[1] == 'a' ? -1 : atoi(buff + 1));
        }
        else if(buff[0] == 'j')
        {
            doJoin(buff[1] == 'a' ? -1 : atoi(buff + 1));
        }
        else if(buff[0] == 'l')
        {
            doLeave(buff[1] == 'a' ? -1 : atoi(buff + 1));
        }
        else if(buff[0] == 'p')
        {
            g_txPriority = atoi(buff + 1);
            std::cout << "tx priority set to " << g_txPriority << std::endl;
        }
        else if(buff[0] == 'f')
        {
            g_txFlags = atoi(buff + 1);
            std::cout << "tx flags set to " << g_txFlags << std::endl;
        }
        else if(buff[0] == 't')
        {
            char *val = strchr(buff, ' ');
            if(val != nullptr)
            {
                *val = 0;
                int tag = atoi(val + 1);
                doSetGroupRxTag(buff[1] == 'a' ? -1 : atoi(buff + 1), tag);
            }
        }
        else if(buff[0] == 'b')
        {
            doBeginTx(buff[1] == 'a' ? -1 : atoi(buff + 1));
        }
        else if(buff[0] == 'e')
        {
            doEndTx(buff[1] == 'a' ? -1 : atoi(buff + 1));
        }
        else if(buff[0] == 'm')
        {
            doMuteRx(buff[1] == 'a' ? -1 : atoi(buff + 1));
        }
        else if(buff[0] == 'u')
        {
            doUnmuteRx(buff[1] == 'a' ? -1 : atoi(buff + 1));
        }
        else if(buff[0] == 'x')
        {
            char *val = strchr(buff, ' ');
            if(val != nullptr)
            {
                *val = 0;
                int tag = atoi(val + 1);
                doRegisterGroupRtpHandler(buff[1] == 'a' ? -1 : atoi(buff + 1), tag);
            }
        }
        else if(buff[0] == 'y')
        {
            char *val = strchr(buff, ' ');
            if(val != nullptr)
            {
                *val = 0;
                int tag = atoi(val + 1);
                doUnregisterGroupRtpHandler(buff[1] == 'a' ? -1 : atoi(buff + 1), tag);
            }
        }
        else
        {
            std::cout << "'" << buff << "' not recognized" << std::endl;
            showHelp();
        }

        memset(buff, 0, sizeof(buff));
    }

    unregisterADAD();

end_function:
    // Stop the Engine
    engageStop();

    // Shut down the library
    pLd = engageShutdown();

    // Clean up
    g_new_groups.clear();

    return 0;
}

std::string buildGroupCreationJson(int index)
{
    // Parse the baseline configuration into "groupConfig"
    ConfigurationObjects::Group groupConfig;
    groupConfig.deserialize(g_new_groups[index].jsonConfiguration.c_str());

    if(!g_anonymous)
    {
        groupConfig.alias = g_szUsepLdonfigAlias;
    }
    else
    {
        groupConfig.alias.clear();
    }

    if(g_rallypoint != nullptr && g_rallypoint->host.port > 0)
    {
        groupConfig.rallypoints.push_back(*g_rallypoint);
    }

    // We'll use our application-defined speaker audio device if we have one
    if(g_speakerDeviceId != 0)
    {
        groupConfig.audio.outputId = g_speakerDeviceId;
    }

    // We'll use our application-defined microphone audio device if we have one
    if(g_microphoneDeviceId != 0)
    {
        groupConfig.audio.inputId = g_microphoneDeviceId;
    }

    // Serialize to a string
    std::string rc = groupConfig.serialize();

    std::cout << "buildGroupCreationJson: " << rc << std::endl;

    return rc;
}

void doStartEngine()
{
    if(g_verboseScript) std::cout << "doStartEngine" << std::endl;
    engageStart();
}

void doStopEngine()
{
    if(g_verboseScript) std::cout << "doStopEngine" << std::endl;
    engageStop();
}

void doUpdatePresence(int index)
{
    if(g_verboseScript) std::cout << "doUpdatePresence: " << index << std::endl;
    std::string jsonText;
    ConfigurationObjects::readTextFileIntoString(g_pszPresenceFile, jsonText);

    if(index == -1)
    {
        for(size_t x = 0; x < g_new_groups.size(); x++)
        {
            engageUpdatePresenceDescriptor(g_new_groups[x].id.c_str(), jsonText.c_str(), true);
        }
    }
    else
    {
        if(index >= 0 && index < (int)g_new_groups.size())
        {
            engageUpdatePresenceDescriptor(g_new_groups[index].id.c_str(), jsonText.c_str(), true);
        }
        else
        {
            std::cerr << "invalid index" << std::endl;
        }                
    }
}

void doCreate(int index)
{
    if(g_verboseScript) std::cout << "doCreate: " << index << std::endl;
    std::string json;

    if(index == -1)
    {
        for(size_t x = 0; x < g_new_groups.size(); x++)
        {
            json = buildGroupCreationJson((int)x);
            engageCreateGroup(json.c_str());
        }
    }
    else
    {
        if(index >= 0 && index < (int)g_new_groups.size())
        {
            json = buildGroupCreationJson(index);
            engageCreateGroup(json.c_str());
        }
        else
        {
            std::cerr << "invalid index" << std::endl;
        }                
    }
}

void doDelete(int index)
{
    if(g_verboseScript) std::cout << "doDelete: " << index << std::endl;
    if(index == -1)
    {
        for(size_t x = 0; x < g_new_groups.size(); x++)
        {
            engageDeleteGroup(g_new_groups[x].id.c_str());
        }
    }
    else
    {
        if(index >= 0 && index < (int)g_new_groups.size())
        {
            engageDeleteGroup(g_new_groups[index].id.c_str());
        }
        else
        {
            std::cerr << "invalid index" << std::endl;
        }                
    }
}

void doJoin(int index)
{
    if(g_verboseScript) std::cout << "doJoin: " << index << std::endl;
    if(index == -1)
    {
        for(size_t x = 0; x < g_new_groups.size(); x++)
        {
            engageJoinGroup(g_new_groups[x].id.c_str());
        }
    }
    else
    {
        if(index >= 0 && index < (int)g_new_groups.size())
        {
            engageJoinGroup(g_new_groups[index].id.c_str());
        }
        else
        {
            std::cerr << "invalid index" << std::endl;
        }                
    }
}

void doLeave(int index)
{
    if(g_verboseScript) std::cout << "doLeave: " << index << std::endl;
    if(index == -1)
    {
        for(size_t x = 0; x < g_new_groups.size(); x++)
        {
            engageLeaveGroup(g_new_groups[x].id.c_str());
        }
    }
    else
    {
        if(index >= 0 && index < (int)g_new_groups.size())
        {
            engageLeaveGroup(g_new_groups[index].id.c_str());
        }
        else
        {
            std::cerr << "invalid index" << std::endl;
        }                
    }
}

void doBeginTx(int index)
{
    ConfigurationObjects::AdvancedTxParams params;
    params.flags = (uint16_t)0;
    params.priority = (uint8_t)0;

    if(!g_anonymous)
    {
        params.alias = g_szUsepLdonfigUserId;
    }

    params.subchannelTag = 0;
    params.includeNodeId = true;
    std::string json = params.serialize();

    if(g_verboseScript) std::cout << "doBeginTx: " << index << std::endl;
    if(index == -1)
    {
        for(size_t x = 0; x < g_new_groups.size(); x++)
        {
            //engageBeginGroupTx(g_new_groups[x].id.c_str(), g_txPriority, g_txFlags);
            engageBeginGroupTxAdvanced(g_new_groups[x].id.c_str(), json.c_str());
        }
    }
    else
    {
        if(index >= 0 && index < (int)g_new_groups.size())
        {
            //engageBeginGroupTx(g_new_groups[index].id.c_str(), g_txPriority, g_txFlags);
            engageBeginGroupTxAdvanced(g_new_groups[index].id.c_str(), json.c_str());
        }
        else
        {
            std::cerr << "invalid index" << std::endl;
        }                
    }
}

void doEndTx(int index)
{
    if(g_verboseScript) std::cout << "doEndTx: " << index << std::endl;
    if(index == -1)
    {
        for(size_t x = 0; x < g_new_groups.size(); x++)
        {
            engageEndGroupTx(g_new_groups[x].id.c_str());
        }
    }
    else
    {
        if(index >= 0 && index < (int)g_new_groups.size())
        {
            engageEndGroupTx(g_new_groups[index].id.c_str());
        }
        else
        {
            std::cerr << "invalid index" << std::endl;
        }                
    }
}

void doMuteRx(int index)
{
    if(g_verboseScript) std::cout << "doMuteRx: " << index << std::endl;
    if(index == -1)
    {
        for(size_t x = 0; x < g_new_groups.size(); x++)
        {
            engageMuteGroupRx(g_new_groups[x].id.c_str());
        }
    }
    else
    {
        if(index >= 0 && index < (int)g_new_groups.size())
        {
            engageMuteGroupRx(g_new_groups[index].id.c_str());
        }
        else
        {
            std::cerr << "invalid index" << std::endl;
        }                
    }
}

void doUnmuteRx(int index)
{
    if(g_verboseScript) std::cout << "doUnmuteRx: " << index << std::endl;
    if(index == -1)
    {
        for(size_t x = 0; x < g_new_groups.size(); x++)
        {
            engageUnmuteGroupRx(g_new_groups[x].id.c_str());
        }
    }
    else
    {
        if(index >= 0 && index < (int)g_new_groups.size())
        {
            engageUnmuteGroupRx(g_new_groups[index].id.c_str());
        }
        else
        {
            std::cerr << "invalid index" << std::endl;
        }                
    }
}

void doSetGroupRxTag(int index, int tag)
{
    if(g_verboseScript) std::cout << "doSetGroupRxTag: " << index << std::endl;
    if(index == -1)
    {
        for(size_t x = 0; x < g_new_groups.size(); x++)
        {
            engageSetGroupRxTag(g_new_groups[x].id.c_str(), tag);
        }
    }
    else
    {
        if(index >= 0 && index < (int)g_new_groups.size())
        {
            engageSetGroupRxTag(g_new_groups[index].id.c_str(), tag);
        }
        else
        {
            std::cerr << "invalid index" << std::endl;
        }                
    }
}

void doRegisterGroupRtpHandler(int index, int payloadId)
{
    if(g_verboseScript) std::cout << "doRegisterGroupRtpHandler: " << index << std::endl;
    if(index == -1)
    {
        for(size_t x = 0; x < g_new_groups.size(); x++)
        {
            engageRegisterGroupRtpHandler(g_new_groups[x].id.c_str(), payloadId);
        }
    }
    else
    {
        if(index >= 0 && index < (int)g_new_groups.size())
        {
            engageRegisterGroupRtpHandler(g_new_groups[index].id.c_str(), payloadId);
        }
        else
        {
            std::cerr << "invalid index" << std::endl;
        }                
    }
}

void doUnregisterGroupRtpHandler(int index, int payloadId)
{
    if(g_verboseScript) std::cout << "doUnregisterGroupRtpHandler: " << index << std::endl;
    if(index == -1)
    {
        for(size_t x = 0; x < g_new_groups.size(); x++)
        {
            engageUnregisterGroupRtpHandler(g_new_groups[x].id.c_str(), payloadId);
        }
    }
    else
    {
        if(index >= 0 && index < (int)g_new_groups.size())
        {
            engageUnregisterGroupRtpHandler(g_new_groups[index].id.c_str(), payloadId);
        }
        else
        {
            std::cerr << "invalid index" << std::endl;
        }                
    }
}

void doSendBlob(int index, const uint8_t* blob, size_t size, const char *jsonParams)
{
    if(g_verboseScript) std::cout << "doSendBlob: " << index << std::endl;
    if(index == -1)
    {
        for(size_t x = 0; x < g_new_groups.size(); x++)
        {
            engageSendGroupBlob(g_new_groups[x].id.c_str(), blob, size, jsonParams);
        }
    }
    else
    {
        if(index >= 0 && index < (int)g_new_groups.size())
        {
            engageSendGroupBlob(g_new_groups[index].id.c_str(), blob, size, jsonParams);
        }
        else
        {
            std::cerr << "invalid index" << std::endl;
        }                
    }
}

void doSendRtp(int index, const uint8_t* payload, size_t size, const char *jsonParams)
{
    if(g_verboseScript) std::cout << "doSendRtp: " << index << std::endl;
    if(index == -1)
    {
        for(size_t x = 0; x < g_new_groups.size(); x++)
        {
            engageSendGroupRtp(g_new_groups[x].id.c_str(), payload, size, jsonParams);
        }
    }
    else
    {
        if(index >= 0 && index < (int)g_new_groups.size())
        {
            engageSendGroupRtp(g_new_groups[index].id.c_str(), payload, size, jsonParams);
        }
        else
        {
            std::cerr << "invalid index" << std::endl;
        }                
    }
}

void doSendRaw(int index, const uint8_t* raw, size_t size, const char *jsonParams)
{
    if(g_verboseScript) std::cout << "doSendRaw: " << index << std::endl;
    if(index == -1)
    {
        for(size_t x = 0; x < g_new_groups.size(); x++)
        {
            engageSendGroupRaw(g_new_groups[x].id.c_str(), raw, size, jsonParams);
        }
    }
    else
    {
        if(index >= 0 && index < (int)g_new_groups.size())
        {
            engageSendGroupRaw(g_new_groups[index].id.c_str(), raw, size, jsonParams);
        }
        else
        {
            std::cerr << "invalid index" << std::endl;
        }                
    }
}

// Application-defined audio device
class ADADInstance
{
public:
    ADADInstance(int16_t deviceId, 
                 int16_t instanceId, 
                 ConfigurationObjects::AudioDeviceDescriptor::Direction_t direction)
    {
        _deviceId = deviceId;
        _instanceId = instanceId;
        _direction = direction;
        _running = false;
        _paused = false;
    }

    int start()
    {
        if(!_running)
        {
            _running = true;
            _paused = false;
            _threadHandle = std::thread(&ADADInstance::thread, this);
        }

        return ENGAGE_AUDIO_DEVICE_RESULT_OK;
    }

    int stop()
    {
        _running = false;
        if(_threadHandle.joinable())
        {
            _threadHandle.join();
        }

        return ENGAGE_AUDIO_DEVICE_RESULT_OK;
    }

    int pause()
    {
        _paused = true;
        return ENGAGE_AUDIO_DEVICE_RESULT_OK;
    }

    int resume()
    {
        _paused = false;
        return ENGAGE_AUDIO_DEVICE_RESULT_OK;
    }

    int reset()
    {
        return ENGAGE_AUDIO_DEVICE_RESULT_OK;
    }

    int restart()
    {
        return ENGAGE_AUDIO_DEVICE_RESULT_OK;
    }

private:
    ADADInstance()
    {
        // Not to be used
    }

    void thread()
    {
        // Our "device" will work in 60ms intervals
        const size_t  MY_AUDIO_DEVICE_INTERVAL_MS = 60;

        // The number of samples we produce/consume is 16 samples per millisecond - i.e. this is a wideband device
        const size_t  MY_AUDIO_DEVICE_BUFFER_SAMPLE_COUNT = (MY_AUDIO_DEVICE_INTERVAL_MS * 16);

        int16_t buffer[MY_AUDIO_DEVICE_BUFFER_SAMPLE_COUNT];
        int     rc;
        //int     x;

        // These are used to generate the sine wave for our "microphone"
        /*
        float amplitute = 1000;
        float pi = 22/7;
        float freq = 1024;
        float tm = 0.0;
        float phaseShift = 0.0;
        */

       memset(buffer, 0, sizeof(buffer));

        while( _running )
        {
            if(!_paused)
            {
                if(_direction == ConfigurationObjects::AudioDeviceDescriptor::Direction_t::dirOutput)
                {
                    rc = engageAudioDeviceReadBuffer(_deviceId, _instanceId, buffer, MY_AUDIO_DEVICE_BUFFER_SAMPLE_COUNT);

                    if(rc > 0)
                    {
                        // At this point we have rc number of audio samples from the Engine.  These now need to be sent 
                        // onward to where they're needed.  For purposes of this demonstration, we'll simply calaculate the
                        // average sample level and display that.

                        /*
                        float total = 0.0;
                        for(x = 0; x < rc; x++)
                        {
                            total += buffer[x];
                        }
                        */

                        //std::cout << "ADADInstance eadReadBuffer received " << rc << "samples with an average sample level of " << (total / rc) << std::endl;
                    }
                }
                else if(_direction == ConfigurationObjects::AudioDeviceDescriptor::Direction_t::dirInput)
                {
                    // For purposes of this demo, we'll fill our buffer with an ongoing sine wave.  In your app you will want to pull in these
                    // samples from your actual audio source

                    /*
                    for(x = 0; x < (int)MY_AUDIO_DEVICE_BUFFER_SAMPLE_COUNT; x++)
                    {
                        tm += 1.0;

                        if(tm > 360.0)
                        {
                            tm = 1.0;
                        }

                        float sampleVal = amplitute * sin(2 * pi * freq * tm + phaseShift);

                        if(sampleVal < -32768.0)
                        {
                            sampleVal = -32768.0;
                        }
                        else if(sampleVal > 32767.0)
                        {
                            sampleVal = 32767.0;
                        }

                        buffer[x] = (int16_t)sampleVal;
                    }
                    */

                   /*
                   static FILE *fp = nullptr;

                   if(fp == nullptr)
                   {
                       fp = fopen("/Users/sbotha/tmp/bah.raw", "rb");
                   }

                   if(fp != nullptr)
                   {
                       size_t amountRead = fread(buffer, 1, MY_AUDIO_DEVICE_BUFFER_SAMPLE_COUNT * 2, fp);
                       if(amountRead != MY_AUDIO_DEVICE_BUFFER_SAMPLE_COUNT * 2)
                       {
                           fseek(fp, 0, SEEK_SET);
                           fread(buffer, 1, MY_AUDIO_DEVICE_BUFFER_SAMPLE_COUNT * 2, fp);
                       }
                   }
                   else
                   {
                       std::cout << "Not reading file" << std::endl;
                   }
                   */
                  
                   /*
                   for(int x = 0; x < MY_AUDIO_DEVICE_BUFFER_SAMPLE_COUNT; x++)
                   {
                       buffer[x] = (rand() % 32767);
                       if(rand() % 100 < 50)
                       {
                           buffer[x] *= -1;
                       }
                   }
                    */

                    rc = engageAudioDeviceWriteBuffer(_deviceId, _instanceId, buffer, MY_AUDIO_DEVICE_BUFFER_SAMPLE_COUNT);
                }
                else
                {
                    assert(0);
                }
            }

            // Sleep for our device's "interval"
            std::this_thread::sleep_for(std::chrono::milliseconds(MY_AUDIO_DEVICE_INTERVAL_MS));
        }
    }

    int16_t                                                     _deviceId;
    int16_t                                                     _instanceId;
    ConfigurationObjects::AudioDeviceDescriptor::Direction_t    _direction;
    std::thread                                                 _threadHandle;
    bool                                                        _running;
    bool                                                        _paused;
};

std::map<int16_t, ADADInstance*>    g_audioDeviceInstances;

int MyAudioDeviceCallback(int16_t deviceId, int16_t instanceId, EngageAudioDeviceCtlOp_t op, uintptr_t p1)
{   
    int rc = ENGAGE_AUDIO_DEVICE_RESULT_OK;

    ADADInstance *instance = nullptr;
    std::cout << "MyAudioDeviceCallback: deviceId=" << deviceId << ", ";

    // Instance creation is a little different from other operations
    if( op == EngageAudioDeviceCtlOp_t::eadCreateInstance)
    {
        g_nextAudioDeviceInstanceId++;

        if(deviceId == g_speakerDeviceId)
        {
            // Create an instance of a speaker
            instance = new ADADInstance(deviceId, g_nextAudioDeviceInstanceId, ConfigurationObjects::AudioDeviceDescriptor::Direction_t::dirOutput);
        }
        else if(deviceId == g_microphoneDeviceId)
        {
            // Create an instance of a speaker
            instance = new ADADInstance(deviceId, g_nextAudioDeviceInstanceId, ConfigurationObjects::AudioDeviceDescriptor::Direction_t::dirInput);
        }
        else
        {
            assert(0);
        }     

        g_audioDeviceInstances[g_nextAudioDeviceInstanceId] = instance;

        rc = g_nextAudioDeviceInstanceId;
    }
    else
    {
        // Track down the instance object
        std::map<int16_t, ADADInstance*>::iterator itr = g_audioDeviceInstances.find(instanceId);
        if(itr != g_audioDeviceInstances.end())
        {
            instance = itr->second;

            // The Engine wants us to ...
            switch( op )
            {
                // We should never fall into this case because the "if" above catered for it.  But, some compilers
                // will warn about the switch not catering for all enum values from EngageAudioDeviceCtlOp_t.  So we'll
                // put in this case to keep them happy.
                case EngageAudioDeviceCtlOp_t::eadCreateInstance:
                    assert(0);
                    break;

                // ... destroy an instance of a speaker identified by "instanceId"
                case EngageAudioDeviceCtlOp_t::eadDestroyInstance:
                    std::cout << "destroy instance instanceId=" << instanceId << std::endl;
                    instance->stop();
                    delete instance;
                    g_audioDeviceInstances.erase(itr);
                    break;

                // ... start an instance of a device identified by "instanceId"
                case EngageAudioDeviceCtlOp_t::eadStart:
                    std::cout << "start" << std::endl;
                    instance->start();
                    break;

                // ... stop an instance of a device identified by "instanceId"
                case EngageAudioDeviceCtlOp_t::eadStop:
                    std::cout << "stop" << std::endl;
                    instance->stop();
                    break;

                // ... pause an instance of a device identified by "instanceId"
                case EngageAudioDeviceCtlOp_t::eadPause:
                    std::cout << "pause" << std::endl;
                    instance->pause();
                    break;

                // ... resume an instance of a device identified by "instanceId"
                case EngageAudioDeviceCtlOp_t::eadResume:
                    std::cout << "resume" << std::endl;
                    instance->resume();
                    break;

                // ... reset an instance of a device identified by "instanceId"
                case EngageAudioDeviceCtlOp_t::eadReset:
                    std::cout << "reset" << std::endl;
                    instance->reset();
                    break;

                // ... restart an instance of a device identified by "instanceId"
                case EngageAudioDeviceCtlOp_t::eadRestart:
                    std::cout << "restart" << std::endl;
                    instance->restart();
                    break;

                // The compiler should catch this.  But, just in case ...
                default:
                    assert(false);
                    rc = ENGAGE_AUDIO_DEVICE_INVALID_OPERATION;
                    break;
            }            
        }
        else
        {
            std::cout << "MyAudioDeviceCallback for an unknown instance id of " << instanceId << std::endl;
            rc = ENGAGE_AUDIO_DEVICE_INVALID_INSTANCE_ID;
        }        
    }

    return rc;
}

void registerADAD()
{
    // Setup the speaker
    {
        ConfigurationObjects::AudioDeviceDescriptor speakerDevice;
        speakerDevice.direction = ConfigurationObjects::AudioDeviceDescriptor::Direction_t::dirOutput;
        speakerDevice.deviceId = 0;
        speakerDevice.samplingRate = 8000;
        speakerDevice.channels = 2;
        speakerDevice.boostPercentage = 0;
        std::string json = speakerDevice.serialize();
        g_speakerDeviceId = engageAudioDeviceRegister(json.c_str(), MyAudioDeviceCallback);
        if(g_speakerDeviceId < 0)
        {
            g_speakerDeviceId = 0;
        }
    }

    // Setup the microphone
    {
        ConfigurationObjects::AudioDeviceDescriptor microphoneDevice;
        microphoneDevice.direction = ConfigurationObjects::AudioDeviceDescriptor::Direction_t::dirInput;
        microphoneDevice.deviceId = 0;
        microphoneDevice.samplingRate = 8000;
        microphoneDevice.channels = 1;
        microphoneDevice.boostPercentage = 0;
        std::string json = microphoneDevice.serialize();
        g_microphoneDeviceId = engageAudioDeviceRegister(json.c_str(), MyAudioDeviceCallback);
        if(g_microphoneDeviceId < 0)
        {
            g_microphoneDeviceId = 0;
        }
    }
}

void unregisterADAD()
{
    if(g_speakerDeviceId > 0)
    {
        engageAudioDeviceUnregister(g_speakerDeviceId);
        g_speakerDeviceId = 0;
    }
    
    if(g_microphoneDeviceId > 0)
    {
        engageAudioDeviceUnregister(g_microphoneDeviceId);
        g_microphoneDeviceId = 0;
    }
}

void cleanupADAInstances()
{
    for(std::map<int16_t, ADADInstance*>::iterator itr = g_audioDeviceInstances.begin();
        itr != g_audioDeviceInstances.end();
        itr++)
    {
        itr->second->stop();
        delete itr->second;
    }

    g_audioDeviceInstances.clear();
}


void showUsage()
{
    std::cout << "usage: engage-cmd -mission:<mission_file> [options]" << std::endl << std::endl
              << "\twhere [options] are:" << std::endl << std::endl
              << "\t-ep:<engine_policy_file> .............. specify an engine policy" << std::endl
              << "\t-rp:<rallypoint_file> ................. specify a rallypoint configuration" << std::endl
              << "\t-nic:<nic_name> ....................... specify the name of the default nic" << std::endl
              << "\t-ua:<user_alias> ...................... set the user's alias" << std::endl
              << "\t-ui:<user_id> ......................... set the user's id" << std::endl
              << "\t-ud:<user_display> .................... set the user's display name" << std::endl
              << "\t-ut:<user_tx_priority> ................ set audio transmit priority" << std::endl
              << "\t-uf:<user_tx_flags> ................... set audio transmit flags" << std::endl
              << "\t-putenv:<key>=<value> ................. set an environment variable" << std::endl
              << "\t-verbose .............................. enable verbose script mode" << std::endl
              << "\t-anon ................................. operate in anonymous identity mode" << std::endl
              << "\t-useadad .............................. use an application-defined audio device" << std::endl
              << "\t-jsonobjects .......................... display json object configuration" << std::endl;
}

void showHelp()
{
    std::cout << "q.............quit" << std::endl;
    std::cout << "!<command>....execute a shell command" << std::endl;
    std::cout << "-.............draw a line" << std::endl;
    std::cout << "sg............show group list" << std::endl;
    std::cout << "y.............request sync" << std::endl;
    std::cout << "p<N>..........set tx priority to N" << std::endl;
    std::cout << "f<N>..........set tx flags to N" << std::endl;
    std::cout << "c<N>..........create group index N" << std::endl;
    std::cout << "d<N>..........delete group index N" << std::endl;
    std::cout << "ca............create all groups" << std::endl;
    std::cout << "da............delete all groups" << std::endl;
    std::cout << "j<N>..........join group index N" << std::endl;
    std::cout << "ja............join all groups" << std::endl;
    std::cout << "l<N>..........leave group index N" << std::endl;
    std::cout << "la............leave all groups" << std::endl;
    std::cout << "b<N>..........begin tx on group index N" << std::endl;
    std::cout << "ba............begin tx on all groups" << std::endl;
    std::cout << "e<N>..........end tx on group index N" << std::endl;
    std::cout << "ea............end tx on all groups" << std::endl;
    std::cout << "m<N>..........mute rx on group index N" << std::endl;
    std::cout << "ma............mute rx on all groups" << std::endl;
    std::cout << "u<N>..........unmute rx on group index N" << std::endl;
    std::cout << "ua............unmute rx on all groups" << std::endl;
    std::cout << "z<N>..........send a presence update on presence group index N" << std::endl;
    std::cout << "za............send a presence update on all presence groups" << std::endl;
    std::cout << "z<N>..........send a presence update on presence group index N" << std::endl;
    std::cout << "ta <V>........set rx tag on all groups groups to V" << std::endl;
    std::cout << "t<N>> <V>.....set rx tag on group index N to V" << std::endl;
    std::cout << "la............send a blob on all groups" << std::endl;
    std::cout << "l<N>..........send a blob on group index N" << std::endl;
    std::cout << "ra............send a rtp payload on all groups" << std::endl;
    std::cout << "r<N>..........send a rtp payload on group index N" << std::endl;
    std::cout << "wa............send a raw payload on all groups" << std::endl;
    std::cout << "w<N>..........send a raw payload on group index N" << std::endl;
    std::cout << "xa <V>........register rtp payload handler <V> on all groups" << std::endl;
    std::cout << "x<N>..........register rtp payload handler <V> on group index N" << std::endl;
    std::cout << "ya <V>........unregister rtp payload handler <V> on all groups" << std::endl;
    std::cout << "y<N>..........unregister rtp payload handler <V> on group index N" << std::endl;
}

void showGroups()
{
    std::cout << "Groups:" << std::endl;
    for(size_t x = 0; x < g_new_groups.size(); x++)
    {
        std::cout << "index=" << x
                  << ", type=" << g_new_groups[x].type
                  << ", id=" << g_new_groups[x].id
                  << ", name=" << g_new_groups[x].name
                  << ", encrypted=" << (g_new_groups[x].isEncrypted ? "yes" : "no")
                  << ", fullDuplex=" << (g_new_groups[x].allowsFullDuplex ? "yes" : "no")
                  << std::endl;
    }
}

void showGroupAliases(ConfigurationObjects::PresenceDescriptor& pd)
{
    for(std::vector<ConfigurationObjects::GroupAlias>::iterator itr = pd.groupAliases.begin();
        itr != pd.groupAliases.end();
        itr++)
    {
        std::cout << "     " << itr->groupId << " = " << itr->alias << std::endl;
    }
}

void showNodes()
{
    std::cout << "\n\nNodes:" << std::endl;

    std::map<std::string, ConfigurationObjects::PresenceDescriptor>::iterator itr;
    for(itr = g_nodes.begin();
        itr != g_nodes.end();
        itr++)
    {
        ConfigurationObjects::PresenceDescriptor *pd = &(itr->second);

        std::cout << (pd->self ? "(*SELF*) " : "")
                  << pd->identity.nodeId 
                  << ", " << pd->identity.userId 
                  << ", " << pd->identity.displayName                  
                  << std::endl;

        showGroupAliases(*pd);

        std::cout << std::endl;
    }

    std::cout << "\n\n" << std::endl;    
}

bool loadPolicy(const char *pszFn, ConfigurationObjects::EnginePolicy *pPolicy)
{
    bool pLd = false;

    try
    {
        std::string jsonText;
        ConfigurationObjects::readTextFileIntoString(pszFn, jsonText);

        nlohmann::json j = nlohmann::json::parse(jsonText);
        ConfigurationObjects::from_json(j, *pPolicy);
        pLd = true;
    }
    catch (...)
    {
    }                
    
    return pLd;
}

bool loadMission(const char *pszFn, ConfigurationObjects::Mission *pMission, ConfigurationObjects::Rallypoint *pRp)
{
    bool pLd = false;

    try
    {
        std::string jsonText;
        ConfigurationObjects::readTextFileIntoString(pszFn, jsonText);

        nlohmann::json j = nlohmann::json::parse(jsonText);
        ConfigurationObjects::from_json(j, *pMission);
        pLd = true;

        // We may have a global rallypoint defined in this JSON
        try
        {
            nlohmann::json rp = j.at("rallypoint");
            std::string address = rp.at("address");
            int port = rp.at("port");

            pRp->host.address = address;
            pRp->host.port = port;
            pRp->certificate = "@default_cert.pem";
            pRp->certificateKey = "@default_key.pem";
        }
        catch(...)
        {
            pRp->clear();
        }
    }
    catch (...)
    {
    }                

    return pLd;
}

bool loadRp(const char *pszFn, ConfigurationObjects::Rallypoint *pRp)
{
    bool pLd = false;

    try
    {
        std::string jsonText;
        ConfigurationObjects::readTextFileIntoString(pszFn, jsonText);

        nlohmann::json j = nlohmann::json::parse(jsonText);
        ConfigurationObjects::from_json(j, *pRp);
        pLd = true;
    }
    catch (...)
    {
    }                
    return pLd;
}

void on_ENGAGE_ENGINE_STARTED(void)
{
    std::cout << "D/EngageMain: on_ENGAGE_ENGINE_STARTED" << std::endl;
}

void on_ENGAGE_ENGINE_STOPPED(void)
{
    std::cout << "D/EngageMain: on_ENGAGE_ENGINE_STOPPED" << std::endl;
}

void on_ENGAGE_RP_PAUSING_CONNECTION_ATTEMPT(const char *pId)
{
    std::cout << "D/EngageMain: on_ENGAGE_RP_PAUSING_CONNECTION_ATTEMPT: " << pId << std::endl;
}

void on_ENGAGE_RP_CONNECTING(const char *pId)
{
    std::cout << "D/EngageMain: on_ENGAGE_RP_CONNECTING: " << pId << std::endl;
}

void on_ENGAGE_RP_CONNECTED(const char *pId)
{
    std::cout << "D/EngageMain: on_ENGAGE_RP_CONNECTED: " << pId << std::endl;
}

void on_ENGAGE_RP_DISCONNECTED(const char *pId)
{
    std::cout << "D/EngageMain: on_ENGAGE_RP_DISCONNECTED: " << pId << std::endl;
}

void on_ENGAGE_RP_ROUNDTRIP_REPORT(const char *pId, uint32_t rtMs, uint32_t rtRating)
{
    std::cout << "D/EngageMain: on_ENGAGE_RP_ROUNDTRIP_REPORT: " << pId << ", ms=" << rtMs << ", rating=" << rtRating << std::endl;
}

void on_ENGAGE_GROUP_CREATED(const char *pId)
{
    std::cout << "D/EngageMain: on_ENGAGE_GROUP_CREATED: " << pId << std::endl;
}

void on_ENGAGE_GROUP_CREATE_FAILED(const char *pId)
{
    std::cout << "D/EngageMain: on_ENGAGE_GROUP_CREATE_FAILED: " << pId << std::endl;
}

void on_ENGAGE_GROUP_DELETED(const char *pId)
{
    std::cout << "D/EngageMain: on_ENGAGE_GROUP_DELETED: " << pId << std::endl;
}

void on_ENGAGE_GROUP_INSTANTIATED(const char *pId)
{
    std::cout << "D/EngageMain: on_ENGAGE_GROUP_INSTANTIATED: " << pId << std::endl;
}

void on_ENGAGE_GROUP_INSTANTIATE_FAILED(const char *pId)
{
    std::cout << "D/EngageMain: on_ENGAGE_GROUP_INSTANTIATE_FAILED: " << pId << std::endl;
}

void on_ENGAGE_GROUP_DEINSTANTIATED(const char *pId)
{
    std::cout << "D/EngageMain: on_ENGAGE_GROUP_DEINSTANTIATED: " << pId << std::endl;
}

void on_ENGAGE_GROUP_CONNECTED(const char *pId)
{
    std::cout << "D/EngageMain: on_ENGAGE_GROUP_CONNECTED: " << pId << std::endl;
}

void on_ENGAGE_GROUP_CONNECT_FAILED(const char *pId)
{
    std::cout << "D/EngageMain: on_ENGAGE_GROUP_CONNECT_FAILED: " << pId << std::endl;
}

void on_ENGAGE_GROUP_DISCONNECTED(const char *pId)
{
    std::cout << "D/EngageMain: on_ENGAGE_GROUP_DISCONNECTED: " << pId << std::endl;
}

void on_ENGAGE_GROUP_JOINED(const char *pId)
{
    std::cout << "D/EngageMain: on_ENGAGE_GROUP_JOINED: " << pId << std::endl;
}

void on_ENGAGE_GROUP_JOIN_FAILED(const char *pId)
{
    std::cout << "D/EngageMain: on_ENGAGE_GROUP_JOIN_FAILED: " << pId << std::endl;
}

void on_ENGAGE_GROUP_LEFT(const char *pId)
{
    std::cout << "D/EngageMain: on_ENGAGE_GROUP_LEFT: " << pId << std::endl;
}

void on_ENGAGE_GROUP_MEMBER_COUNT_CHANGED(const char *pId, size_t newCount)
{
    std::cout << "D/EngageMain: on_ENGAGE_GROUP_MEMBER_COUNT_CHANGED: " 
              << pId 
              << ", " 
              << newCount << std::endl;
}

void on_ENGAGE_GROUP_RX_STARTED(const char *pId)
{
    std::cout << "D/EngageMain: on_ENGAGE_GROUP_RX_STARTED: " << pId << std::endl;
}

void on_ENGAGE_GROUP_RX_ENDED(const char *pId)
{
    std::cout << "D/EngageMain: on_ENGAGE_GROUP_RX_ENDED: " << pId << std::endl;
}

void on_ENGAGE_GROUP_RX_MUTED(const char *pId)
{
    std::cout << "D/EngageMain: on_ENGAGE_GROUP_RX_MUTED: " << pId << std::endl;
}

void on_ENGAGE_GROUP_RX_UNMUTED(const char *pId)
{
    std::cout << "D/EngageMain: on_ENGAGE_GROUP_RX_UNMUTED: " << pId << std::endl;
}

void on_ENGAGE_GROUP_RX_SPEAKERS_CHANGED(const char *pId, const char *groupTalkerJson)
{
    std::string listOfNames;
    ConfigurationObjects::GroupTalkers gt;

    if(!gt.deserialize(groupTalkerJson))
    {
        listOfNames = "(none)";
    }
    else
    {
        for(std::vector<ConfigurationObjects::TalkerInformation>::iterator itr = gt.list.begin();
            itr != gt.list.end();
            itr++)
        {
            if(!listOfNames.empty())
            {
                listOfNames += ", ";
            }

            std::string info;

            info = itr->alias;
            info.append(" nodeId='");
            info.append(itr->nodeId);
            info.append("'");
            listOfNames += info;
        }
    }

    std::cout << "D/EngageMain: on_ENGAGE_GROUP_RX_SPEAKERS_CHANGED: " << pId << " [" << listOfNames << "]" << std::endl;
}

void on_ENGAGE_GROUP_TX_STARTED(const char *pId)
{
    std::cout << "D/EngageMain: on_ENGAGE_GROUP_TX_STARTED: " << pId << std::endl;
}

void on_ENGAGE_GROUP_TX_ENDED(const char *pId)
{
    std::cout << "D/EngageMain: on_ENGAGE_GROUP_TX_ENDED: " << pId << std::endl;
}

void on_ENGAGE_GROUP_TX_FAILED(const char *pId)
{
    std::cout << "D/EngageMain: on_ENGAGE_GROUP_TX_FAILED: " << pId << std::endl;
}

void on_ENGAGE_GROUP_TX_USURPED_BY_PRIORITY(const char *pId)
{
    std::cout << "D/EngageMain: on_ENGAGE_GROUP_TX_USURPED_BY_PRIORITY: " << pId << std::endl;
}

void on_ENGAGE_GROUP_MAX_TX_TIME_EXCEEDED(const char *pId)
{
    std::cout << "D/EngageMain: on_ENGAGE_GROUP_MAX_TX_TIME_EXCEEDED: " << pId << std::endl;
}

void addOrUpdatePd(ConfigurationObjects::PresenceDescriptor& pd)
{
    g_nodes.erase(pd.identity.nodeId);
    g_nodes[pd.identity.nodeId] = pd;
}

void removePd(ConfigurationObjects::PresenceDescriptor& pd)
{
    g_nodes.erase(pd.identity.nodeId);
}

void on_ENGAGE_GROUP_NODE_DISCOVERED(const char *pId, const char *pszNodeJson)
{
    ConfigurationObjects::PresenceDescriptor pd;
    if(pd.deserialize(pszNodeJson))
    {
        std::cout << "D/EngageMain: on_ENGAGE_GROUP_NODE_DISCOVERED: " << pId << ", " << pd.identity.nodeId 
                  << " : " << pszNodeJson
                  << std::endl;
        addOrUpdatePd(pd);
    }
    else
    {
       std::cout << "D/EngageMain: on_ENGAGE_GROUP_NODE_DISCOVERED: " << pId << ", COULD NOT PARSE JSON" << std::endl; 
    }    
}

void on_ENGAGE_GROUP_NODE_REDISCOVERED(const char *pId, const char *pszNodeJson)
{
    ConfigurationObjects::PresenceDescriptor pd;
    if(pd.deserialize(pszNodeJson))
    {
        std::cout << "D/EngageMain: on_ENGAGE_GROUP_NODE_REDISCOVERED: " << pId << ", " << pd.identity.nodeId 
                  << " : " << pszNodeJson
                  << std::endl;
        addOrUpdatePd(pd);
    }
    else
    {
       std::cout << "D/EngageMain: on_ENGAGE_GROUP_NODE_REDISCOVERED: " << pId << ", COULD NOT PARSE JSON" << std::endl; 
    }    
}

void on_ENGAGE_GROUP_NODE_UNDISCOVERED(const char *pId, const char *pszNodeJson)
{
    ConfigurationObjects::PresenceDescriptor pd;
    if(pd.deserialize(pszNodeJson))
    {
        std::cout << "D/EngageMain: on_ENGAGE_GROUP_NODE_UNDISCOVERED: " << pId << ", " << pd.identity.nodeId << std::endl;
        removePd(pd);
    }
    else
    {
       std::cout << "D/EngageMain: on_ENGAGE_GROUP_NODE_UNDISCOVERED: " << pId << ", COULD NOT PARSE JSON" << std::endl; 
    }    
}

void on_ENGAGE_GROUP_ASSET_DISCOVERED(const char *pId, const char *pszNodeJson)
{
    std::cout << "D/EngageMain: on_ENGAGE_GROUP_ASSET_DISCOVERED: " << pId << ", " << pszNodeJson << std::endl;

    engageJoinGroup(pId);
}

void on_ENGAGE_GROUP_ASSET_REDISCOVERED(const char *pId, const char *pszNodeJson)
{
    std::cout << "D/EngageMain: on_ENGAGE_GROUP_ASSET_REDISCOVERED: " << pId << ", " << pszNodeJson << std::endl;
}

void on_ENGAGE_GROUP_ASSET_UNDISCOVERED(const char *pId, const char *pszNodeJson)
{
    std::cout << "D/EngageMain: on_ENGAGE_GROUP_ASSET_UNDISCOVERED: " << pId << ", " << pszNodeJson << std::endl;
}

void on_ENGAGE_LICENSE_CHANGED()
{
    std::cout << "D/EngageMain: on_ENGAGE_LICENSE_CHANGED" << std::endl;
    const char *p = engageGetActiveLicenseDescriptor();
    std::cout << p << std::endl;
}

void on_ENGAGE_LICENSE_EXPIRED()
{
    std::cout << "D/EngageMain: on_ENGAGE_LICENSE_EXPIRED" << std::endl;
}

void on_ENGAGE_LICENSE_EXPIRING(const char *pSecsLeft)
{
    std::cout << "D/EngageMain: on_ENGAGE_LICENSE_EXPIRING in " << pSecsLeft << " seconds" << std::endl;
}

void on_ENGAGE_GROUP_BLOB_SENT(const char *pId)
{
    std::cout << "D/EngageMain: on_ENGAGE_GROUP_BLOB_SENT: " << pId << std::endl;
}

void on_ENGAGE_GROUP_BLOB_SEND_FAILED(const char *pId)
{
    std::cout << "D/EngageMain: on_ENGAGE_GROUP_BLOB_SEND_FAILED: " << pId << std::endl;
}

void on_ENGAGE_GROUP_BLOB_RECEIVED(const char *pId, const char *pszBlobJson, const uint8_t *blob, size_t blobSize)
{
    ConfigurationObjects::BlobInfo bi;

    if(!bi.deserialize(pszBlobJson))
    {
        std::cout << "D/EngageMain: on_ENGAGE_GROUP_BLOB_RECEIVED ERROR: Cannot parse blob info JSON" << std::endl;
        return;
    }

    // We'll make a copy to work with
    uint8_t *blobCopy = new uint8_t[blobSize];
    memcpy(blobCopy, blob, blobSize);

    if(bi.payloadType == ConfigurationObjects::BlobInfo::bptUndefined)
    {
        std::cout << "D/EngageMain: on_ENGAGE_GROUP_BLOB_RECEIVED [UNDEFINED]: " << pId << ", blobSize=" << blobSize << ", blobJson=" << pszBlobJson << std::endl;
    }
    else if(bi.payloadType == ConfigurationObjects::BlobInfo::bptAppTextUtf8)
    {
        std::cout << "D/EngageMain: on_ENGAGE_GROUP_BLOB_RECEIVED [APP TEXT UTF8]: " << pId << ", blobSize=" << blobSize << ", blobJson=" << pszBlobJson << std::endl;
    }
    else if(bi.payloadType == ConfigurationObjects::BlobInfo::bptJsonTextUtf8)
    {
        std::cout << "D/EngageMain: on_ENGAGE_GROUP_BLOB_RECEIVED [JSON TEXT UTF8]: " << pId << ", blobSize=" << blobSize << ", blobJson=" << pszBlobJson << std::endl;
    }
    else if(bi.payloadType == ConfigurationObjects::BlobInfo::bptAppBinary)
    {
        std::cout << "D/EngageMain: on_ENGAGE_GROUP_BLOB_RECEIVED [APP BINARY]: " << pId << ", blobSize=" << blobSize << ", blobJson=" << pszBlobJson << std::endl;
    }
    else if(bi.payloadType == ConfigurationObjects::BlobInfo::bptEngageBinaryHumanBiometrics)
    {
        std::cout << "D/EngageMain: on_ENGAGE_GROUP_BLOB_RECEIVED [ENGAGE HUMAN BIOMETRICS : HEART RATE]: " << pId << ", blobSize=" << blobSize << ", blobJson=" << pszBlobJson << std::endl;

        uint8_t *src = blobCopy;
        size_t bytesLeft = blobSize;

        while( bytesLeft > 0 )
        {
            ConfigurationObjects::DataSeriesHeader_t *hdr = (ConfigurationObjects::DataSeriesHeader_t*)src;        
            hdr->ts = ntohl(hdr->ts);

            src += sizeof(ConfigurationObjects::DataSeriesHeader_t);
            bytesLeft -= sizeof(ConfigurationObjects::DataSeriesHeader_t);

            if(hdr->t == ConfigurationObjects::HumanBiometricsTypes_t::heartRate)
            {
                std::cout << "\tHEART RATE: " << (int)hdr->ss << " samples" << std::endl;
            }
            else if(hdr->t == ConfigurationObjects::HumanBiometricsTypes_t::skinTemp)
            {
                std::cout << "\tSKIN TEMP: " << (int)hdr->ss << " samples" << std::endl;
            }
            else if(hdr->t == ConfigurationObjects::HumanBiometricsTypes_t::coreTemp)
            {
                std::cout << "\tCORE TEMP: " << (int)hdr->ss << " samples" << std::endl;
            }
            else if(hdr->t == ConfigurationObjects::HumanBiometricsTypes_t::hydration)
            {
                std::cout << "\tHYDRATION: " << (int)hdr->ss << " samples" << std::endl;
            }
            else if(hdr->t == ConfigurationObjects::HumanBiometricsTypes_t::bloodOxygenation)
            {
                std::cout << "\tBLOOD OXYGENATION: " << (int)hdr->ss << " samples" << std::endl;
            }
            else if(hdr->t == ConfigurationObjects::HumanBiometricsTypes_t::fatigueLevel)
            {
                std::cout << "\tFATIGUE LEVEL: " << (int)hdr->ss << " samples" << std::endl;
            }
            else if(hdr->t == ConfigurationObjects::HumanBiometricsTypes_t::taskEffectiveness)
            {
                std::cout << "\tTASK EFFECTIVENESS: " << (int)hdr->ss << " samples" << std::endl;
            }
            else
            {
                std::cout << "\tUNKNOWN BIOMETRIC: " << (int)hdr->ss << " samples" << std::endl;
            }  

            std::cout << "\t\t";

            if(hdr->vt == ConfigurationObjects::DataSeriesValueType_t::uint8)
            {
                ConfigurationObjects::DataElementUint8_t *element = (ConfigurationObjects::DataElementUint8_t*) src;

                for(uint8_t x = 0; x < hdr->ss; x++)
                {
                    std::cout << (int) element->ofs << ", " << (int) element->val << " | ";
                    element++;                    
                }

                src += (sizeof(ConfigurationObjects::DataElementUint8_t) * hdr->ss);
                bytesLeft -= (sizeof(ConfigurationObjects::DataElementUint8_t) * hdr->ss);
            }
            else if(hdr->vt == ConfigurationObjects::DataSeriesValueType_t::uint16)
            {
                // TODO : display 16-bit numbers
            }
            else if(hdr->vt == ConfigurationObjects::DataSeriesValueType_t::uint32)
            {
                // TODO : display 32-bit numbers
            }
            else if(hdr->vt == ConfigurationObjects::DataSeriesValueType_t::uint64)
            {
                // TODO : display 64-bit numbers
            }

            std::cout << std::endl;                      
        }        
    }

    delete[] blobCopy;
}

void on_ENGAGE_GROUP_RTP_SENT(const char *pId)
{
    std::cout << "D/EngageMain: on_ENGAGE_GROUP_RTP_SENT: " << pId << std::endl;
}

void on_ENGAGE_GROUP_RTP_SEND_FAILED(const char *pId)
{
    std::cout << "D/EngageMain: on_ENGAGE_GROUP_RTP_SEND_FAILED: " << pId << std::endl;
}

void on_ENGAGE_GROUP_RTP_RECEIVED(const char *pId, const char *pszRtpHeaderJson, const uint8_t *payload, size_t payloadSize)
{
    std::cout << "D/EngageMain: on_ENGAGE_GROUP_RTP_RECEIVED: " << pId << ", payloadSize=" << payloadSize << ", rtpHeaderJson=" << pszRtpHeaderJson << std::endl;
}

void on_ENGAGE_GROUP_RAW_SENT(const char *pId)
{
    std::cout << "D/EngageMain: on_ENGAGE_GROUP_RAW_SENT: " << pId << std::endl;
}

void on_ENGAGE_GROUP_RAW_SEND_FAILED(const char *pId)
{
    std::cout << "D/EngageMain: on_ENGAGE_GROUP_RAW_SEND_FAILED: " << pId << std::endl;
}

void on_ENGAGE_GROUP_RAW_RECEIVED(const char *pId, const uint8_t *raw, size_t rawSize)
{
    std::cout << "D/EngageMain: on_ENGAGE_GROUP_RAW_RECEIVED: " << pId << std::endl;
}


void on_ENGAGE_GROUP_TIMELINE_EVENT_STARTED(const char *pId, const char *eventJson)
{
    std::cout << "D/EngageMain: on_ENGAGE_GROUP_TIMELINE_EVENT_STARTED: " << pId << ":" << eventJson << std::endl;
}

void on_ENGAGE_GROUP_TIMELINE_EVENT_UPDATED(const char *pId, const char *eventJson)
{
    std::cout << "D/EngageMain: on_ENGAGE_GROUP_TIMELINE_EVENT_UPDATED: " << pId << ":" << eventJson << std::endl;
}

void on_ENGAGE_GROUP_TIMELINE_EVENT_ENDED(const char *pId, const char *eventJson)
{
    std::cout << "D/EngageMain: on_ENGAGE_GROUP_TIMELINE_EVENT_ENDED: " << pId << ":" << eventJson << std::endl;
}

void on_ENGAGE_GROUP_TIMELINE_REPORT(const char *pId, const char *reportJson)
{
    std::cout << "D/EngageMain: on_ENGAGE_GROUP_TIMELINE_REPORT: " << pId << ":" << reportJson << std::endl;
}

void on_ENGAGE_GROUP_TIMELINE_REPORT_FAILED(const char *pId)
{
    std::cout << "D/EngageMain: on_ENGAGE_GROUP_TIMELINE_REPORT_FAILED: " << pId << std::endl;
}

class Instruction
{
public:
    typedef enum {iUnknown, 
                  iGoto, 
                  iLabel, 
                  iMessage, 
                  iSleep,
                  iCreate, 
                  iDelete,
                  iJoin,
                  iLeave,
                  iBeginTx,
                  iEndTx,
                  iMuteRx,
                  iUnmuteRx,
                  iEndScript,
                  iSet,
                  iAdd,
                  iSub,
                  iCompare,
                  iOnGoto,
                  iCls,
                  iStartEngine,
                  iStopEngine
                  } Type_t;

    Type_t type;
    int intParam;
    bool randomizeInt;
    std::string stringParam;
    std::string stringParam2;

    Instruction()
    {
        clear();
    }

    void clear()
    {
        type = iUnknown;
        intParam = 0;
        randomizeInt = false;
        stringParam.clear();
        stringParam2.clear();
    }
};

std::list<Instruction> g_scriptInstructions;
std::map<std::string, int>  g_intVars;

void processIntParameter(Instruction *pIns, char *tok)
{
    printf("[%s]\n", tok);
    if(strcmp(tok, "(allgroups)") == 0)
    {
        pIns->intParam = -1;
    }
    else if(strcmp(tok, "(randomgroup)") == 0)
    {
        pIns->intParam = 0;
        pIns->randomizeInt = true;
    }
    else if(tok[0] == 'r')
    {
        tok++;
        pIns->intParam = atoi(tok);
        pIns->randomizeInt = true;
    }
    else
    {
        pIns->intParam = atoi(tok);
    }
}


bool interpret(char *line, Instruction *pIns)
{
    pIns->clear();

    // Labels are special
    if(line[0] == ':')
    {
        pIns->type = Instruction::iLabel;
        pIns->stringParam = line+1;
        return true;
    }

    static const char *SEPS = " ";
	char *tokenCtx = nullptr;
    char *tok = strtok_s(line, SEPS, &tokenCtx);

    if( strcmp(tok, "message") == 0 )
    {
        pIns->type = Instruction::iMessage;
        pIns->stringParam = tok + strlen(tok) + 1;
    }
    else if( strcmp(tok, "cls") == 0 )
    {
        pIns->type = Instruction::iCls;
    }
    else if( strcmp(tok, "endscript") == 0 )
    {
        pIns->type = Instruction::iEndScript;
    }
    else if( strcmp(tok, "goto") == 0 )
    {
        pIns->type = Instruction::iGoto;
        pIns->stringParam = tok + strlen(tok) + 1;
    }
    else if( strcmp(tok, "sleep") == 0 )
    {
        pIns->type = Instruction::iSleep;
        tok = strtok_s(nullptr, SEPS, &tokenCtx);
        processIntParameter(pIns, tok);
    }
    else if( strcmp(tok, "create") == 0 )
    {
        pIns->type = Instruction::iCreate;
        tok = strtok_s(nullptr, SEPS, &tokenCtx);
        processIntParameter(pIns, tok);
    }
    else if( strcmp(tok, "delete") == 0 )
    {
        pIns->type = Instruction::iDelete;
        tok = strtok_s(nullptr, SEPS, &tokenCtx);
        processIntParameter(pIns, tok);
    }
    else if( strcmp(tok, "join") == 0 )
    {
        pIns->type = Instruction::iJoin;
        tok = strtok_s(nullptr, SEPS, &tokenCtx);
        processIntParameter(pIns, tok);
    }
    else if( strcmp(tok, "leave") == 0 )
    {
        pIns->type = Instruction::iLeave;
        tok = strtok_s(nullptr, SEPS, &tokenCtx);
        processIntParameter(pIns, tok);
    }
    else if( strcmp(tok, "begintx") == 0 )
    {
        pIns->type = Instruction::iBeginTx;
        tok = strtok_s(nullptr, SEPS, &tokenCtx);
        processIntParameter(pIns, tok);
    }
    else if( strcmp(tok, "endtx") == 0 )
    {
        pIns->type = Instruction::iEndTx;
        tok = strtok_s(nullptr, SEPS, &tokenCtx);
        processIntParameter(pIns, tok);
    }
    else if( strcmp(tok, "muterx") == 0 )
    {
        pIns->type = Instruction::iMuteRx;
        tok = strtok_s(nullptr, SEPS, &tokenCtx);
        processIntParameter(pIns, tok);
    }
    else if( strcmp(tok, "unmuterx") == 0 )
    {
        pIns->type = Instruction::iUnmuteRx;
        tok = strtok_s(nullptr, SEPS, &tokenCtx);
        processIntParameter(pIns, tok);
    }
    else if( strcmp(tok, "set") == 0 )
    {
        pIns->type = Instruction::iSet;

        tok = strtok_s(nullptr, SEPS, &tokenCtx);
        pIns->stringParam = tok;

        tok = strtok_s(nullptr, SEPS, &tokenCtx);
        pIns->intParam = atoi(tok);
    }
    else if( strcmp(tok, "add") == 0 )
    {
        pIns->type = Instruction::iAdd;

        tok = strtok_s(nullptr, SEPS, &tokenCtx);
        pIns->stringParam = tok;

        tok = strtok_s(nullptr, SEPS, &tokenCtx);
        pIns->intParam = atoi(tok);
    }
    else if( strcmp(tok, "sub") == 0 )
    {
        pIns->type = Instruction::iSub;

        tok = strtok_s(nullptr, SEPS, &tokenCtx);
        pIns->stringParam = tok;

        tok = strtok_s(nullptr, SEPS, &tokenCtx);
        pIns->intParam = atoi(tok);
    }
    else if( strcmp(tok, "on") == 0 )
    {
        pIns->type = Instruction::iOnGoto;

        tok = strtok_s(nullptr, SEPS, &tokenCtx);
        pIns->stringParam = tok;

        tok = strtok_s(nullptr, SEPS, &tokenCtx);
        pIns->intParam = atoi(tok);

        tok = strtok_s(nullptr, SEPS, &tokenCtx);
        if(strcmp(tok, "goto") != 0)
        {
            return false;
        }

        tok = strtok_s(nullptr, SEPS, &tokenCtx);
        pIns->stringParam2 = tok;
    }
    else if( strcmp(tok, "enginestart") == 0 )
    {
        pIns->type = Instruction::iStartEngine;
    }
    else if( strcmp(tok, "enginestop") == 0 )
    {
        pIns->type = Instruction::iStopEngine;
    }
    else
    {
        return false;
    }

    return true;
}

bool loadScript(const char *pszFile)
{
	FILE *fp;

	#ifndef WIN32
		fp = fopen(pszFile, "rt");
	#else
		if (fopen_s(&fp, pszFile, "rt") != 0)
		{
			fp = nullptr;
		}
	#endif

    if(fp == nullptr)
    {
        std::cerr << "cannot load script file '" << pszFile << "'" << std::endl;
        return false;
    }

    char tmp[1024];
    char *lineStart;
    char *lineEnd;
    bool pLd = true;

    while( fgets(tmp, sizeof(tmp), fp) )
    {
        // Strip leading space
        lineStart = tmp;
        while(*lineStart != 0 && (*lineStart == '\n' || *lineStart == '\r' || *lineStart == '\t' || *lineStart == ' '))
        {
            lineStart++;
        }        

        // Skip empty lines and comments
        if(*lineStart == 0 || *lineStart == '#')
        {
            continue;
        }

        // Strip trailing junk
        lineEnd = lineStart + strlen(lineStart) - 1;
        while(lineEnd >= lineStart && (*lineEnd == '\n' || *lineEnd == '\r'))
        {
            *lineEnd = 0;
            lineEnd--;
        }

        if(lineEnd <= lineStart)
        {
            continue;
        }

        if(strlen(lineStart) == 0)
        {
            continue;
        }

        // OK, we have something valid, let's see what it is
        Instruction ins;

        ins.clear();
        if(!interpret(lineStart, &ins))
        {
            pLd = false;
            break;
        }

        // Add the instruction
        g_scriptInstructions.push_back(ins);
    }

    fclose(fp);

    return pLd;
}

std::string replaceAll(std::string str, const std::string& from, const std::string& to) 
{
    size_t start_pos = 0;

    while((start_pos = str.find(from, start_pos)) != std::string::npos) 
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }

    return str;
}

std::string expandString(const char *s)
{
    size_t tokenStart = 0;
    std::string pLd;

    pLd = s;

    while( true )
    {
        tokenStart = pLd.find("${", tokenStart);
        if(tokenStart == std::string::npos)
        {
            break;
        }
        size_t tokenEnd = pLd.find("}", tokenStart);
        if(tokenEnd == std::string::npos)
        {
            break;
        }

        std::string varName = pLd.substr(tokenStart + 2, (tokenEnd - tokenStart - 2));
        std::string replSeapLdh = "${" + varName + "}";
        std::string replReplace;
        if(!varName.empty())
        {
            std::map<std::string, int>::iterator    itr = g_intVars.find(varName);
            if(itr != g_intVars.end())
            {
                char tmp[64];
                sprintf_s(tmp, sizeof(tmp), "%d", itr->second);
                replReplace = tmp;
            }
        }

        pLd = replaceAll(pLd, replSeapLdh, replReplace);
    }

    return pLd;
}

bool setVar(const char *nm, int val)
{
    if(g_verboseScript) std::cout << "setVar: " << nm << ", " << val << std::endl;
    std::map<std::string, int>::iterator    itr = g_intVars.find(nm);
    if(itr != g_intVars.end())
    {
        itr->second = val;
    }
    else
    {
        g_intVars[nm] = val;
    }    

    return true;
}

bool addVar(const char *nm, int val)
{
    if(g_verboseScript) std::cout << "addVar: " << nm << ", " << val << std::endl;
    std::map<std::string, int>::iterator    itr = g_intVars.find(nm);
    if(itr != g_intVars.end())
    {
        itr->second += val;
    }
    else
    {
        g_intVars[nm] = val;
    }    

    return true;
}

bool subVar(const char *nm, int val)
{
    if(g_verboseScript) std::cout << "subVar: " << nm << ", " << val << std::endl;
    std::map<std::string, int>::iterator    itr = g_intVars.find(nm);
    if(itr != g_intVars.end())
    {
        itr->second -= val;
    }
    else
    {
        g_intVars[nm] = val;
    }    

    return true;
}

bool compVar(const char *nm, int comp, int *pResult)
{
    std::map<std::string, int>::iterator    itr = g_intVars.find(nm);
    if(itr != g_intVars.end())
    {
        if(itr->second == comp)
        {
            *pResult = 0;
        }
        else if(itr->second < comp)
        {
            *pResult = -1;
        }
        else
        {
            *pResult = 1;
        }        
    }
    else
    {
        return false;
    }    

    return true;
}

bool runScript()
{
    bool endScript = false;
    bool pLd = false;
    bool error = false;
    std::list<Instruction>::iterator itrIns;

    itrIns = g_scriptInstructions.begin();
    while( !error && !endScript && itrIns != g_scriptInstructions.end() )
    {
        Instruction ins = *itrIns;
        switch( ins.type )
        {
            case Instruction::iUnknown:
            {
                std::cerr << "unknown instruction" << std::endl;
            }
            break;

            case Instruction::iEndScript:            
            {
                if(g_verboseScript) std::cout << "endscript" << std::endl;
                endScript = true;
                pLd = true;
            }
            break;

            case Instruction::iCls:            
            {
                if(g_verboseScript) std::cout << "cls" << std::endl;
                #ifdef WIN32
                    system("cls");
                #else
                    if(system("clear") != 0)
                    {
                        std::cout << "system('clear') failed" << std::endl;
                    }
                #endif
                itrIns++;
            }
            break;

            case Instruction::iLabel:
            {
                if(g_verboseScript) std::cout << "label: " << ins.stringParam << std::endl;
                itrIns++;
            }
            break;

            case Instruction::iGoto:
            {
                if(g_verboseScript) std::cout << "goto: " << ins.stringParam << std::endl;
                std::list<Instruction>::iterator itrFind;
                for(itrFind = g_scriptInstructions.begin();
                    itrFind != g_scriptInstructions.end();
                    itrFind++)
                {
                    if(itrFind->type == Instruction::iLabel && itrFind->stringParam.compare(ins.stringParam) == 0)
                    {
                        break;
                    }
                }

                if(itrFind == g_scriptInstructions.end())
                {
                    std::cerr << "goto: label '" << ins.stringParam << "' not found!" << std::endl;
                }

                itrIns = itrFind;
            }
            break;

            case Instruction::iMessage:
            {                
                if(g_verboseScript) std::cout << "message: " << ins.stringParam << std::endl;
                std::string expanded = expandString(ins.stringParam.c_str());
                std::cout << "[SCR] " << expanded << std::endl;
                itrIns++;
            }
            break;

            case Instruction::iSleep:
            {
                int ms = (ins.randomizeInt) ? rand() % ins.intParam : ins.intParam;
                if(g_verboseScript) std::cout << "sleep: " << ms << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(ms));
                itrIns++;
            }
            break;

            case Instruction::iCreate:
            {
                doCreate((ins.randomizeInt) ? rand() % g_new_groups.size() : ins.intParam);
                itrIns++;
            }
            break;

            case Instruction::iDelete:
            {
                doDelete((ins.randomizeInt) ? rand() % g_new_groups.size() : ins.intParam);                
                itrIns++;
            }
            break;

            case Instruction::iJoin:
            {
                doJoin((ins.randomizeInt) ? rand() % g_new_groups.size() : ins.intParam);
                itrIns++;
            }
            break;

            case Instruction::iLeave:
            {
                doLeave((ins.randomizeInt) ? rand() % g_new_groups.size() : ins.intParam);
                itrIns++;
            }
            break;
     
            case Instruction::iBeginTx:
            {
                doBeginTx((ins.randomizeInt) ? rand() % g_new_groups.size() : ins.intParam);
                itrIns++;
            }
            break;

            case Instruction::iEndTx:
            {
                doEndTx((ins.randomizeInt) ? rand() % g_new_groups.size() : ins.intParam);
                itrIns++;
            }
            break;

            case Instruction::iMuteRx:
            {
                doMuteRx((ins.randomizeInt) ? rand() % g_new_groups.size() : ins.intParam);
                itrIns++;
            }
            break;

            case Instruction::iUnmuteRx:
            {
                doUnmuteRx((ins.randomizeInt) ? rand() % g_new_groups.size() : ins.intParam);
                itrIns++;
            }
            break;

            case Instruction::iSet:
            {
                error = !setVar(ins.stringParam.c_str(), ins.intParam);
                itrIns++;                    
            }
            break;

            case Instruction::iAdd:
            {
                error = !addVar(ins.stringParam.c_str(), ins.intParam);
                itrIns++;                    
            }
            break;

            case Instruction::iSub:
            {
                error = !subVar(ins.stringParam.c_str(), ins.intParam * -1);
                itrIns++;                    
            }
            break;

            case Instruction::iCompare:
            {
                int result = 0;
                error = !compVar(ins.stringParam.c_str(), ins.intParam, &result);
                if(!error)
                {
                    // what do with the comparison result
                }
                itrIns++;
            }
            break;        

            case Instruction::iOnGoto:
            {
                if(g_verboseScript) std::cout << "ongoto: " << ins.stringParam << std::endl;
                std::map<std::string, int>::iterator    itr = g_intVars.find(ins.stringParam);
                if(itr == g_intVars.end())
                {
                    std::cerr << "var: '" << ins.stringParam << "' not found!" << std::endl;
                    error = true;
                }
                else
                {
                    if(itr->second == itrIns->intParam)
                    {
                        std::list<Instruction>::iterator itrFind;
                        for(itrFind = g_scriptInstructions.begin();
                            itrFind != g_scriptInstructions.end();
                            itrFind++)
                        {
                            if(itrFind->type == Instruction::iLabel && itrFind->stringParam.compare(ins.stringParam2) == 0)
                            {
                                break;
                            }
                        }

                        if(itrFind == g_scriptInstructions.end())
                        {
                            std::cerr << "ongoto: label '" << ins.stringParam2 << "' not found!" << std::endl;
                        }

                        itrIns = itrFind;
                    }
                    else
                    {
                        itrIns++;
                    }                    
                }                
            }
            break;

            case Instruction::iStartEngine:
            {
                doStartEngine();
                itrIns++;
            }
            break;

            case Instruction::iStopEngine:
            {
                doStopEngine();
                itrIns++;
            }
            break;
        }
    }

    return pLd;
}

bool registepLdallbacks()
{
    EngageEvents_t cb;

    memset(&cb, 0, sizeof(cb));

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

    cb.PFN_ENGAGE_GROUP_MEMBER_COUNT_CHANGED = on_ENGAGE_GROUP_MEMBER_COUNT_CHANGED;

    cb.PFN_ENGAGE_GROUP_RX_STARTED = on_ENGAGE_GROUP_RX_STARTED;
    cb.PFN_ENGAGE_GROUP_RX_ENDED = on_ENGAGE_GROUP_RX_ENDED;
    cb.PFN_ENGAGE_GROUP_RX_SPEAKERS_CHANGED = on_ENGAGE_GROUP_RX_SPEAKERS_CHANGED;
    cb.PFN_ENGAGE_GROUP_RX_MUTED = on_ENGAGE_GROUP_RX_MUTED;
    cb.PFN_ENGAGE_GROUP_RX_UNMUTED = on_ENGAGE_GROUP_RX_UNMUTED;

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

    return (engageRegisterEventCallbacks(&cb) == ENGAGE_RESULT_OK);
}
