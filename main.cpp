#include <iostream>
#include <unistd.h>
#include <chrono>
#include <thread>
#include "ini.h"
#include "config.h"
#include "beacon.h"
#include "sensor.h"
#include "mcu.h"
#include "print.h"
#include "weather.h"

void Daemon();

Print::Buffer debugPrint;
void DebugPrint(const std::string& line) { debugPrint.Add(line); }

void show_help(const char* cmdline)
{
    std::cout << "Usage: " << cmdline << " [options]" << std::endl;
    std::cout << "  -c <file>\tSpecify config file (default is " << Config::defaultConfigFile << ')' << std::endl;
    std::cout << "  -d\t\tPrint debugging info (implies -f)" << std::endl;
    std::cout << "  -f\t\tRun in foreground" << std::endl;
}

int main(int argc, char **argv)
{
    int opt;
    bool foreground = false;
    std::string configFile = Config::defaultConfigFile;

    /* Parse command line arguments */
    while((opt = getopt(argc, argv, "c:d")) != -1)
    {
        switch(opt)
        {
            case 'd':   // Debug
                Config::debug = true;
                break;

            case 'c':   // Use a different config file
                configFile = optarg;
                break;

            case 'f':   // Run in foreground
                foreground = true;
                break;

            default:
                show_help(argv[0]);
                return 1;
        }
    }

    /* Parse config file */
    if (ini_parse(configFile.c_str(), Config::Parse, NULL) < 0)
    {
        std::cerr << "Could not load " << configFile << std::endl;
        return 1;
    }

    /* Do initialization things */
    MCU::Init(DebugPrint);
    Sensor::Init(DebugPrint);
    Weather::Init(DebugPrint);
    Beacon::Init();

    /* Write initial beacon files */
    auto next = std::chrono::steady_clock::now() + std::chrono::seconds(3);
    std::this_thread::sleep_until(next);

    for (Beacon::Beacon& b : Beacon::beacons)
    {
        b.Write();
        b.counter = 0;
    }

    /* Now we'll move on to the main program loop */
    unsigned int pttCheckCounter = 0;
    for (;;)
    {
        next += std::chrono::seconds(1);
        std::this_thread::sleep_until(next);

        /* Print any debug messages in the queue */
        if (Config::debug) debugPrint.Dump();

        /* MCU watchdog */
        if (pttCheckCounter++ >= MCU::pttCheckInterval)
        {
            bool needsReset = false;
            for (unsigned int i = 0; i < MCU::pttChannels; i++)
            {
                MCU::PTTStatus ps = MCU::GetPTTStatus(i);
                if (ps.initialized != ps.enabled) needsReset = true;
            }
            if (needsReset) MCU::InitMCU();
            pttCheckCounter = 0;
        }

        /* Update beacon files */
        for (Beacon::Beacon& b : Beacon::beacons)
        {
            if (b.interval != 0 && ++b.counter >= b.interval)
            {
                b.Write();
                b.counter = 0;
            }
        }
    }

    // Done.
    return 0;
}
