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

        switch(type)
        {
            case Sensor_MCU_ADC:
                val = read_mcu_adc(mcuAdcNum);
                break;

            case Sensor_File:
                val = readFile(fileName.c_str());
                break;

            case Sensor_HWMon:
                val = readFile(fileName.c_str());

                if (!raw)
                {
                    if (unit == 'F') val = (val * (9.0 / 5.0)) + 32000;
                    else if (unit == 'K') val += 273150;

                    return val / 1000;
                }
                return val;

            default:
                return 0;
        }

        if (!raw)
        {
            val += offset;
            val *= scale;
        }

        return val;
    }
}