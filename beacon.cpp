#include <time.h>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <experimental/filesystem>
#include <string.h> // strerror
#include "config.h"
#include "util.h"
#include "sensor.h"
#include "mcu.h"
#include "weather.h"
#include "beacon.h"

namespace Beacon
{
    std::vector<Beacon> beacons;
    std::string beaconFilePath = defaultBeaconFilePath;

    void Init()
    {
        /* Clean up path variable */
        if (beaconFilePath.length() == 0) beaconFilePath = defaultBeaconFilePath;
        if (beaconFilePath.back() != '/') beaconFilePath.append("/");

        /* Create beacon directory if it does not already exist */
        if (!std::experimental::filesystem::is_directory(beaconFilePath))
        {
            bool res = std::experimental::filesystem::create_directories(beaconFilePath);
            if (!res)
            {
                std::cerr << "Failed to create beacon directory " << beaconFilePath << ": " << strerror(errno) << std::endl;
                exit(1);
            }
        }
    }

    unsigned int GetSeqNum()
    {
        static unsigned int num;
        unsigned int ret = num;
        num++;
        if (num > 999) num = 0;
        return ret;
    }

    std::string GetZulu(bool includeMonth)
    {
        char* zulu = (char*)calloc(1, 10);
        time_t rawtime;
        struct tm* timeinfo;
        time(&rawtime);
        timeinfo = gmtime(&rawtime);

        if (includeMonth) strftime(zulu, 9, "%m%d%H%M", timeinfo);
        else strftime(zulu, 7, "%d%H%M", timeinfo);

        const std::string retVal = std::string(zulu);
        free(zulu);

        return retVal;
    }

    std::string Parse(std::string text)
    {
        /* Generate a beacon string */
        int textSz = text.length();
        std::stringstream ss;
        ss << std::fixed;

        /* Look through the beacon string and make substitutions */
        for (int i=0; i<textSz; i++)
        {
            if (text[i] == '~') /* Aprs spec doesn't allow tilde, so we'll use it as an escape character */
            {
                int param = text[i+2] - '0';
                switch (text[i+1])
                {
                    case 'a':   // Scaled and averaged sensor value
                        ss << std::setprecision(Sensor::sensors[param].precision) << Sensor::Value(param);
                        i += 2;
                        break;

                    case 'A':   // Raw sensor value
                        ss << std::setprecision(0) << Sensor::sensors[param].Read(true);
                        i += 2;
                        break;

                    case 'c':   // Station callsign
                        ss << Config::myCall;
                        i++;
                        break;

                    case 'C':   // Station callsign, fixed width
                        ss << std::setw(9) << std::left << Config::myCall << std::setw(0);
                        i++;
                        break;

                    case 'g':   // GPIO pin value
                        ss << MCU::ReadGPIO(param);
                        i += 2;
                        break;

                    case 'p':   // Ping 8.8.8.8 and insert 1 for good, 0 for bad
                        ss << !(system("ping -c1 8.8.8.8 > /dev/null"));
                        i++;
                        break;

                    case 's':   // Sequence number
                        ss << std::setfill('0') << std::setw(3) << GetSeqNum();
                        ss << std::setfill(' ') << std::setw(0);
                        i++;
                        break;

                    case 'w':   // Positionless weather report
                    {
                        const std::string wx = Weather::GetReportStr(true);
                        if (wx.length() == 0) return "";
                        ss << wx;
                        i++;
                        break;
                    }

                    case 'W':   // Weather report as part of a position packet
                    {
                        const std::string wx = Weather::GetReportStr(false);
                        if (wx.length() == 0) return "";
                        ss << wx;
                        i++;
                        break;
                    }

                    case 'z':   // Timestamp, DHM
                        ss << GetZulu(false);
                        i++;
                        break;

                    case 'Z':   // Timestamp, MDHM
                        ss << GetZulu(true);
                        i++;
                        break;

                    case '~':   // Literal tilde
                        ss << '~';
                        i++;
                        break;

                    default:
                        break;
                }
            }
            else ss << text[i];
        }
        return ss.str();
    }

    void Beacon::Write()
    {
        std::ofstream f;
        const std::string fname = beaconFilePath + fileName;
        f.open(fname + ".tmp", std::ofstream::out | std::ofstream::trunc);
        if (!f)
        {
            std::cerr << "Opening beacon file " << fname << ".tmp filed: " << strerror(errno) << std::endl;
            exit (1);
        }

        f << Parse(text);
        f.close();

        std::experimental::filesystem::rename(fname + ".tmp", fname);
    }
}