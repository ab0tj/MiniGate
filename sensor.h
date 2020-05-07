#ifndef INC_SENSOR_H
#define INC_SENSOR_H

namespace Sensor
{
    enum SensorType { Sensor_None, Sensor_MCU_ADC, Sensor_File, Sensor_HWMon };

    class Sensor
    {
        public:
            float scale;
            float offset;
            unsigned int precision;
            std::string fileName;
            uint mcuAdcNum;
            SensorType type;
            char unit;
            float Read(bool raw);
    };

    extern std::vector<Sensor> sensors;
}

#endif