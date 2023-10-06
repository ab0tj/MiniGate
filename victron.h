#pragma once

#include <string>
#include <shared_mutex>
#include <unordered_map>
#include <thread>
#include <atomic>
#include <chrono>
#include <climits>
#include <sstream>
#include "print.h"

namespace Victron
{
    const int messageTimeout = 5;   // Time out after this many seconds of no serial activity

    /* Monitors a serial port for VE.Direct messages and parses out the values */
    class Parser
    {
        public:
            Parser() {}
            Parser(const std::string serialPort, void (*debugFunc)(const std::string&));
            ~Parser();
            int GetInt(const std::string label);
            std::string GetString(const std::string label);
            bool GetBool(const std::string label);
            long GetDataAge();
            bool DataValid();
            bool GetParserStatus() { return parserExitErrno; };
        
        private:
            std::chrono::steady_clock::time_point lastUpdate;
            mutable std::shared_mutex dataLock;
            int port;
            std::unordered_map<std::string, std::string> dataVals;
            std::thread thd;
            std::atomic_bool timeToExit;
            std::atomic_int parserExitErrno;
            void (*debugFunction)(const std::string&);
            std::stringstream debugBuf;
            void ParseLoop();
    };
}