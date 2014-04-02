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
typedef boost::signals2::signal<void ()> disconnect_complete_signal;

class PSMoveListener
{
public:
    PSMoveListener(Log &log,
                   OpMode mode,
                   int pollTimeout,
                   int connectTimeout,
                   int disconnectTimeout,
                   int ledTimeout,
                   int gestureTimeout);
    virtual ~PSMoveListener();

    gyro_signal &getGyroSignal() { return gyroSignal_; }
    gyro_signal &getGestureSignal() { return gestureSignal_; }
    button_signal &getButtonSignal() { return buttonSignal_; }
    disconnect_complete_signal &getDisconnectCompleteSignal() { return disconnectCompleteSignal_; }
    void run();
    void stop();
    bool needToStop() { return (stop_ || threadStop_); }
    void onDisconnect();
    void onDisconnectKey(ControllerId id);

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
                   int pollTimeout,
                   int disconnectTimeout,
                   int ledTimeout,
                   int gestureTimeout);
        void join() { if (thread_ != nullptr) thread_->join(); }
        bool running();
        void operator ()();
        int getPSMoveId() { return psmoveId_; }
        std::string getBtaddr() { return btaddr_; }

    protected:
        ControllerId id_;
        PSMove *move_;
        PSMoveListener* listener_;
        boost::thread *thread_;
        Log &log_;
        int pollTimeout_;
        int disconnectTimeout_;
        int ledTimeout_;
        int buttons_;
        timespec lastTp_;
        int pollCount_;
        int psmoveId_;
        std::string btaddr_;
        bool calibrated_;
        int gestureTimeout_;

        void setLeds();
        void updateLeds();
    };

    gyro_signal gyroSignal_;
    gyro_signal gestureSignal_;
    button_signal buttonSignal_;
    disconnect_complete_signal disconnectCompleteSignal_;
    Log &log_;
    bool stop_;
    OpMode mode_;
    ControllerThread *controllerThreads_[MAX_CONTROLLERS];
    boost::mutex mutex_;
    bool threadStop_;
    int pollTimeout_;
    int connectTimeout_;
    int disconnectTimeout_;
    int ledTimeout_;
    int gestureTimeout_;

    void init();
    void handleNewDevice(int psmoveId, PSMove *move);
    PSMove *connect(int &psmoveId); 
    bool isFullCapacity();
};

} // namespace psmoveinput

#endif // PSMOVEINPUT_PSMOVE_LISTENER_HPP
