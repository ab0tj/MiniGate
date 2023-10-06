#pragma once

#include <string>
#include <vector>

namespace Beacon
{
    const std::string defaultBeaconFilePath = "/tmp/minigate/";

    std::string Parse(std::string text);
    void Init();
    std::string GetZulu(bool includeMonth);

    class Beacon
    {
        public:
            std::string text;
            std::string fileName;
            unsigned int interval, counter;
            std::string GetString() { return Parse(text); }
            void Write();
    };

    extern std::vector<Beacon> beacons;
    extern std::string beaconFilePath;
}