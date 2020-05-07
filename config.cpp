#include <string.h>
#include <stdlib.h>
#include "config.h"

namespace Config
{
    std::vector<PttConfig> ptt;
    std::vector<Beacon::Beacon> beacons;
    std::vector<ADC> adc;
    char tempUnit;
    char* tempFile;
    unsigned int tempPrecision;
    std::string myCall;
    bool verbose;
    bool debug;
    HardwareType hardwareType;

    int Parse(void* user, const char* section, const char* name, const char* value)
    {
        std::string s_section = std::string(section);
        std::string s_name = std::string(name);
        std::string s_value = std::string(value);
        uint secNum;

        // ptt: enabled, timeout
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
        
        else if (s_section.substr(0, 6).compare("beacon") == 0)
        {
            if (s_section.length() < 7) return 0;
            secNum = std::stoi(s_section.substr(6));
            if (secNum >= beacons.size()) beacons.resize(secNum + 1);

            if (s_name.compare("text") == 0)
            {
                beacons[secNum].text = s_value;
            }
            else if (s_name.compare("interval") == 0)
            {
                beacons[secNum].interval = atoi(value);
            }
            else if (s_name.compare("file") == 0)
            {
                beacons[secNum].fileName = value;
            }
            else return 0;
        }

        else if (MATCH("adc0", "scale"))
        {
            adc[0].scale = atof(value);
        }
        else if (MATCH("adc0", "precision"))
        {
            adc[0].precision = atoi(value);
        }
        else if (MATCH("adc0", "offset"))
        {
            adc[0].offset = atof(value);
        }
        else if (MATCH("adc1", "scale"))
        {
            adc[1].scale = atof(value);
        }
        else if (MATCH("adc1", "precision"))
        {
            adc[1].precision = atoi(value);
        }
        else if (MATCH("adc1", "offset"))
        {
            adc[1].offset = atof(value);
        }
        else if (MATCH("adc2", "scale"))
        {
            adc[2].scale = atof(value);
        }
        else if (MATCH("adc2", "precision"))
        {
            adc[2].precision = atoi(value);
        }
        else if (MATCH("adc2", "offset"))
        {
            adc[2].offset = atof(value);
        }
        else if (MATCH("adc2", "file"))
        {
            adc[2].fileName = (char*)malloc(strlen(value) + 1);
            strcpy(adc[2].fileName, value);
        }
        else if (MATCH("adc3", "scale"))
        {
            adc[1].scale = atof(value);
        }
        else if (MATCH("adc3", "precision"))
        {
            adc[1].precision = atoi(value);
        }
        else if (MATCH("adc3", "offset"))
        {
            adc[1].offset = atof(value);
        }
        else if (MATCH("adc3", "file"))
        {
            adc[3].fileName = (char*)malloc(strlen(value) + 1);
            strcpy(adc[3].fileName, value);
        }
        else if (MATCH("temp", "unit"))
        {
            tempUnit = value[0];
        }
        else if (MATCH("temp", "file"))
        {
            tempFile = (char*)malloc(strlen(value) + 1);
            strcpy(tempFile, value);
        }
        else if (MATCH("temp", "precision"))
        {
            tempPrecision = atoi(value);
        }
        else if (MATCH("station", "mycall"))
        {
            myCall = (char*)malloc(strlen(value) + 1);
            strcpy(myCall, value);
        }
        else return 0;

        return 1;
    }
}