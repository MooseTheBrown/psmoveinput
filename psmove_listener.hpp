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

#include "log.hpp"
#include <boost/signals2.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <psmoveapi/psmove.h>
#include <time.h>

namespace psmoveinput
{

typedef boost::signals2::signal<void (int, int)> gyro_signal;
typedef boost::signals2::signal<void (int, ControllerId)> button_signal;

class PSMoveListener
{
public:
// default timeout values (ms)
// TODO: make configurable
#define DEFAULT_POLL_TIMEOUT    20
#define DEFAULT_CONN_TIMEOUT    3000
// controller disconnect timeout (s)
#define DISCONNECT_TIMEOUT      7
// LED update timeout (ms)
#define LED_TIMEOUT             4000

    PSMoveListener(Log &log,
                   OpMode mode,
                   int pollTimeout = DEFAULT_POLL_TIMEOUT,
                   int connectTimeout = DEFAULT_CONN_TIMEOUT);
    virtual ~PSMoveListener();

    gyro_signal &getGyroSignal() { return gyroSignal_; }
    button_signal &getButtonSignal() { return buttonSignal_; }
    void run();
    void stop();
    bool needToStop() { return (stop_ || threadStop_); }
    void onDisconnect();

protected:

    // controller thread manages connection to single PSMove controller
    // and invokes listener's signals on getting input events
    class ControllerThread
    {
    public:
        ControllerThread(Log &log);
        virtual ~ControllerThread();

        void start(ControllerId id,
                   int psmoveId,
                   PSMove *move,
                   PSMoveListener *listener,
                   int pollTimeout);
        void join() { if (thread_ != nullptr) thread_->join(); }
        bool running();
        void operator ()();
        int getPSMoveId() { return psmoveId_; }

    protected:
        ControllerId id_;
        PSMove *move_;
        PSMoveListener* listener_;
        boost::thread *thread_;
        Log &log_;
        int pollTimeout_;
        int buttons_;
        timespec lastTp_;
        int pollCount_;
        int psmoveId_;

        void setLeds();
        void updateLeds();
    };

    gyro_signal gyroSignal_;
    button_signal buttonSignal_;
    Log &log_;
    int pollTimeout_;
    int connectTimeout_;
    bool stop_;
    OpMode mode_;
    ControllerThread *controllerThreads_[MAX_CONTROLLERS];
    boost::mutex mutex_;
    bool threadStop_;

    void init();
    bool checkMoved();
    void handleNewDevice(int psmoveId, PSMove *move);
    PSMove *connect(int &psmoveId); 
    bool isFullCapacity();
};

} // namespace psmoveinput

#endif // PSMOVEINPUT_PSMOVE_LISTENER_HPP
