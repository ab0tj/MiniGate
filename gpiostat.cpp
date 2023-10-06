#include <iostream>
#include <string>
#include <chrono>
#include <unistd.h>
#include <wiringPi.h>

void printUsage(const char* fileName);
void txSet();
void rxSet();

std::chrono::time_point<std::chrono::steady_clock> progStart, txStart;
double totalTxSec;

int main(int argc, char** argv)
{
    int opt, pttPin;
    bool pttInverted = false, pttState, lastPttState;
    float txAmps, rxAmps;
    const uint cycleTime = 2000;
    const uint cyclesPerSec = 1000000 / cycleTime;

    /* Parse command line arguments */
    if (argc < 2)
    {
        printUsage(argv[0]);
        return 1;
    }

    while((opt = getopt(argc, argv, "p:t:r:i")) != -1)
    {
        switch(opt)
        {
            case 'p':
                pttPin = atoi(optarg);
                std::cout << "Using GPIO pin " << pttPin << '\n';
                break;

            case 'i':
                pttInverted = true;
                break;

            case 't':
                txAmps = atof(optarg);
                break;

            case 'r':
                rxAmps = atof(optarg);
                break;

            default:
                printUsage(argv[0]);
                return 1;
        }
    }

    /* Set up GPIO */
    wiringPiSetupGpio();

    progStart = std::chrono::steady_clock::now();
    
    /* Main loop, monitor PTT and print stats every second */
    lastPttState = digitalRead(pttPin);
    for (;;)
    {
        for (uint t = 0; t < cyclesPerSec; t++)
        {
            usleep(cycleTime);
            pttState = digitalRead(pttPin);

            if (pttState != lastPttState)
            {
                lastPttState = pttState;
                if (pttState == pttInverted)
                {
                    rxSet();
                }
                else
                {
                    txSet();
                }
                
            }
        }

        std::chrono::duration<double> runTime = std::chrono::steady_clock::now() - progStart;
        double totalRxSec = runTime.count() - totalTxSec;
        std::cout << "TX time: " << totalTxSec << "s ";
        std::cout << '(' << (totalTxSec / runTime.count()) * 100 << "%) ";
        std::cout << "RX time: " << totalRxSec << "s ";
        std::cout << '(' << (totalRxSec / runTime.count()) * 100 << "%)\n";
    }

    return 0;
}

void printUsage(const char* fileName)
{
    std::cout << "Usage: " << fileName << " [options]\n";
    std::cout << "  -p <num>\tPTT pin number (required)\n";
    std::cout << "  -i\t\tPTT pin is inverted\n";
    std::cout << "  -t <amps>\tTX amp draw\n";
    std::cout << "  -r <amps>\tRX amp draw\n";
}

void txSet()
{
    txStart = std::chrono::steady_clock::now();
    std::cout << "TX\n";
}

void rxSet()
{
    std::chrono::duration<double> txLength = std::chrono::steady_clock::now() - txStart;
    totalTxSec += txLength.count();
    std::cout << "RX (" << txLength.count() << "s)\n";
}