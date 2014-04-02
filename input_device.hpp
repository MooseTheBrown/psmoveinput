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



#ifndef PSMOVEINPUT_INPUT_DEVICE_HPP
#define PSMOVEINPUT_INPUT_DEVICE_HPP

#include "log.hpp"
#include <linux/uinput.h>
#include <vector>
#include <string>

namespace psmoveinput
{

typedef std::vector<int> key_array;

class InputDevice
{
public:
    InputDevice(const char *devname, key_array &keys, Log &log);
    virtual ~InputDevice();

    const char *getDeviceName() { return devname_.c_str(); }
    void reportMove(int dx, int dy);
    void reportKey(int code, bool pressed);
    void reportMWheel(int value);

protected:
    int fd_;
    std::string devname_;
    Log &log_;

    void reportSyn();
};
        
} // namespace psmoveinput

#endif // PSMOVEINPUT_INPUT_DEVICE_HPP
