#pragma once

namespace MCU
{
    const unsigned int pttChannels = 2;
    const unsigned int pttCheckInterval = 600;
    const unsigned int adcChannels = 2;

    namespace Pins
    {
        const int MOSI = 11;
        const int MISO = 12;
        const int CLK = 14;
        const int RST = 9;
        const int PA0 = 7;
        const int PA1 = 5;
    }

    struct PTTStatus
    {
        bool initialized;
        bool enabled;
        bool asserted;
        bool timeout;
    };

    void Init(void (*debugFunc)(const std::string&));
    void InitMCU();
    uint ReadADC(unsigned int a);
    PTTStatus GetPTTStatus(unsigned int p);
    bool ReadGPIO(int num);
}