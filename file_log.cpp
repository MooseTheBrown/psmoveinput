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



#include "file_log.hpp"
#include <sys/time.h>
#include <boost/format.hpp>

namespace psmoveinput
{

FileLog::FileLog()
{
}

FileLog::~FileLog()
{
    if (file_.is_open())
    {
        file_.close();
    }
}

void FileLog::init(const LogParams &params)
{
    filename_ = params.logfile;
    file_.open(filename_.c_str(), std::ios_base::out | std::ios_base::ate);
}

void FileLog::write(const char *msg)
{
    if (!file_.good())
    {
        return;
    }

    std::string message = getTimestamp();
    message += msg;
    file_ << message.c_str() << "\n";
    file_.flush();
}

std::string FileLog::getTimestamp()
{
    timeval tv;
    std::string ret;

    if (gettimeofday(&tv, nullptr) == 0)
    {
        ret = boost::str(boost::format("%1%.%2%: ") % tv.tv_sec % tv.tv_usec);
    }

    return ret;
}

} // namespace psmoveinput
