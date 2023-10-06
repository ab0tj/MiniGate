#pragma once

#include <time.h>
#include <vector>
#include <string>
#include "beacon.h"
#include "sensor.h"

namespace Config
{
    const float version = 1.10;
    const std::string defaultConfigFile{"/etc/minigate.conf"};

    class PttConfig
    {
        public:
            bool enabled;
            unsigned int timeout;
    };

    extern std::vector<PttConfig> ptt;
    extern std::string myCall;
    extern std::string myLat, myLon;
    extern bool debug;

    int Parse(void* user, const char* section, const char* name, const char* value);
}