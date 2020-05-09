#ifndef INC_CONFIG_H
#define INC_CONFIG_H

#include <time.h>
#include <vector>
#include <string>
#include "beacon.h"
#include "sensor.h"

namespace Config
{
    const float version = 1.00;
    const std::string seqFile{"/tmp/minigate.seq"};

    class PttConfig
    {
        public:
            bool enabled;
            unsigned int timeout;
    };

    extern std::vector<PttConfig> ptt;
    extern std::string myCall;
    extern bool verbose;
    extern bool debug;

    int Parse(void* user, const char* section, const char* name, const char* value);
}

#endif