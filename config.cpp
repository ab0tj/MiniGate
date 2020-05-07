#include <string.h>
#include <stdlib.h>
#include "config.h"
#include "beacon.h"

namespace Config
{
    std::vector<PttConfig> ptt;
    std::string myCall;
    bool verbose;
    bool debug;

    int Parse(void* user, const char* section, const char* name, const char* value)
    {
        std::string s_section = std::string(section);
        std::string s_name = std::string(name);
        std::string s_value = std::string(value);
        uint secNum;

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
            else if (s_name.compare("interval") == 0)
            {
                Beacon::beacons[secNum].interval = atoi(value);
            }
            else if (s_name.compare("file") == 0)
            {
                Beacon::beacons[secNum].fileName = value;
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
            else if (s_name.compare("file") == 0)
            {
                Sensor::sensors[secNum].fileName = value;
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
                else if (s_value.compare("hwmon") == 0)
                {
                    Sensor::sensors[secNum].type = Sensor::Sensor_HWMon;
                }
                else Sensor::sensors[secNum].type = Sensor::Sensor_None;
            }
            else if (s_name.compare("unit") == 0)
            {
                Sensor::sensors[secNum].unit = value[0];
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
            else return 0;
        }

        else return 0;

        return 1;
    }
}