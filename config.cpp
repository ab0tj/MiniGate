#include <string.h>
#include <stdlib.h>
#include <iostream>
#include "config.h"
#include "beacon.h"
#include "weather.h"
#include "hass_wx.h"
#include "victron.h"

namespace Config
{
    std::vector<PttConfig> ptt;
    std::string myCall;
    std::string myLat, myLon;
    bool verbose;
    bool debug;

    int Parse(void* user, const char* section, const char* name, const char* value)
    {
        std::string s_section(section);
        std::string s_name(name);
        std::string s_value(value);
        uint secNum;

        if (debug) std::cout << "Config parse: " << section << ',' << name << ',' << value << std::endl;

        /* PTT config */
        if (s_section.substr(0, 3).compare("ptt") == 0)
        {
            if (s_section.length() < 4) return 0;
            secNum = std::stoi(s_section.substr(3));    // Get PTT number
            if (secNum >= ptt.size()) ptt.resize(secNum + 1);   // Resize vector if necessary

            if (s_name.compare("enabled") == 0)
            {
                ptt[secNum].enabled = (s_value.compare("true") == 0);
            }
            else if (s_name.compare("timeout") == 0)
            {
                ptt[secNum].timeout = atoi(value);
            }
            else return 0;
        }

        /* Beacon file config */
        else if (s_section.compare("beacon_file") == 0)
        {
            if (s_name.compare("dir") == 0)
            {
                Beacon::beaconFilePath = s_value;
            }
        }
        
        /* Beacon config */
        else if (s_section.substr(0, 6).compare("beacon") == 0)
        {
            if (s_section.length() < 7) return 0;
            secNum = std::stoi(s_section.substr(6));
            if (secNum >= Beacon::beacons.size()) Beacon::beacons.resize(secNum + 1);

            if (s_name.compare("text") == 0)
            {
                Beacon::beacons[secNum].text = s_value;
            }
            else if (s_name.compare("file") == 0)
            {
                Beacon::beacons[secNum].fileName = s_value;
            }
            else if (s_name.compare("interval") == 0)
            {
                Beacon::beacons[secNum].interval = atoi(value);
            }
            else return 0;
        }

        /* Sensor config */
        else if (s_section.substr(0, 6).compare("sensor") == 0)
        {
            if (s_section.length() < 7) return 0;
            secNum = std::stoi(s_section.substr(6));
            if (secNum >= Sensor::sensors.size()) Sensor::sensors.resize(secNum + 1);

            if (s_name.compare("scale") == 0)
            {
                Sensor::sensors[secNum].scale = atof(value);
            }
            else if (s_name.compare("precision") == 0)
            {
                Sensor::sensors[secNum].precision = atoi(value);
            }
            else if (s_name.compare("offset") == 0)
            {
                Sensor::sensors[secNum].offset = atof(value);
            }
            else if (s_name.compare("file") == 0 || s_name.compare("value_name") == 0)
            {
                Sensor::sensors[secNum].locator = value;
            }
            else if (s_name.compare("mcu_adc_num") == 0)
            {
                Sensor::sensors[secNum].mcuAdcNum = atoi(value);
            }
            else if (s_name.compare("type") == 0)
            {
                if (s_value.compare("mcu_adc") == 0)
                {
                    Sensor::sensors[secNum].type = Sensor::Sensor_MCU_ADC;
                }
                else if (s_value.compare("file") == 0)
                {
                    Sensor::sensors[secNum].type = Sensor::Sensor_File;
                }
                else if (s_value.compare("victron") == 0)
                {
                    Sensor::sensors[secNum].type = Sensor::Sensor_Victron;
                }
                else Sensor::sensors[secNum].type = Sensor::Sensor_None;
            }
            else if (s_name.compare("raw_offset") == 0)
            {
                Sensor::sensors[secNum].rawOffset = atof(value);
            }
            else if (s_name.compare("min_raw_val") == 0)
            {
                Sensor::sensors[secNum].minRawVal = atof(value);
            }
            else if (s_name.compare("max_raw_val") == 0)
            {
                Sensor::sensors[secNum].maxRawVal = atof(value);
            }
            else if (s_name.compare("zero_offset") == 0)
            {
                Sensor::sensors[secNum].zeroOffset = atof(value);
            }
            else if (s_name.compare("max_read_attempts") == 0)
            {
                Sensor::sensors[secNum].maxReadAttempts = atoi(value);
            }
            else if (s_name.compare("allow_negative") == 0)
            {
                Sensor::sensors[secNum].allowNegative = (strcmp(value, "false") != 0);
            }
            else if (s_name.compare("sample_rate") == 0)
            {
                Sensor::sensors[secNum].sampleRate = atoi(value);
            }
            else if (s_name.compare("avg") == 0)
            {
                Sensor::sensors[secNum].avgSamples = atoi(value);
            }
            else return 0;
        }

        /* Station config */
        else if (s_section.compare("station") == 0)
        {
            if (s_name.compare("mycall") == 0)
            {
                myCall = value;
            }
            else if (s_name.compare("lat") == 0)
            {
                myLat = value;
            }
            else if (s_name.compare("lon") == 0)
            {
                myLon = value;
            }
            else return 0;
        }

        else if (s_section.compare("weather") == 0)
        {
            if (s_name.compare("source") == 0)
            {
                if (s_value.compare("hass") == 0) Weather::source = Weather::WX_Hass;
                else if (s_value.compare("mqtt") == 0) Weather::source = Weather::WX_MQTT;
                else Weather::source = Weather::WX_None;
            }
        }

        else if (s_section.compare("hass_wx") == 0)
        {
            if (s_name.compare("api_url") == 0)
            {
                std::string s = value;
                if (s.length() > 0 && s.back() == '/') s.erase(s.end() - 1);
                HassWx::apiUrl = s;
            }
            else if (s_name.compare("token") == 0)
            {
                HassWx::apiToken = value;
            }
            else if (s_name.compare("wind_dir") == 0)
            {
                HassWx::windDirSensor = value;
            }
            else if (s_name.compare("wind_spd") == 0)
            {
                HassWx::windSpdSensor = value;
            }
            else if (s_name.compare("wind_gust") == 0)
            {
                HassWx::windGustSensor = value;
            }
            else if (s_name.compare("temp") == 0)
            {
                HassWx::tempSensor = value;
            }
            else if (s_name.compare("rain_hr") == 0)
            {
                HassWx::rainHrSensor = value;
            }
            else if (s_name.compare("rain_24h") == 0)
            {
                HassWx::rain24hSensor = value;
            }
            else if (s_name.compare("day_rain") == 0)
            {
                HassWx::rainDaySensor = value;
            }
            else if (s_name.compare("humidity") == 0)
            {
                HassWx::humSensor = value;
            }
            else if (s_name.compare("press") == 0)
            {
                HassWx::pressSensor = value;
            }
            else if (s_name.compare("luminosity") == 0)
            {
                HassWx::lumSensor = value;
            }
            else if (s_name.compare("timestamp") == 0)
            {
                HassWx::timeStampSensor = value;
            }
            else if (s_name.compare("max_age") == 0)
            {
                HassWx::maxDataAge = atoi(value);
            }
            else return 0;
        }

        else if (s_section.compare("victron") == 0)
        {
            if (s_name.compare("serial_port") == 0)
            {
                Sensor::victronSerialPort = value;
            }
        }

        else return 0;

        return 1;
    }
}