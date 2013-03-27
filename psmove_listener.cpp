/* 
 * Copyright (C) 2012, 2013 Mikhail Sapozhnikov
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

#define CALIBRATED_GYRO_COEFF   10

PSMoveListener::PSMoveListener(Log &log,
                               OpMode mode,
                               int pollTimeout,
                               int connectTimeout,
                               int disconnectTimeout,
                               int ledTimeout,
                               int gestureTimeout) :
    log_(log),
    stop_(false),
    mode_(mode),
    threadStop_(false),
    pollTimeout_(pollTimeout),
    connectTimeout_(connectTimeout),
    disconnectTimeout_(disconnectTimeout),
    ledTimeout_(ledTimeout),
    gestureTimeout_(gestureTimeout)
{
    for (int i = 0; i < MAX_CONTROLLERS; i++)
    {
        controllerThreads_[i] = new ControllerThread(log_);
    }
}

PSMoveListener::~PSMoveListener()
{
    gyroSignal_.disconnect_all_slots();
    buttonSignal_.disconnect_all_slots();

    for (int i = 0; i < MAX_CONTROLLERS; i++)
    {
        if (controllerThreads_[i] != nullptr)
        {
            delete controllerThreads_[i];
        }
    }
}

void PSMoveListener::run()
{
    PSMove *move = nullptr;
    int psmoveId = 0;

    init();

    log_.write("PSMoveListener: starting main loop");

    // main listener loop : establish and handle controller connections
    while (true)
    {
        if (stop_ || threadStop_)
        {
            // wait until all controller threads complete execution
            for (int i = 0; i < MAX_CONTROLLERS; i++)
            {
                controllerThreads_[i]->join();
            }

            if (stop_)
            {
                // we're not just stopping controller threads,
                // the listener itself has to stop working
                log_.write("Stopping PSMoveListener");
                break;
            }
            else
            {
                // no need to use mutex here because at this poing there's only one
                // main thread remaining
                threadStop_ = false;
                // notify that all controllers have been disconnected
                disconnectCompleteSignal_();
            }
        }

        // try to connect to new controller if there is less than
        // maximum number of controllers already connected
        if (isFullCapacity() == false)
        {
            move = connect(psmoveId);
            if (move != nullptr)
            {
                log_.write(boost::str(boost::format("Connected to PSMove, psmoveapi id = %1%") % psmoveId).c_str());
                handleNewDevice(psmoveId, move);
                move = nullptr;
            }
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
    std::string modestr;
    if (mode_ == OpMode::STANDALONE)
    {
        modestr = "standalone";
    }
    else
    {
        modestr = "client";
    }
    log_.write(boost::str(boost::format("PSMoveListener initializing in %1% mode") % modestr).c_str());

    // in moved client mode we need to check if moved is running,
    // and if it is, then make psmoveapi ignore all hidapi controllers
    if (mode_ == OpMode::CLIENT)
    {
        psmove_set_remote_config(PSMove_OnlyRemote);
    }
    else
    {
        psmove_set_remote_config(PSMove_OnlyLocal);
    }
}

void PSMoveListener::handleNewDevice(int psmoveId, PSMove *move)
{
    if (controllerThreads_[0]->running() == false)
    {
        controllerThreads_[0]->start(ControllerId::FIRST, psmoveId, move, this,
                                     pollTimeout_, disconnectTimeout_, ledTimeout_, gestureTimeout_);
    }
    else if (controllerThreads_[1]->running() == false)
    {
        controllerThreads_[1]->start(ControllerId::SECOND, psmoveId, move, this,
                                     pollTimeout_, disconnectTimeout_, ledTimeout_, gestureTimeout_);
    }
}

void PSMoveListener::onDisconnect()
{
    // to avoid messing up controller ids on single controller disconnect
    // we kill all controller threads
    boost::lock_guard<boost::mutex> lock(mutex_);
    threadStop_ = true;

    // controller threads for still connected controllers will be
    // re-created on next main listener loop iteration
}

void PSMoveListener::onDisconnectKey(ControllerId id)
{
    std::string btaddr;

    if (id == ControllerId::FIRST)
    {
        btaddr = controllerThreads_[0]->getBtaddr();
    }
    else
    {
        btaddr = controllerThreads_[1]->getBtaddr();
    }

    // stop controller threads
    onDisconnect();

    log_.write(boost::str(boost::format("PSMoveListener: disconnecting controller btaddr=%1%") % btaddr.c_str()).c_str());
    // call psmoveinput_disconnect giving it controller's Bluetooth address
    std::string cmd = "psmoveinput_disconnect.py ";
    cmd += btaddr;
    system(cmd.c_str());
}

PSMove *PSMoveListener::connect(int &psmoveId)
{
    PSMove *move = nullptr;

    // find correct psmoveapi id of controller to connect
    for (psmoveId = 0; psmoveId < MAX_CONTROLLERS; psmoveId++)
    {
        bool exists = false;

        // check if there is a running controller thread with this id
        for (int i = 0; i < MAX_CONTROLLERS; i++)
        {
            if ((controllerThreads_[i]->running() == true) &&
                (controllerThreads_[i]->getPSMoveId() == psmoveId))
            {
                exists = true;
                break;
            }
        }

        if (exists == false)
        {
            // no thread handles controller with this id, so try to connect to it
            move = psmove_connect_by_id(psmoveId);
            if (psmove_connection_type(move) != Conn_Bluetooth)
            {
                // ignore controllers connected via USB
                psmove_disconnect(move);
                move = nullptr;
            }
            break;
        }
    }


    return move;
}

bool PSMoveListener::isFullCapacity()
{
    bool isFull = true;

    for (int i = 0; i < MAX_CONTROLLERS; i++)
    {
        if (controllerThreads_[i]->running() == false)
        {
            isFull = false;
            break;
        }
    }

    return isFull;
}





PSMoveListener::ControllerThread::ControllerThread(Log &log) :
    id_(ControllerId::FIRST),
    move_(nullptr),
    listener_(nullptr),
    thread_(nullptr),
    log_(log),
    pollTimeout_(0),
    disconnectTimeout_(0),
    ledTimeout_(0),
    buttons_(0),
    pollCount_(0),
    psmoveId_(0),
    calibrated_(false)
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
                                             int psmoveId,
                                             PSMove *move,
                                             PSMoveListener *listener,
                                             int pollTimeout,
                                             int disconnectTimeout,
                                             int ledTimeout,
                                             int gestureTimeout)
{
    // only start new thread if there isn't one already running
    if (thread_ == nullptr)
    {
        int num = (id == ControllerId::FIRST) ? 0 : 1;
        log_.write(boost::str(boost::format("Starting controller thread for controller #%1%") %num).c_str());
        id_ = id;
        psmoveId_ = psmoveId;
        move_ = move;
        listener_ = listener;
        pollTimeout_ = pollTimeout;
        disconnectTimeout_ = disconnectTimeout;
        ledTimeout_ = ledTimeout;
        gestureTimeout_ = gestureTimeout;
        btaddr_ = psmove_get_serial(move_);
        if (psmove_has_calibration(move_) == PSMove_True)
        {
            calibrated_ = true;
        }
        else
        {
            calibrated_ = false;
        }

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
    int gestureCount = 0;

    tp.tv_sec = 0;
    tp.tv_nsec = 0;

    if ((move_ == nullptr) || (listener_ == nullptr))
    {
        return;
    }

    setLeds();

    // thread main loop
    while (true)
    {
        int gx, gy, gz, buttons;

        if (listener_->needToStop() == true)
        {
            break;
        }

        // fetch data from PSMove as long as there is something to fetch
        while (psmove_poll(move_))
        {
            // only the first controller moves the mouse pointer
            if (calibrated_ == true)
            {
                float fx, fy, fz;
                psmove_get_gyroscope_frame(move_, Frame_SecondHalf, &fx, &fy, &fz);
                /* since calibrated gyroscope values can be less than 1, e.g. something
                   like 0.00354, we multiply them by special coefficient before converting
                   them to integers in order not to miss small controller movements
                   and prevent the mouse cursor from being twitchy */
                gx = static_cast<int>(fx * CALIBRATED_GYRO_COEFF);
                gz = static_cast<int>(fz * CALIBRATED_GYRO_COEFF);
            }
            else
            {
                psmove_get_gyroscope(move_, &gx, &gy, &gz);
            }

            if (id_ == ControllerId::FIRST)
            {
                // first controller moves the cursor
                listener_->getGyroSignal()(-gz, -gx);
            }
            else
            {
                // second controller is used for gestures
                gestureCount++;
                if (gestureCount > (gestureTimeout_ / pollTimeout_))
                {
                    gestureCount = 0;
                    listener_->getGestureSignal()(-gz, gx);
                }
            }

            buttons = psmove_get_buttons(move_);
            if (buttons_ != buttons)
            {
                buttons_ = buttons;
                listener_->getButtonSignal()(buttons, id_);
            }

            // remember when we received last piece of data from PSMove
            clock_gettime(CLOCK_MONOTONIC_RAW, &lastTp_);

            updateLeds();
        }

        // if controller does not give data updates for a specific period
        // of time, consider it disconnected and stop the thread
        clock_gettime(CLOCK_MONOTONIC_RAW, &tp);
        if (lastTp_.tv_sec != 0)
        {
            if ((tp.tv_sec - lastTp_.tv_sec) > disconnectTimeout_)
            {
                break;
            }
        }
        
        boost::this_thread::sleep(boost::posix_time::millisec(pollTimeout_));
    }

    int num = (id_ == ControllerId::FIRST) ? 0 : 1;
    log_.write(boost::str(boost::format("Stopping controller thread for controller #%1%") %num).c_str());
    
    // the thread is about to stop, so we don't need thread object anymore
    thread_->detach();
    delete thread_;
    thread_ = nullptr;
    lastTp_.tv_sec = 0;
    lastTp_.tv_nsec = 0;
    
    // clean up
    psmove_disconnect(move_);
    move_ = nullptr;
    // notify the listener
    listener_->onDisconnect();
}

void PSMoveListener::ControllerThread::setLeds()
{
    if (move_ == nullptr)
    {
        return;
    }

    // TODO: make colors configurable

    if (id_ == ControllerId::FIRST)
    {
        psmove_set_leds(move_, 33, 119, 47);
    }
    else
    {
        psmove_set_leds(move_, 123, 59, 160);
    }
    psmove_update_leds(move_);
}

void PSMoveListener::ControllerThread::updateLeds()
{
    pollCount_++;
    if (pollCount_ > (ledTimeout_ / pollTimeout_))
    {
        pollCount_ = 0;
        psmove_update_leds(move_);
    }
}

} // namespace psmoveinput
