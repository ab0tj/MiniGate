#pragma once

namespace Sensor
{
    enum SensorType { Sensor_None, Sensor_MCU_ADC, Sensor_File, Sensor_Victron, Sensor_MQTT };

    class Sensor
    {
        public:
            float scale;
            float offset, rawOffset;
            float minRawVal, maxRawVal;
            float zeroOffset;
            unsigned int precision;
            std::string locator;
            uint maxReadAttempts;
            uint mcuAdcNum;
            SensorType type;
            bool allowNegative;
            unsigned int sampleRate;
            unsigned int avgSamples;
            float Read(bool raw);
    };

    void Init(void (*debugFunc)(const std::string&));
    float Value(const int s);

    extern std::vector<Sensor> sensors;
    extern std::string victronSerialPort;
}