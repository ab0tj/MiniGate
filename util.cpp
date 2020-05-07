#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>
#include "config.h"
#include "mcu.h"

float fround(float f, uint digits)
{
    if (digits == 0) return round(f);
    return float(int(f * pow(10, digits) + 0.5)) / pow(10, digits);
}

float read_temp()
{
    float val;
    long size;
    char* buff;
    FILE *fp = fopen(Config::tempFile, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Error opening temperature file: %d\n", errno);
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

    if (Config::tempUnit == 'F') val = (val * (9.0 / 5.0)) + 32000;
    else if (Config::tempUnit == 'K') val += 273150;

    return val / 1000;
}

float read_adc_file(int num, int scale)
{
    float val;
    long size;
    char* buff;
    FILE *fp = fopen(Config::adc[num].fileName, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Error opening ADC file: %d\n", errno);
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

    if (scale)
    {
        val += Config::adc[num].offset;
        val *= Config::adc[num].scale;
    }

    return val;
}

float read_adc(int num, int scale)
{
    if (num < 2) return read_adc_mcu(num, scale);
    else if (num < 4) return read_adc_file(num, scale);
    else
    {
        fprintf(stderr, "ADC number must be 0-3.\n");
        exit(1);
    }
}