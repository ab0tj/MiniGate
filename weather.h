#pragma once
#include <string>

namespace Weather
{
    enum WXSource { WX_None, WX_Hass, WX_MQTT };
    extern WXSource source;

    struct Report
    {
        bool valid;
        bool hasTemperature;
        int temperature;        // Temperature in degrees fahrenheit
        bool hasHumidity;
        unsigned int humidity;  // Humidity in percent
        bool hasWindDir;
        unsigned int windDir;   // Wind direction in degrees
        bool hasWindSpd;
        unsigned int windSpd;   // Wind speed in mph
        bool hasWindGust;
        unsigned int windGust;  // Peak wind speed in the last 5 minutes, in mph
        bool hasRainHour;
        unsigned int rainHour;  // Rainfall in the last hour, in hundreths of an inch
        bool hasRain24h;
        unsigned int rain24h;   // Rainfall in the last 24 hours, in hundreths of an inch
        bool hasRainSinceMidnight;
        unsigned int rainSinceMidnight; // Rainfall since midnight, in hundreths of an inch
        bool hasPressure;
        unsigned int pressure;  // Barometric pressure in millibars
        bool hasLuminosity;
        unsigned int luminosity;    // Luminosity in W/sq. meter
        bool hasSnowfall;
        unsigned int snowfall;  // Snowfall in the past 24 hours, in inches
        bool hasRawRainCtr;
        unsigned int rawRainCtr;    // Raw rain counter
        bool hasTimestamp;
        unsigned int timestampMonth;
        unsigned int timestampDay;
        unsigned int timestampHour;
        unsigned int timestampMinute;

        std::string ToAPRS(bool positionless);
    };

    void Init(void (*debugFunc)(const std::string&));
    Report GetReport();
    std::string GetReportStr(bool positionless);
}