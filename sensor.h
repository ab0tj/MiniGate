#ifndef INC_SENSOR_H
#define INC_SENSOR_H

namespace Sensor
{
    enum SensorType { Sensor_None, Sensor_MCU_ADC, Sensor_File };

    class Sensor
    {
        public:
            float scale;
            float offset, rawOffset;
            float minRawVal, maxRawVal;
            float zeroOffset;
            unsigned int precision;
            std::string fileName;
            uint maxReadAttempts;
            uint mcuAdcNum;
            SensorType type;
            float Read(bool raw);
    };

    extern std::vector<Sensor> sensors;
}

#endif