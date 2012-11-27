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



#ifndef PSMOVEINPUT_PSMOVE_LISTENER_HPP
#define PSMOVEINPUT_PSMOVE_LISTENER_HPP

#include <boost/signals2.hpp>
#include <psmoveapi/psmove.h>
#include "log.hpp"

namespace psmoveinput
{

typedef boost::signals2::signal<void (int, int)> gyro_signal;
typedef boost::signals2::signal<void (int)> button_signal;

class PSMoveListener
{
public:
// default timeout values (ms)
// TODO: make configurable
#define DEFAULT_POLL_TIMEOUT    10
#define DEFAULT_CONN_TIMEOUT    10000

    PSMoveListener(Log &log,
                   OpMode mode,
                   int pollTimeout = DEFAULT_POLL_TIMEOUT,
                   int connectTimeout = DEFAULT_CONN_TIMEOUT);
    virtual ~PSMoveListener();

    gyro_signal &getGyroSignal() { return gyroSignal_; }
    button_signal &getButtonSignal() { return buttonSignal_; }
    void run();
    void stop() { stop_ = true; }

protected:
    gyro_signal gyroSignal_;
    button_signal buttonSignal_;
    PSMove *move_;
    Log &log_;
    int pollTimeout_;
    int connectTimeout_;
    bool stop_;
    int buttons_;
    OpMode mode_;

    void connect();
};

} // namespace psmoveinput

#endif // PSMOVEINPUT_PSMOVE_LISTENER_HPP
