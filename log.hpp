/* 
 * Copyright (C) 2012, 2013, 2014 Mikhail Sapozhnikov
 *
 * This file is part of psmoveinput.
 *
 * psmoveinput is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * psmoveinput is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with psmoveinput.  If not, see <http://www.gnu.org/licenses/>.
 *
 */



#ifndef PSMOVEINPUT_LOG_HPP
#define PSMOVEINPUT_LOG_HPP

#include "common.hpp"
#include <string>

namespace psmoveinput
{

struct LogParams
{
    std::string logfile;
    LogLevel loglevel;

    LogParams();
    LogParams(std::string file, LogLevel level);
    LogParams(const LogParams &params);
    LogParams(LogParams &&params);

    LogParams &operator = (const LogParams &params);
};

class LogBackend
{
public:
    virtual void init(const LogParams &params) = 0;
    virtual void write(const char *msg) = 0;
};

class Log
{
public:
    Log(const LogParams &params);
    virtual ~Log();

    void addBackend(LogBackend *backend);
    // log takes backend ownership, dont try to free
    // memory occupied by backend object

    void write(const char *msg, LogLevel lvl = LogLevel::INFO);

    Log(const Log &) = delete;
    Log &operator = (const Log &) = delete;
protected:
    std::vector<LogBackend*> backends_;
    LogParams params_;
};

} // namespace psmoveinput

#endif // PSMOVEINPUT_LOG_HPP
