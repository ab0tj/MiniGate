#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "ini.h"
#include "config.h"
#include "mcu.h"
#include "beacon.h"
#include "util.h"
#include "sensor.h"

void Daemon();

void show_help(const char* cmdline)
{
    printf("Usage: %s [options]\n", cmdline);
    printf("  -b <num>\tGenerate APRS beacon string from config\n");
    printf("  -B <string>\tGenerate APRS beacon string from command line\n");
    printf("  -c <file>\tSpecify config file (default is /etc/minigate.conf)\n");
    printf("  -d\t\tPrint debugging info\n");
    printf("  -i\t\tInitialize MCU\n");
    printf("  -r\t\tRaw sensor output\n");
    printf("  -p <ptt>\tPrint PTT status\n");
    printf("  -s <num>\tRead sensor\n");
    printf("  -v\t\tBe verbose\n");
    printf("  -x\t\tReset MCU\n");
    printf("\n");
}

int main(int argc, char **argv)
{
    int opt, do_init = 0, sensor = -1, pttStat = -1, scaled = 1, reset = 0, do_beacon = -1;
    char *beaconText, *configFile = NULL;

    /* Parse command line arguments */
    if (argc == 1)
    {
        show_help(argv[0]);
        return 1;
    }
    while((opt = getopt(argc, argv, "b:B:c:dirp:s:vx")) != -1)
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

            case 'i':   // Init
                reset = 1;
                do_init = 1;
                break;

            case 's':   // Read sensor
                sensor = atoi(optarg);
                break;

            case 'p':   // Get PTT status
                pttStat = atoi(optarg);
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

            case 'r':   // Raw sensor output
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

    /* Do initialization things */
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

    if (reset) resetMcu();

    initMcu();

    if (do_init) initPtt();

    usleep(100000); // Let MCU's SPI counter reset

    /* Seed the RNG */
    srand(time(NULL));

    /* Now we'll move on to doing what the user requested */
    if (do_beacon >= 0) std::cout << Beacon::beacons[do_beacon].getString() << '\n';
    if (do_beacon == -2) std::cout << Beacon::Parse(beaconText) << '\n';
    if (pttStat != -1) get_ptt_status(pttStat);
    if (sensor != -1) printf("%g\n", scaled ? fround(Sensor::sensors[sensor].Read(false), Sensor::sensors[sensor].precision) : Sensor::sensors[sensor].Read(true));
    return 0;
}
