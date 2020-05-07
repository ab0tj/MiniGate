#ifndef INC_BEACON_H
#define INC_BEACON_H

#include <string>

namespace Beacon
{
    std::string Parse(std::string text);

    class Beacon
    {
        public:
            std::string text, fileName;
            uint interval, timer;
            std::string getString() { return Parse(text); }
    };

    extern std::vector<Beacon> beacons;
}

#endif