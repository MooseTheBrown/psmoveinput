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
#include <boost/format.hpp>
#include <boost/thread/locks.hpp>
#include <cstdlib>

namespace psmoveinput
{

PSMoveListener::PSMoveListener(Log &log,
                               OpMode mode,
                               int pollTimeout,
                               int connectTimeout) :
    log_(log),
    pollTimeout_(pollTimeout),
    connectTimeout_(connectTimeout),
    stop_(false),
    mode_(mode)
{
    for (ControllerThread *thread : controllerThreads_)
    {
        thread = new ControllerThread(log_);
    }
}

PSMoveListener::~PSMoveListener()
{
    gyroSignal_.disconnect_all_slots();
    buttonSignal_.disconnect_all_slots();

    for (ControllerThread *thread : controllerThreads_)
    {
        if (thread != nullptr)
        {
            delete thread;
        }
    }
}

void PSMoveListener::run()
{
    PSMove *move = nullptr;
    int connected = 0;

    init();

    // main listener loop
    while (true)
    {
        if (stop_)
        {
            log_.write("Stopping PSMoveListener");
            // wait until all controller threads complete execution
            for (ControllerThread *thread : controllerThreads_)
            {
                if (thread->running() == true)
                {
                    thread->join();
                }
            }
            break;
        }

        // are there any new controllers availalbe?
        connected = psmove_count_connected();
        move = psmove_connect_by_id(connected);
        if (move != nullptr)
        {
            log_.write("Found new PSMove device");
            handleNewDevice(move);
            move = nullptr;
        }

        boost::this_thread::sleep(boost::posix_time::millisec(connectTimeout_));
    }
}

void PSMoveListener::stop()
{
    stop_ = true;
}

void PSMoveListener::init()
{
    // in moved client mode we need to check if moved is running,
    // and if it is, then make psmoveapi ignore all hidapi controllers
    if (mode_ == OpMode::CLIENT)
    {
        if (checkMoved() == true)
        {
            psmove_set_remote_config(PSMove_OnlyRemote);
        }
        else
        {
            // there is no moved, force standalone mode
            mode_ = OpMode::STANDALONE;
            psmove_set_remote_config(PSMove_OnlyLocal);
            log_.write("Launched in client mode, but couldn't find moved", LogLevel::ERROR);
            log_.write("Forcing standalone mode", LogLevel::ERROR);
        }
    }
}

// ugly way to check if moved is running
bool PSMoveListener::checkMoved()
{
    if (system("ps -e | grep moved") == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void PSMoveListener::handleNewDevice(PSMove *move)
{
    if (controllerThreads_[0]->running() == false)
    {
        controllerThreads_[0]->start(ControllerId::FIRST, move, this, pollTimeout_);
    }
    else if (controllerThreads_[1]->running() == false)
    {
        controllerThreads_[1]->start(ControllerId::SECOND, move, this, pollTimeout_);
    }
}





PSMoveListener::ControllerThread::ControllerThread(Log &log) :
    id_(ControllerId::FIRST),
    move_(nullptr),
    listener_(nullptr),
    thread_(nullptr),
    log_(log),
    pollTimeout_(DEFAULT_POLL_TIMEOUT),
    buttons_(0)
{
    lastTp_.tv_sec = 0;
    lastTp_.tv_nsec = 0;
}

PSMoveListener::ControllerThread::~ControllerThread()
{
    if (thread_ != nullptr)
    {
        delete thread_;
    }
}

void PSMoveListener::ControllerThread::start(ControllerId id,
                                             PSMove *move,
                                             PSMoveListener *listener,
                                             int pollTimeout)
{
    bool running = false;

    // only start new thread if there isn't one already running
    {
        boost::lock_guard<boost::mutex> lock(mutex_);
        if (thread_ != nullptr)
        {
            running = true;
        }
    }
    if (running == false)
    {
        id_ = id;
        move_ = move;
        listener_ = listener;
        pollTimeout_ = pollTimeout;
        thread_ = new boost::thread(boost::ref(*this));
    }
}

bool PSMoveListener::ControllerThread::running()
{
    return ((thread_ == nullptr) ? false : true);
}

void PSMoveListener::ControllerThread::operator ()()
{
    timespec tp;

    tp.tv_sec = 0;
    tp.tv_nsec = 0;

    if ((move_ == nullptr) || (listener_ == nullptr))
    {
        return;
    }

    // thread main loop
    while (true)
    {
        int gx, gy, gz, buttons;

        if (listener_->needToStop() == true)
        {
            log_.write(boost::str(boost::format("Stopping controller thread for controller #%1%") %static_cast<int>(id_)).c_str());
            break;
        }

        // fetch data from PSMove as long as there is something to fetch
        while (psmove_poll(move_))
        {
            psmove_get_gyroscope(move_, &gx, &gy, &gz);
            listener_->getGyroSignal()(-gz, -gx);

            buttons = psmove_get_buttons(move_);
            if (buttons_ != buttons)
            {
                buttons_ = buttons;
                listener_->getButtonSignal()(buttons, id_);
            }

            // remember when we received last piece of data from PSMove
            clock_gettime(CLOCK_MONOTONIC_RAW, &lastTp_);
        }

        // if controller does not give data updates for a specific period
        // of time, consider it disconnected and stop the thread
        clock_gettime(CLOCK_MONOTONIC_RAW, &tp);
        if ((tp.tv_sec - lastTp_.tv_sec) > DISCONNECT_TIMEOUT)
        {
            break;
        }
        
        boost::this_thread::sleep(boost::posix_time::millisec(pollTimeout_));
    }
    
    // the thread is about to stop, so we don't need thread object anymore
    {
        boost::lock_guard<boost::mutex> lock(mutex_);
        thread_->detach();
        delete thread_;
        thread_ = nullptr;
    }
    
    // clean up
    psmove_disconnect(move_);
    move_ = nullptr;
}

} // namespace psmoveinput
