#ifndef INC_CONFIG_H
#define INC_CONFIG_H

#include <time.h>
#include <vector>
#include <string>
#include "beacon.h"

namespace Config
{
    const float version = 1.00;

    enum HardwareType
    {
        Generic,
        OrangeGate_v1,
        PiCrumbsMini_v1
    };

    class PttConfig
    {
        public:
            bool enabled;
            unsigned int timeout;
    };

    class ADC
    {
        public:
            float scale;
            float offset;
            unsigned int precision;
            char* fileName;
    };

    extern std::vector<PttConfig> ptt;
    extern std::vector<Beacon::Beacon> beacons;
    extern std::vector<ADC> adc;
    extern char tempUnit;
    extern char* tempFile;
    extern unsigned int tempPrecision;
    extern std::string myCall;
    extern bool verbose;
    extern bool debug;
    extern HardwareType hardwareType;

    int Parse(void* user, const char* section, const char* name, const char* value);
}

#endif