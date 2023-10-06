#pragma once

#include <string>
#include "weather.h"

namespace HassWx
{
    extern std::string apiUrl;
    extern std::string apiToken;
    extern std::string windDirSensor;
    extern std::string windSpdSensor;
    extern std::string windGustSensor;
    extern std::string tempSensor;
    extern std::string rainHrSensor;
    extern std::string rain24hSensor;
    extern std::string rainDaySensor;
    extern std::string humSensor;
    extern std::string pressSensor;
    extern std::string lumSensor;
    extern std::string timeStampSensor;
    extern int maxDataAge;

    Weather::Report GetReport();
}