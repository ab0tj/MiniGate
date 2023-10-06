#include <sstream>
#include <iomanip>
#include "weather.h"
#include "hass_wx.h"
#include "beacon.h"
#include "config.h"

namespace Weather
{
    WXSource source;
    void (*debugFunction)(const std::string&);

    void Init(void (*debugFunc)(const std::string&))
    {
        debugFunction = debugFunc;
    }

    Report GetReport()
    {
        switch (source)
        {
            case WX_Hass:
                return HassWx::GetReport();
            
            default:
            {
                Report r;
                r.valid = false;
                return r;
            }
        }
    }

    std::string GetReportStr(bool positionless)
    {
        return GetReport().ToAPRS(positionless);
    }

    std::string Report::ToAPRS(bool positionless)
    {
        std::stringstream ss;
        ss << std::setfill('0');

        if (positionless) ss << '_';
        else ss << '@';

        if (hasTimestamp)
        {
            if (positionless) ss << std::setw(2) << timestampMonth;
            ss << std::setw(2) << timestampDay << std::setw(2) << timestampHour << std::setw(2) << timestampMinute;
            if (!positionless) ss << 'z';
        }
        else
        {
            if (positionless) ss << Beacon::GetZulu(true);
            else ss << Beacon::GetZulu(false) << 'z';
        }

        if (!positionless) ss << Config::myLat << '/' << Config::myLon << '_';

        if (positionless) ss << 'c';
        if (hasWindDir) ss << std::setw(3) << windDir;
        else ss << "...";

        if (positionless) ss << 's';
        else ss << '/';
        if (hasWindSpd) ss << std::setw(3) << windSpd;
        else ss << "...";

        ss << 'g';
        if (hasWindGust) ss << std::setw(3) << windGust;
        else ss << "...";

        ss << 't';
        if (hasTemperature)
        {
            int temp = temperature;
            if (temp < -99) temp = -99;
            ss << std::setw(3) << temp;
        }
        else ss << "...";

        if (hasRainHour) ss << 'r' << std::setw(3) << rainHour;

        if (hasRain24h) ss << 'p' << std::setw(3) << rain24h;

        if (hasLuminosity)
        {
            if (luminosity < 1000) ss << 'L' << std::setw(3) << luminosity;
            else ss << 'l' << std::setw(3) << (luminosity - 1000);
        }
        else if (hasRainSinceMidnight) ss << 'P' << std::setw(3) << rainSinceMidnight;

        if (hasHumidity)
        {
            unsigned int hum = humidity;
            if (hum < 1) hum = 1;
            else if (hum >= 100) hum = 0;
            ss << 'h' << std::setw(2) << hum;
        }

        if (hasPressure) ss << 'b' << std::setw(5) << pressure;

        return ss.str();
    }
}