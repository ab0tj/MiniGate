#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <list>
#include <sstream>
#include "hass_wx.h"
#include "json.hpp"
using json = nlohmann::json;

namespace HassWx
{
    std::string apiUrl;
    std::string apiToken;
    std::string windDirSensor;
    std::string windSpdSensor;
    std::string windGustSensor;
    std::string tempSensor;
    std::string rainHrSensor;
    std::string rain24hSensor;
    std::string rainDaySensor;
    std::string humSensor;
    std::string pressSensor;
    std::string lumSensor;
    std::string timeStampSensor;
    int maxDataAge;

    std::string GetState(std::string& sensor, curlpp::Easy& curl)
    {
        std::ostringstream result;
        curl.setOpt(curlpp::options::WriteStream(&result));
        json j;

        try
        {
            curl.setOpt(curlpp::options::Url(apiUrl + "/states/" + sensor));
            curl.perform();
        }
        catch(const std::exception& e)
        {
            std::cerr << "HassWx: CURL error: " << e.what() << '\n';
            return "";
        }

        try
        {
            j = json::parse(result.str());
            return j["state"].get<std::string>();
        }
        catch(const std::exception& e)
        {
            std::cerr << "HassWx: JSON error: " << e.what() << '\n';
            return "";
        }
    }

    bool GetSensor(std::string& sensor, curlpp::Easy& curl, long& result)
    {
        if (sensor.empty()) return false;

        try
        {
	    std::string s = GetState(sensor, curl);
	        if (s.empty()) return false;
            result = std::stoi(s);
            return true;
        }
        catch(const std::exception& e)
        {
            std::cerr << "HassWx: Error parsing result into int: " << e.what() << '\n';
            return false;
        }
    }

    bool GetSensor(std::string& sensor, curlpp::Easy& curl, float& result)
    {
        if (sensor.empty()) return false;

        try
        {
	    std::string s = GetState(sensor, curl);
	        if (s.empty()) return false;
            result = std::stof(s);
            return true;
        }
        catch(const std::exception& e)
        {
            std::cerr << "HassWx: Error parsing result into float: " << e.what() << '\n';
            return false;
        }
    }

    Weather::Report GetReport()
    {
        Weather::Report report;
        time_t ts;
        float state;

        report.valid = false;
        if (apiUrl.length() == 0 || apiToken.length() == 0 || timeStampSensor.length() == 0) return report;

        /* Set up CURL */
        curlpp::Cleanup curlCleanup;
        curlpp::Easy curl;
        std::list<std::string> headers;
        headers.push_back("Content-Type: application/json");
        headers.push_back("Authorization: Bearer " + apiToken);
        curl.setOpt(curlpp::options::HttpHeader(headers));

        /* Get timestamp */
        if (GetSensor(timeStampSensor, curl, ts))
        {
            if ((time(NULL) - ts) > maxDataAge) return report;
            struct tm* timestamp = gmtime(&ts);
            report.timestampMonth = timestamp->tm_mon;
            report.timestampDay = timestamp->tm_mday;
            report.timestampHour = timestamp->tm_hour;
            report.timestampMinute = timestamp->tm_min;
            report.hasTimestamp = true;
        }
        else
        {
            report.hasTimestamp = false;
        }

        /* Get wind direction */
        if (GetSensor(windDirSensor, curl, state))
        {
            report.windDir = std::lround(state);
            report.hasWindDir = true;
        }
        else
        {
            report.hasWindDir = false;
        }

        /* Get wind speed */
        if (GetSensor(windSpdSensor, curl, state))
        {
            report.windSpd = std::lround(state);
            report.hasWindSpd = true;
        }
        else
        {
            report.hasWindSpd = false;
        }

        /* Get wind gust */
        if (GetSensor(windGustSensor, curl, state))
        {
            report.windGust = std::lround(state);
            report.hasWindGust = true;
        }
        else
        {
            report.hasWindGust = false;
        }

        /* Get temperature */
        if (GetSensor(tempSensor, curl, state))
        {
            report.temperature = std::lround(state);
            report.hasTemperature = true;
        }
        else
        {
            report.hasTemperature = false;
        }

        /* Get hour rain */
        if (GetSensor(rainHrSensor, curl, state))
        {
            report.rainHour = std::lround(state*100);
            report.hasRainHour = true;
        }
        else
        {
            report.hasRainHour = false;
        }

        /* Get rain 24h */
        if (GetSensor(rain24hSensor, curl, state))
        {
            report.rain24h = std::lround(state*100);
            report.hasRain24h = true;
        }
        else
        {
            report.hasRain24h = false;
        }

        /* Get rain since midnight */
        if (GetSensor(rainDaySensor, curl, state))
        {
            report.rainSinceMidnight = std::lround(state*100);
            report.hasRainSinceMidnight = true;
        }
        else
        {
            report.hasRainSinceMidnight = false;
        }

        /* Get humidity */
        if (GetSensor(humSensor, curl, state))
        {
            state = round(state);
            report.humidity = state;
            report.hasHumidity = true;
        }
        else
        {
            report.hasHumidity = false;
        }

        /* Get barometric pressure */
        if (GetSensor(pressSensor, curl, state))
        {
            state *= 338.63886666667;   // Convert inHg to tenths of millibars
            report.pressure = std::lround(state);
            report.hasPressure = true;
        }
        else
        {
            report.hasPressure = false;
        }

        if (GetSensor(lumSensor, curl, state))
        {
            report.luminosity = std::lround(state);
            report.hasLuminosity = true;
        }
        else
        {
            report.hasLuminosity = false;
        }

        /* TODO: Support snow and raw rain counter? */
        report.hasSnowfall = false;
        report.hasRawRainCtr = false;
        report.valid = true;

        return report;
    }
}
