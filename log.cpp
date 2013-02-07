/* 
 * Copyright (C) 2012, 2013 Mikhail Sapozhnikov
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



#include "log.hpp"

namespace psmoveinput
{

// --------------------------------------------------
// LogParams implementation
// --------------------------------------------------

LogParams::LogParams() :
    loglevel(LogLevel::ERROR)
{
}

LogParams::LogParams(std::string file, LogLevel level) :
    logfile(file),
    loglevel(level)
{
}

LogParams::LogParams(const LogParams &params) : 
    logfile(params.logfile),
    loglevel(params.loglevel)
{
}

LogParams::LogParams(LogParams &&params) :
    logfile(std::move(params.logfile)),
    loglevel(params.loglevel)
{
}

LogParams &LogParams::operator = (const LogParams &params)
{
    logfile = params.logfile;
    loglevel = params.loglevel;

    return *this;
}






// --------------------------------------------------
// Log implementation
// --------------------------------------------------

Log::Log(const LogParams &params) :
    params_(params)
{
}

Log::~Log()
{
    for (LogBackend *backend : backends_)
    {
        if (backend != nullptr)
        {
            delete backend;
        }
    }
}

void Log::addBackend(LogBackend *backend)
{
    backend->init(params_);
    backends_.push_back(backend);
}

void Log::write(const char *msg, LogLevel lvl)
{
    // higher log levels have lower values
    if (lvl <= params_.loglevel)
    {
        // write to each backend
        for (LogBackend *backend : backends_)
        {
            if (backend != nullptr)
            {
                backend->write(msg);
            }
        }
    }
}

} // namespace psmoveinput
