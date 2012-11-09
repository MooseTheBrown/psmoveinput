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



#include "psmove_handler.hpp"
#include <boost/format.hpp>

namespace psmoveinput
{

PSMoveHandler::PSMoveHandler(const key_map &keymap,
                             const MoveCoeffs &coeffs,
                             int moveThreshold,
                             Log &log) :
    keymap_(keymap),
    buttons_(0),
    log_(log),
    moveThreshold_(moveThreshold)
{
    coeffs_.cx = coeffs.cx;
    coeffs_.cy = coeffs.cy;

    lastGyroTp_.tv_sec = 0;
    lastGyroTp_.tv_nsec = 0;
}

PSMoveHandler::~PSMoveHandler()
{
    move_signal_.disconnect_all_slots();
    key_signal_.disconnect_all_slots();
}

void PSMoveHandler::onGyroscope(int gx, int gy)
{
    log_.write(boost::str(boost::format("PSMoveHandler::onGyroscope(%1%, %2%)") % gx %gy).c_str());
    
    // get current time
    timespec gyroTp;
    clock_gettime(CLOCK_MONOTONIC_RAW, &gyroTp);

    // report pointer movement only if this is not the first measurement, and we have
    // previous measurement's timestamp to calculate time delta
    if ((lastGyroTp_.tv_sec != 0) && (lastGyroTp_.tv_nsec != 0))
    {
        // pointer movement for each axis is calculated by multiplying gyroscope values
        // we receive from psmove by time delta between current and previous measurements
        // in milliseconds and then multiplying the result by the coefficient
        long timeDelta = static_cast<long>((gyroTp.tv_nsec - lastGyroTp_.tv_nsec) / 1000000);
        if (timeDelta < 0)
        {
            timeDelta += 1000;
        }

        log_.write(boost::str(boost::format("timeDelta=%1%") % timeDelta).c_str());

        int dx = static_cast<int>(gx * timeDelta * coeffs_.cx);
        int dy = static_cast<int>(gy * timeDelta * coeffs_.cy);
        if (((dx > 0) && (dx < moveThreshold_)) ||
            ((dx < 0) && (dx > -moveThreshold_)))
        {
            dx = 0;
        }
        if (((dy > 0) && (dy < moveThreshold_)) ||
            ((dy < 0) && (dy > -moveThreshold_)))
        {
            dy = 0;
        }

        if ((dx !=0) || (dy != 0))
        {
            move_signal_(dx, dy);
        }
    }
    
    lastGyroTp_.tv_sec = gyroTp.tv_sec;
    lastGyroTp_.tv_nsec = gyroTp.tv_nsec;
}

void PSMoveHandler::onButtons(int buttons)
{
    log_.write(boost::str(boost::format("PSMoveHandler::onButtons(%1%)") % buttons).c_str());
    log_.write(boost::str(boost::format("PSMoveHandler::buttons_ = %1%") % buttons_).c_str());

    if (buttons_ != buttons)
    {
        int pressed = (buttons & ~buttons_);
        int released = (buttons_ & ~buttons);


        for (int i = 1; i <= Btn_T; i <<= 1)
        {
            if (pressed & i)
            {
                reportKey(i, true);
            }
            else if (released & i)
            {
                reportKey(i, false);
            }
        }

        buttons_ = buttons;
    }
}

/*int PSMoveHandler::getIndex(int button)
{
    // get button's index in internal keymap
    int ret = 0;

    while (button != 1)
    {
        button >>= 1;
        ret++;
    }

    return ret;
}*/

void PSMoveHandler::reportKey(int button, bool pressed)
{
    log_.write(boost::str(boost::format("PSMoveHandler::reportKey(%1%, %2%)") % button % pressed).c_str());

    for (KeyMapEntry entry : keymap_)
    {
        if (entry.pscode == button)
        {
            key_signal_(entry.lincode, pressed);
        }
    }
}

} // namespace psmoveinput
