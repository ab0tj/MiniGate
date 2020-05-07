#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include "config.h"
#include "util.h"
#include "sensor.h"
#include "mcu.h"

namespace Beacon
{
    std::vector<Beacon> beacons;

    unsigned int getSeqNum()
    {
        /* Return the next sequence number, or make one up if we don't know the last one */
        uint8_t buffer;
        size_t result;
        uint8_t num = rand();

        /* Open temp file */
        FILE* tempFile = fopen((Config::tmpFolder + "/sequence").c_str(), "rb");
        if (tempFile != NULL)
        {
            result = fread(&buffer, sizeof(uint8_t), 1, tempFile);
            if (result) num = buffer;

            fclose(tempFile);
        }

        /* Save the next number to the temp file */
        tempFile = fopen((Config::tmpFolder + "/sequence").c_str(), "wb");
        if (tempFile == NULL)
        {
            fprintf(stderr, "Could not open %s for writing!", (Config::tmpFolder + "/sequence").c_str());
            exit(1);
        }

        /* Increment and write it */
        buffer = num + 1;
        fwrite(&buffer, sizeof(char), 1, tempFile);
        fclose(tempFile);

        return num;
    }

    std::string getZulu()
    {
        char* zulu = (char*)calloc(1, 8);
        time_t rawtime;
        struct tm* timeinfo;
        time(&rawtime);
        timeinfo = gmtime(&rawtime);
        strftime(zulu, 8, "%d%H%Mz", timeinfo);

        std::string retVal = std::string(zulu);
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
                    case 'a':   // Scaled sensor value
                        ss << std::setprecision(Sensor::sensors[param].precision) << Sensor::sensors[param].Read(false);
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
                        ss << readGpio(param);
                        i += 2;
                        break;

                    case 'p':   // Ping 8.8.8.8 and insert 1 for good, 0 for bad
                        ss << !(system("ping -c1 8.8.8.8 > /dev/null"));
                        i++;
                        break;

                    case 's':   // Sequence number
                        ss << std::setfill('0') << std::setw(3) << getSeqNum();
                        ss << std::setfill(' ') << std::setw(0);
                        i++;
                        break;

                    case 'z':   // Timestamp
                        ss << getZulu();
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
}