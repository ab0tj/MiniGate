#include "victron.h"
#include "config.h"
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <exception>
#include <string.h>     // strerror
#include <chrono>
#include <mutex>
#include "config.h"

#include <iostream>

namespace Victron
{
    void ParseLoop();
    enum ParseState { PS_IDLE, PS_WAIT_FIELD_START, PS_LABEL, PS_VALUE, PS_HEX, PS_CKSUM };

    class Checksum
    {
        public:
            inline void Add(const unsigned char c) { value += c; }
            inline bool Check(const unsigned char c) { Add(c); return value == 0; }
            inline void Reset() { value = 0; }
        private:
            uint8_t value;
    };

    Parser::Parser(const std::string serialPort, void (*debugFunc)(const std::string&))
    {
        /* Set vars */
        lastUpdate = std::chrono::steady_clock::time_point::min();
        timeToExit = false;
        parserExitErrno = -1;
        debugFunction = debugFunc;

        /* Open serial port */
        port = open(serialPort.c_str(), O_RDWR);
        if (port < 0) throw std::runtime_error("Failed to open serial port: " + std::string(strerror(errno)));

        /* Set serial port options */
        struct termios tty;
        if (tcgetattr(port, &tty) != 0) throw std::runtime_error("Error from tcgetattr: " + std::string(strerror(errno)));
        tty.c_cflag &= ~(PARENB|CSTOPB|CSIZE|CRTSCTS);                  // No parity, 1 stop bit, no hardware flow control, clear size bits
        tty.c_cflag |= (CS8|CREAD|CLOCAL);                              // 8 bits, local mode, enable reading
        tty.c_lflag &= ~(ICANON|ECHO|ECHOE|ECHONL|ISIG);                // Disable canonical mode, echo, and signal chars
        tty.c_iflag &= ~(IXON|IXOFF|IXANY);                             // Disable software flow control
        tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes
        tty.c_oflag &= ~(OPOST|ONLCR);                                  // Disable any special handling of transmitted bytes
        tty.c_cc[VTIME] = 20;                                           // 2 second timeout on reads
        tty.c_cc[VMIN] = 0;                                             // No minimum chars on read
        cfsetspeed(&tty, B19200);                                       // 19200 baud
        if (tcsetattr(port, TCSANOW, &tty) != 0) throw std::runtime_error("Error from tcsetattr: " + std::string(strerror(errno)));

        /* Start parse thread */
        thd = std::thread(&Parser::ParseLoop, this);
    }

    Parser::~Parser()
    {
        /* Tell the parse thread it is time to quit and wait for it to finish */
        timeToExit = true;
        thd.join();

        /* Close serial port */
        close(port);
    }

    std::string Parser::GetString(const std::string label)
    {
        std::shared_lock lock(dataLock);
        auto it = dataVals.find(label);
        if (it == dataVals.end()) return std::string();
        else return it->second;
    }

    long Parser::GetDataAge()
    {
        std::shared_lock lock(dataLock);
        if (lastUpdate == std::chrono::steady_clock::time_point::min()) return -1;
        else return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - lastUpdate).count();
    }

    int Parser::GetInt(const std::string label)
    {
        const std::string val = GetString(label);
        if (val.empty()) return INT_MIN;
        else return std::stoi(val);
    }

    bool Parser::GetBool(const std::string label)
    {
        return GetString(label).compare("ON") == 0;
    }

    bool Parser::DataValid()
    {
        std::shared_lock lock(dataLock);
        return dataVals.size() != 0;
    }

    void Parser::ParseLoop()
    {
        ParseState state = PS_IDLE, prevState;
        std::string label, value;
        std::unordered_map<std::string, std::string> nextMap;
        char c;
        ssize_t bytes;
        std::chrono::steady_clock::time_point lastRx;
        Checksum cs;

        while (!timeToExit.load())
        {
            bytes = read(port, &c, 1);
            if (bytes < 0)
            {
                parserExitErrno = errno;
                return;
            }

            else if (bytes > 0)
            {
                lastRx = std::chrono::steady_clock::now();

                if (c == ':' && state != PS_CKSUM)
                /* Interrupted by an async HEX frame */
                {
                    if (state != PS_HEX) prevState = state;
                    state = PS_HEX;
                    continue;
                }

                switch (state)
                {
                    case PS_HEX:
                        if (c == '\n') state = prevState;
                        continue;

                    case PS_IDLE:
                        if (c == '\r')
                        {
                            state = PS_WAIT_FIELD_START;
                            cs.Reset();
                        }
                        break;

                    case PS_WAIT_FIELD_START:
                        if (c == '\n')
                        {
                            state = PS_LABEL;
                            label = std::string();
                        }
                        else state = PS_IDLE;
                        break;

                    case PS_LABEL:
                        if (c == '\t')
                        {
                            if (label.compare("Checksum") == 0)
                            {
                                state = PS_CKSUM;
                            }
                            else
                            {
                                state = PS_VALUE;
                                value = std::string();
                            }
                        }
                        else label += c;
                        break;

                    case PS_VALUE:
                        if (c == '\r')
                        {
                            nextMap.emplace(label, value);
                            state = PS_WAIT_FIELD_START;

                            if (Config::debug) debugBuf << "Victron: Parsed label " << label << " with value " << value << std::endl;
                        }
                        else value += c;
                        break;

                    case PS_CKSUM:
                        if (Config::debug) debugBuf << "Victron: Checksum ";
                        if (cs.Check(c))
                        {
                            std::unique_lock lock(dataLock);
                            dataVals.swap(nextMap);
                            lastUpdate = std::chrono::steady_clock::now();
                            lock.unlock();

                            nextMap.clear();

                            if (Config::debug) debugBuf << "OK";
                        }
                        else if (Config::debug) debugBuf << "FAIL";
                        state = PS_IDLE;

                        if (Config::debug)
                        {
                            std::cout << debugBuf.str() << std::endl;
                            //debugFunction(debugBuf.str());
                            debugBuf.str(std::string());
                        }
                        continue;
                }

                cs.Add(c);
            }

            /* read timed out */
            else if (state != PS_IDLE)
            {
                auto elapsed = std::chrono::steady_clock::now() - lastRx;
                if (std::chrono::duration_cast<std::chrono::seconds>(elapsed).count() >= messageTimeout)
                {
                    state = PS_IDLE;
                }
            }
        }

        parserExitErrno = 0;
    }
}