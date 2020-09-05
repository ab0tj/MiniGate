#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "config.h"
#include "mcu.h"

namespace Sensor
{
    std::vector<Sensor> sensors;
    
    int readFile(const char* fileName)
    {
        int val;
        long size;
        char* buff;
        FILE *fp = fopen(fileName, "r");
        if (fp == NULL)
        {
            fprintf(stderr, "Error opening sensor file %s: %d\n", fileName, errno);
            exit(1);
        }

        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        rewind(fp);

        buff = (char*)calloc(1, size + 1);
        fread(buff, size, 1, fp);
        fclose(fp);

        val = atoi(buff);
        free(buff);

        return val;
    }

    float Sensor::Read(bool raw)
    {
        float val;
        uint attempt = 1;

        do
        {
            switch(type)
            {
                case Sensor_MCU_ADC:
                    val = read_mcu_adc(mcuAdcNum);
                    break;

                case Sensor_File:
                    val = readFile(fileName.c_str());
                    break;

                default:
                    return 0;
            }

            if (attempt++ >= maxReadAttempts) break;
        }
        while (val < minRawVal || val > maxRawVal); /* Keep trying to read until we get a sane value or try too many times */

        if (!raw)
        {
            val += rawOffset;
            if (abs(val) < zeroOffset) val = 0; /* If the value is close enough to 0, consider it to be 0 */
            val *= scale;
            val += offset;
        }

        return val;
    }
}