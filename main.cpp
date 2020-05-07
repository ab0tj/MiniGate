#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "ini.h"
#include "config.h"
#include "mcu.h"
#include "beacon.h"
#include "util.h"

void show_help(const char* cmdline)
{
    printf("Usage: %s [-abBcdDisv]\n", cmdline);
    printf("  -a <adc>\tRead ADC\n");
    printf("  -b <num>\tGenerate APRS beacon string from config\n");
    printf("  -B <string>\tGenerate APRS beacon string from command line\n");
    printf("  -c\t\tSpecify config file (default is /etc/minigate.conf)\n");
    printf("  -d\t\tPrint debugging info\n");
    printf("  -D\t\tRun as a daemon\n");
    printf("  -i\t\tInitialize MCU\n");
    printf("  -r\t\tRaw ADC output\n");
    printf("  -s <ptt>\tPrint PTT status\n");
    printf("  -t\t\tPrint temperature\n");
    printf("  -x\t\tReset MCU\n");
    printf("  -v\t\tBe verbose\n");
    printf("\n");
}

int main(int argc, char **argv)
{
    int opt, do_init = 0, adc = -1, stat = -1, printTemp = 0, scaled = 1, reset = 0, daemon = 0, do_beacon = -1;
    char *beaconText, *configFile = NULL;

    if (argc == 1)
    {
        show_help(argv[0]);
        return 1;
    }
    while((opt = getopt(argc, argv, ":vidDb:B:trxc:s:a:")) != -1)
    {
        switch(opt)
        {
            case 'v':   // Verbose
                printf("Minigate Utility v%.02f\n\n", Config::version);
                Config::verbose = true;
                break;

            case 'd':   // Debug
                Config::debug = true;
                break;

            case 'D':   // Daemon mode
                daemon = 1;
                break;

            case 'i':   // Init
                reset = 1;
                do_init = 1;
                break;

            case 'a':   // Read ADC
                adc = atoi(optarg);
                break;

            case 's':   // Get PTT status
                stat = atoi(optarg);
                break;

            case 't':   // Print temperature
                printTemp = 1;
                break;

            case 'b':   // Generate an APRS status beacon string from config values
                do_beacon = atoi(optarg);
                if (do_beacon < 0) do_beacon = -1;
                break;

            case 'B':   // Generate beacon text from command-line supplied string
                do_beacon = -2;
                beaconText = optarg;
                break;

            case 'c':   // Use a different config file
                configFile = optarg;
                break;

            case 'r':   // Raw ADC output
                scaled = 0;
                break;

            case 'x':   // Just reset the MCU
                reset = 1;
                break;

            default:
                show_help(argv[0]);
                return 1;
        }
    }

    initSpi();

    if (configFile == NULL)
    {
        configFile = (char*)malloc(20);
        strcpy(configFile, "/etc/minigate.ini");
    }

    if (ini_parse(configFile, Config::Parse, NULL) < 0)
    {
        fprintf(stderr, "Could not load %s\n", configFile);
        return 1;
    }
    if (Config::tempUnit != 'F' && Config::tempUnit != 'K') Config::tempUnit = 'C';

    if (reset) resetMcu();

    initMcu();

    if (do_init) initPtt();

    usleep(100000); // Let MCU's SPI counter reset

    if (do_beacon >= 0) std::cout << Config::beacons[do_beacon].getString() << '\n';
    if (do_beacon == -2) std::cout << Beacon::Parse(beaconText) << '\n';
    if (stat != -1) get_ptt_status(stat);
    if (adc != -1) printf("%g\n", scaled ? fround(read_adc(adc, 1), Config::adc[adc].precision) : read_adc(adc, 0));
    if (printTemp) printf("%g%c\n", fround(read_temp(), Config::tempPrecision), Config::tempUnit);
    return 0;
}
