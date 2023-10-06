#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <mutex>
#include <shared_mutex>
#include <chrono>
#include <thread>
#include <atomic>
#include "config.h"
#include "mcu.h"
#include "victron.h"

namespace Sensor
{
    class SensorWatcher
    {
        public:
            SensorWatcher(const int s);
            SensorWatcher(const SensorWatcher& obj);
            float Avg();
            void Tick();
        private:
            unsigned int counter;
            unsigned int sensor;
            unsigned int next;
            unsigned int numSamples;
            std::vector<float> samples;
            mutable std::shared_mutex sampleLock;
    };

    std::vector<Sensor> sensors;
    std::vector<SensorWatcher> sensorWatchers;
    std::string victronSerialPort;
    Victron::Parser* victronParser = nullptr;
    std::thread watcherThread;
    void (*debugFunction)(const std::string&);
    std::atomic_bool timeToExit = false;
    void SensorWatcherLoop();

    void Init(void (*debugFunc)(const std::string&))
    {
        debugFunction = debugFunction;

        /* Start a Victron parser if we need it */
        if (!victronSerialPort.empty())
        {
            try
            {
                victronParser = new Victron::Parser(victronSerialPort, debugFunction);
                if (Config::debug) printf("Started Victron parser on %s\n", victronSerialPort.c_str());
            }
            catch (std::exception ex)
            {
                fprintf(stderr, "Error starting Victron parser: %s\n", ex.what());
                exit(1);
            }

            unsigned int waitTime = 0;
            while (victronParser->GetDataAge() < 0)
            {
                sleep(1);
                if (waitTime++ > Victron::messageTimeout)
                {
                    fprintf(stderr, "Error starting Victron parser: Data timeout\n");
                    exit(1);
                }
            } 
        }

        /* Populate sensorWatchers and fix any weird sensor parameters */
        sensorWatchers.reserve(sensors.size());
        for (unsigned int i = 0; i < sensors.size(); i++)
        {
            if (sensors[i].sampleRate < 1) sensors[i].sampleRate = 1;
            if (sensors[i].avgSamples < 1) sensors[i].avgSamples = 1;
            sensorWatchers.push_back(SensorWatcher(i));
        }

        /* Start sensor watcher thread */
        watcherThread = std::thread(&SensorWatcherLoop);
    }

    int ReadFile(const char* fileName)
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

        /* TODO: Return a type that also indicates if the value is valid */

        do
        {
            switch(type)
            {
                case Sensor_MCU_ADC:
                    val = MCU::ReadADC(mcuAdcNum);
                    break;

                case Sensor_File:
                    val = ReadFile(locator.c_str());
                    break;

                case Sensor_Victron:
                    val = victronParser->GetInt(locator);
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
            if (!allowNegative && val < 0) val = 0;
        }

        return val;
    }

    float Value(const int s)
    {
        return sensorWatchers[s].Avg();
    }

    SensorWatcher::SensorWatcher(const int s)
    {
        sensor = s;
        counter = sensors[s].sampleRate;
        next = 0;
        numSamples = 0;
        samples.resize(sensors[s].avgSamples);
    }

    SensorWatcher::SensorWatcher(const SensorWatcher& obj)
    {
        sensor = obj.sensor;
        counter = obj.counter;
        next = obj.next;
        numSamples = obj.next;
        samples = obj.samples;
    }

    float SensorWatcher::Avg()
    /* Return average of sensor values */
    {
        std::shared_lock lock(sampleLock);
        double sum = 0.0;
        for (unsigned int i = 0; i < numSamples; i++) sum += samples[i];
        return sum / numSamples;
    }

    void SensorWatcher::Tick()
    /* Read in new values if it's time. Call once per second. */
    {
        std::unique_lock lock(sampleLock);
        counter++;
        if (counter >= sensors[sensor].sampleRate)
        {
            counter = 0;
            samples[next] = sensors[sensor].Read(false);
            next++;
            if (next >= samples.size()) next = 0;
            if (numSamples < samples.size()) numSamples++;
        }
    }

    void SensorWatcherLoop()
    {
        auto next = std::chrono::steady_clock::now();
        while (!timeToExit.load())
        {
            next += std::chrono::seconds(1);
            for (SensorWatcher& watcher : sensorWatchers) watcher.Tick();
            std::this_thread::sleep_until(next);
        }
    }
}