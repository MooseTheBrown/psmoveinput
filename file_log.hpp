/* 
 * Copyright (C) 2012 Mikhail Sapozhnikov
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



#ifndef PSMOVEINPUT_FILE_LOG_HPP
#define PSMOVEINPUT_FILE_LOG_HPP

#include "log.hpp"
#include <fstream>

namespace psmoveinput
{

// log backend, which outputs data to a file
class FileLog : public LogBackend
{
public:
    FileLog();
    virtual ~FileLog();

    virtual void init(const LogParams &params);
    virtual void write(const char *msg);

protected:
    std::string filename_;
    std::ofstream file_;

    std::string getTimestamp();
};

} // namespace psmoveinput

#endif // PSMOVEINPUT_FILE_LOG_HPP
