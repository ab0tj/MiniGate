#include "print.h"
#include <iostream>
#include <mutex>

namespace Print
{
    void Buffer::Add(const std::string& line)
    {
        std::unique_lock lock(buffLock);
        buff.push_back(line);
    }

    void Buffer::Dump()
    {
        std::unique_lock lock(buffLock);
        for (std::string line : buff) std::cout << line << std::endl;
        buff.clear();
    }

    void Buffer::Clear()
    {
        std::unique_lock lock(buffLock);
        buff.clear();
    }

    bool Buffer::Available()
    {
        std::shared_lock lock(buffLock);
        return !buff.empty();
    }
}