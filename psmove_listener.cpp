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



#include "psmove_listener.hpp"
#include <boost/thread.hpp>
#include <boost/format.hpp>

namespace psmoveinput
{

PSMoveListener::PSMoveListener(Log &log,
                               OpMode mode,
                               int pollTimeout,
                               int connectTimeout) :
    move_(nullptr),
    log_(log),
    pollTimeout_(pollTimeout),
    connectTimeout_(connectTimeout),
    stop_(false),
    buttons_(0),
    mode_(mode)
{
}

PSMoveListener::~PSMoveListener()
{
    if (move_ != nullptr)
    {
        psmove_disconnect(move_);
    }
    
    gyroSignal_.disconnect_all_slots();
    buttonSignal_.disconnect_all_slots();
}

void PSMoveListener::run()
{
    connect();

    // if stopped, don't enter the polling loop
    if (stop_)
    {
        log_.write("PSMoveListener stopped");
        return;
    }

    while (true)
    {
        int gx, gy, gz, buttons;

        if (stop_)
        {
            log_.write("PSMoveListener stopped");
            break;
        }

        while (psmove_poll(move_))
        {
            psmove_get_gyroscope(move_, &gx, &gy, &gz);
            gyroSignal_(-gz, -gx);

            buttons = psmove_get_buttons(move_);
            if (buttons_ != buttons)
            {
                buttons_ = buttons;
                buttonSignal_(buttons);
            }
        }

        boost::this_thread::sleep(boost::posix_time::millisec(pollTimeout_));
    }
}

void PSMoveListener::connect()
{
    log_.write("Trying to connect PSMove device");

    // try to connect over and over until success
    while (true)
    {
        if (stop_)
        {
            break;
        }

        move_ = psmove_connect();
        if (move_ != nullptr)
        {
            log_.write("Connected to PSMove device");
            break;
        }

        boost::this_thread::sleep(boost::posix_time::millisec(connectTimeout_));
    }
}

}
