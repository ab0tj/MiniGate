#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string>
#include <sstream>
#include <iostream>
#include "config.h"
#include "util.h"
#include "mcu.h"

#define SEQFILE "/tmp/minigate.seq"

namespace Beacon
{
    unsigned int getSeqNum()
    {
        /* Return the next sequence number, or make one up if we don't know the last one */
        char buffer;
        size_t result;
        unsigned int num = rand() % 256;

        /* Open temp file */
        FILE* tempFile = fopen(SEQFILE, "rb");
        if (tempFile != NULL)
        {
            result = fread(&buffer, sizeof(char), 1, tempFile);
            if (result) num = buffer;

            fclose(tempFile);
        }

        /* Save the next number to the temp file */
        tempFile = fopen(SEQFILE, "wb");
        if (tempFile == NULL)
        {
            fprintf(stderr, "Could not open %s for writing!", SEQFILE);
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

        /* Look through the beacon string and make substitutions */
        for (int i=0; i<textSz; i++)
        {
            if (text[i] == '~') /* Aprs spec doesn't allow tilde, so we'll use it as an escape character */
            {
                int param = text[i+2] - '0';
                switch (text[i+1])
                {
                    case 'a':   // Scaled ADC value
                        ss << std::setprecision(Config::adc[param].precision) << read_adc(param, 1);
                        i += 2;
                        break;

                    case 'A':   // Raw ADC value
                        ss << std::setprecision(0) << read_adc(param, 0);
                        i += 2;
                        break;

                    case 'c':   // Station callsign
                        ss << Config::myCall;
                        i++;
                        break;

                    case 'C':   // Station callsign, fixed width
                        ss << setw(9) << std::left << Config::myCall << setw(0);
                        i++;
                        break;

                    case 'g':   // GPIO pin value (0 or 1)
                        ss << readGpio(param);
                        i += 2;
                        break;

                    case 'p':   // Ping 8.8.8.8 and insert 1 for good, 0 for bad
                        ss << !(system("ping -c1 8.8.8.8 > /dev/null"));
                        i++;
                        break;

                    case 's':   // Sequence number
                        ss << setfill('0') << setw(3) << getSeqNum());
                        ss << setfill(' ') << setw(0);
                        i++;
                        break;

                    case 't':   // Temperature value
                        ss << std::setprecision(Config::tempPrecision) << read_temp();
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