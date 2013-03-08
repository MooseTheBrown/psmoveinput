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



#include "psmove_handler.hpp"
#include <boost/format.hpp>

namespace psmoveinput
{

PSMoveHandler::PSMoveHandler(const key_map &keymap1,
                             const key_map &keymap2,
                             const MoveCoeffs &coeffs,
                             int moveThreshold,
                             int gestureThreshold,
                             Log &log) :
    log_(log),
    moveThreshold_(moveThreshold),
    gestureThreshold_(gestureThreshold),
    useMoveTrigger_(false),
    moveTrigger_(false),
    useGestureTrigger_(false),
    gestureTrigger_(false)
{
    coeffs_.cx = coeffs.cx;
    coeffs_.cy = coeffs.cy;

    lastGyroTp_.tv_sec = 0;
    lastGyroTp_.tv_nsec = 0;

    lastGestureTp_.tv_sec = 0;
    lastGestureTp_.tv_nsec = 0;
    
    keymaps_[0] = keymap1;
    keymaps_[1] = keymap2;

    // check for triggers after key maps are initialized
    checkTriggers();

    buttons_[0] = 0;
    buttons_[1] = 0;
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
        // if move trigger is used, then report move only while
        // move trigger button is pressed
        if (((useMoveTrigger_ == true) && (moveTrigger_ == true)) ||
             (useMoveTrigger_ == false))
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

            if ((dx != 0) || (dy != 0))
            {
                move_signal_(dx, dy);
            }
        }
    }
    
    lastGyroTp_.tv_sec = gyroTp.tv_sec;
    lastGyroTp_.tv_nsec = gyroTp.tv_nsec;
}

void PSMoveHandler::onGesture(int gx, int gy)
{
    log_.write(boost::str(boost::format("PSMoveHandler::onGesture(%1%, %2%)") % gx %gy).c_str());

    // get current time
    timespec gestureTp;
    clock_gettime(CLOCK_MONOTONIC_RAW, &gestureTp);

    // handle gestures only if this is not the first measurement, and we have
    // previous measurement's timestamp to calculate time delta
    if ((lastGestureTp_.tv_sec != 0) && (lastGestureTp_.tv_nsec != 0))
    {
        // if gesture trigger is used, then handle gestures only while
        // gesture trigger button is pressed
        if (((useGestureTrigger_ == true) && (gestureTrigger_ == true)) ||
             (useGestureTrigger_ == false))
        {
            // calculations are made in the same way as in onGyroscope() function
            long timeDelta = static_cast<long>((gestureTp.tv_nsec - lastGestureTp_.tv_nsec) / 1000000);
            if (timeDelta < 0)
            {
                timeDelta += 1000;
            }

            log_.write(boost::str(boost::format("timeDelta=%1%") % timeDelta).c_str());

            int dx = static_cast<int>(gx * timeDelta * coeffs_.cx);
            int dy = static_cast<int>(gy * timeDelta * coeffs_.cy);
            if (((dx > 0) && (dx < gestureThreshold_)) ||
                ((dx < 0) && (dx > -gestureThreshold_)))
            {
                dx = 0;
            }
            if (((dy > 0) && (dy < gestureThreshold_)) ||
                ((dy < 0) && (dy > -gestureThreshold_)))
            {
                dy = 0;
            }

            int gestureButtons = 0;
            if (dx > 0)
            {
                gestureButtons |= BTN_GESTURE_RIGHT;
                log_.write("Gesture RIGHT");
            }
            else if (dx < 0)
            {
                gestureButtons |= BTN_GESTURE_LEFT;
                log_.write("Gesture LEFT");
            }

            if (dy > 0)
            {
                gestureButtons |= BTN_GESTURE_UP;
                log_.write("Gesture UP");
            }
            else if (dy < 0)
            {
                gestureButtons |= BTN_GESTURE_DOWN;
                log_.write("Gesture DOWN");
            }

            // report gesture button presses
            onButtons(gestureButtons | buttons_[1], ControllerId::SECOND);
            // report gesture buttons releases
            onButtons(gestureButtons ^ buttons_[1], ControllerId::SECOND);
        }
    }
    
    lastGestureTp_.tv_sec = gestureTp.tv_sec;
    lastGestureTp_.tv_nsec = gestureTp.tv_nsec;
}

void PSMoveHandler::onButtons(int buttons, ControllerId controller)
{
    boost::lock_guard<boost::mutex> lock(mutex_);

    log_.write(boost::str(boost::format("PSMoveHandler::onButtons(%1%)") % buttons).c_str());
    log_.write(boost::str(boost::format("PSMoveHandler::buttons_ = %1%") % buttons_).c_str());

    int buttonIndex = (controller == ControllerId::FIRST) ? 0 : 1;

    if (buttons_[buttonIndex] != buttons)
    {
        int pressed = (buttons & ~buttons_[buttonIndex]);
        int released = (buttons_[buttonIndex] & ~buttons);

        for (int i = 1; i <= BTN_GESTURE_RIGHT; i <<= 1)
        {
            if (pressed & i)
            {
                reportKey(i, true, controller);
            }
            else if (released & i)
            {
                reportKey(i, false, controller);
            }
        }

        buttons_[buttonIndex] = buttons;
    }
}

void PSMoveHandler::reset()
{
    buttons_[0] = 0;
    buttons_[1] = 0;
    lastGyroTp_.tv_sec = 0;
    lastGyroTp_.tv_nsec = 0;
    lastGestureTp_.tv_sec = 0;
    lastGestureTp_.tv_nsec = 0;
    moveTrigger_ = false;
    gestureTrigger_ = false;
}

void PSMoveHandler::reportKey(int button, bool pressed, ControllerId controller)
{
    key_map *keymap = nullptr;

    if (controller == ControllerId::FIRST)
    {
        keymap = &keymaps_[0];
    }
    else
    {
        keymap = &keymaps_[1];
    }

    log_.write(boost::str(boost::format("PSMoveHandler::reportKey(%1%, %2%)") % button % pressed).c_str());

    for (KeyMapEntry entry : *keymap)
    {
        if (entry.pscode == button)
        {
            if (handleSpecialKeys(entry.lincode, controller, pressed) == false)
            {
                // this is not a special key handled internally, so
                // pass it further
                key_signal_(entry.lincode, pressed);
            }
        }
    }
}

bool PSMoveHandler::handleSpecialKeys(int lincode, ControllerId controller, bool pressed)
{
    bool ret = false;

    if (lincode == KEY_PSMOVE_DISCONNECT)
    {
        disconnect_signal_(controller);
        ret = true;
    }
    else if((controller == ControllerId::FIRST) && (lincode == KEY_PSMOVE_MOVE_TRIGGER)) {
        moveTrigger_ = pressed;
        ret = true;
    }
    else if((controller == ControllerId::SECOND) && (lincode == KEY_PSMOVE_GESTURE_TRIGGER)) {
        gestureTrigger_ = pressed;
        ret = true;
    }

    return ret;
}

void PSMoveHandler::checkTriggers()
{
    // check the first key map for move trigger
    for (KeyMapEntry entry : keymaps_[0])
    {
        if (entry.lincode == KEY_PSMOVE_MOVE_TRIGGER)
        {
            useMoveTrigger_ = true;
            break;
        }
    }

    // check the second key map for gesture trigger
    for (KeyMapEntry entry : keymaps_[1])
    {
        if (entry.lincode == KEY_PSMOVE_GESTURE_TRIGGER)
        {
            useGestureTrigger_ = true;
            break;
        }
    }
}

} // namespace psmoveinput
