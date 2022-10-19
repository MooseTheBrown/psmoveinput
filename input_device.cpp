/* 
 * Copyright (C) 2012 - 2022 Mikhail Sapozhnikov
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



#include "input_device.hpp"
#include <fcntl.h>
#include <cstring>
#include <cstdio>
#include <stdexcept>
#include <boost/format.hpp>

namespace psmoveinput
{

#define UINPUT_FILE_NAME "/dev/uinput"
#define PSMOVE_VENDOR_ID 0x054C
#define PSMOVE_PRODUCT_ID 0x03D5

InputDevice::InputDevice(const char *devname, key_array &keys, Log &log) :
    devname_(devname),
    log_(log)
{
    fd_ = open(UINPUT_FILE_NAME, O_WRONLY | O_NONBLOCK);
    if (fd_ < 0)
    {
        throw std::runtime_error("Failed to open uinput device");
    }

    ioctl(fd_, UI_SET_EVBIT, EV_REL);
    ioctl(fd_, UI_SET_RELBIT, REL_X);
    ioctl(fd_, UI_SET_RELBIT, REL_Y);
    ioctl(fd_, UI_SET_RELBIT, REL_WHEEL);
    ioctl(fd_, UI_SET_EVBIT, EV_KEY);
    for (int key : keys)
    {
        ioctl(fd_, UI_SET_KEYBIT, key);
    }

    uinput_user_dev uidev;
    std::memset(&uidev, 0, sizeof (uidev));
    std::snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "%s", devname);
    uidev.id.bustype = BUS_BLUETOOTH;
    uidev.id.vendor = PSMOVE_VENDOR_ID;
    uidev.id.product = PSMOVE_PRODUCT_ID;
    uidev.id.version = 1;

    write(fd_, &uidev, sizeof (uidev));

    ioctl(fd_, UI_DEV_CREATE);
}

InputDevice::~InputDevice()
{
    ioctl(fd_, UI_DEV_DESTROY);
    close(fd_);
}

void InputDevice::reportMove(int dx, int dy)
{
    input_event event[2];

    std::memset(event, 0, sizeof (event));

    if (dx != 0)
    {
        event[0].type = EV_REL;
        event[0].code = REL_X;
        event[0].value = dx;
    }
    if (dy != 0)
    {
        event[1].type = EV_REL;
        event[1].code = REL_Y;
        event[1].value = dy;
    }

    if (dx != 0)
    {
        write(fd_, &event[0], sizeof (input_event));
    }
    if (dy != 0)
    {
        write(fd_, &event[1], sizeof (input_event));
    }

    reportSyn();

    log_.write(boost::str(boost::format("InputDevice::reportMove(%1%, %2%)") % dx % dy).c_str());
}

void InputDevice::reportKey(int code, bool pressed)
{
    input_event event;

    std::memset(&event, 0, sizeof (event));

    event.type = EV_KEY;
    event.code = code;
    event.value = pressed == true ? 1 : 0;

    write(fd_, &event, sizeof (event));

    reportSyn();

    log_.write(boost::str(boost::format("InputDevice::reportKey(%1%, %2%)") % code % pressed).c_str());
}

void InputDevice::reportMWheel(int value)
{
    input_event event;

    std::memset(&event, 0, sizeof (event));

    event.type = EV_REL;
    event.code = REL_WHEEL;
    event.value = value;

    write(fd_, &event, sizeof (event));

    reportSyn();

    log_.write(boost::str(boost::format("InputDevice::reportMWheel(%1%)") % value).c_str());
}

void InputDevice::reportSyn()
{
    input_event event;

    std::memset(&event, 0, sizeof (event));

    event.type = EV_SYN;
    event.code = SYN_REPORT;
    event.value = 0;
    write(fd_, &event, sizeof (event));
}

} // namespace psmoveinput
