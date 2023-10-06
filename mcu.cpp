#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiringPi.h>
#include <string>
#include <sstream>
#include <mutex>
#include "config.h"
#include "mcu.h"

namespace MCU
{
    void (*debugFunction)(const std::string&);
    std::mutex mcuLock;

    unsigned char SPITrxByte(unsigned char b)
    {
        /* Send a byte via SPI while reading another in.
        Using bitbang here as the spidev interface doesn't
        seem to work well for talking to the MCU. */

        unsigned char c = 0;

        if (Config::debug) printf("SPI out:%02X", b);
        for (int i=0; i<8; i++)
        {
            digitalWrite(Pins::MOSI, b & 0x80);
            b <<= 1;
            c <<= 1;
            usleep(1);
            c |= digitalRead(Pins::MISO);
            digitalWrite(Pins::CLK, 1);
            usleep(1);
            digitalWrite(Pins::CLK, 0);
        }

        if (Config::debug) printf(" in:%02X\n", c);
        usleep(1);
        return c;
    }

    unsigned int spiTrxWord(unsigned int w)
    {
        /* Read a 16 bit word from SPI */
        unsigned int val = SPITrxByte(w);
        val |= (unsigned int)SPITrxByte(w>>8) << 8;
        return val;
    }

    void InitMCU()
    {
        std::unique_lock lock(mcuLock);
        std::stringstream ss;

        digitalWrite(Pins::RST, 0);     // Reset the MCU
        usleep(10000);
        digitalWrite(Pins::RST, 1);     // Release reset
        usleep(100000);         // Give the MCU some time to be ready

        unsigned int temp;

        SPITrxByte(0x02);           // Check for MCU presence
        temp = spiTrxWord(0xFFFF);
        if (temp != 0xAA55)
        {
            fprintf(stderr, "Error: Invalid magic number (Expected 0xAA55 but read 0x%04X)\n", temp);
            exit(1);
        }
        
        if (Config::debug)
        {
            SPITrxByte(0x01);
            temp = spiTrxWord(0xFFFF);
            ss << "MCU Initialized. Found MCU firmware revision " << temp;
        }

        for (unsigned int i = 0; i < pttChannels; i++)
        {
            if (Config::ptt[i].enabled)
            {
                SPITrxByte(0x10 | i);
                spiTrxWord(Config::ptt[i].timeout);
                usleep(100000);
                if (Config::debug) ss << std::endl << "MCU: Initialized PTT" << i;
            }
        }

        if (Config::debug) debugFunction(ss.str());
    }

    void Init(void (*debugFunc)(const std::string&))
    {
        debugFunction = debugFunc;

        /* Initialize SPI interface */
        wiringPiSetup();
        pinMode(Pins::MOSI, OUTPUT);
        pullUpDnControl(Pins::MOSI, PUD_OFF);
        digitalWrite(Pins::MOSI, 0);
        pinMode(Pins::MISO, INPUT);
        pullUpDnControl(Pins::MISO, PUD_OFF);
        pinMode(Pins::CLK, OUTPUT);
        pullUpDnControl(Pins::CLK, PUD_OFF);
        digitalWrite(Pins::CLK, 0);
        pinMode(Pins::RST, OUTPUT);
        pullUpDnControl(4, PUD_OFF);
        digitalWrite(Pins::RST, 1);

        /* Initialize the MCU */
        InitMCU();
    }

    uint ReadADC(unsigned int a)
    {
        unsigned int val;

        if (a < adcChannels)
        {
            std::unique_lock lock(mcuLock);
            do
            {
                SPITrxByte(0x80 | a);   // Start ADC conversion
                usleep(250);            // Allow time for the conversion
                val = spiTrxWord(0xFFFF);
            } while (val > 0x3ff);      // Filter invalid results
        }

        return val;
    }

    PTTStatus GetPTTStatus(unsigned int p)
    {
        PTTStatus ps;

        if (p < pttChannels)
        {
            unsigned char val;
            std::unique_lock lock(mcuLock);
            SPITrxByte(0x20 | p);
            val = SPITrxByte(0xff);

            ps.enabled = Config::ptt[p].enabled;
            ps.initialized = val & 0x01;
            ps.asserted = val & 0x02;
            ps.timeout = val & 0x04;
        }
        return ps;
    }

    bool ReadGPIO(int num)
    {
        if (num == 0)
        {
            return digitalRead(Pins::PA0) == 1;
        }
        else if (num == 1)
        {
            return digitalRead(Pins::PA1) == 1;
        }
        else return false;
    }
}