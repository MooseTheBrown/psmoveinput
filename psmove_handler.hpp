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



#ifndef PSMOVEINPUT_PSMOVE_HANDLER_HPP
#define PSMOVEINPUT_PSMOVE_HANDLER_HPP

#include "common.hpp"
#include "log.hpp"
#include "config_defs.hpp"
#include <psmoveapi/psmove.h>
#include <boost/signals2.hpp>
#include <boost/thread/mutex.hpp>
#include <time.h>
#include <map>

namespace psmoveinput
{

typedef boost::signals2::signal<void (int, int)> move_signal;
typedef boost::signals2::signal<void (int, bool)> key_signal;
typedef boost::signals2::signal<void (ControllerId)> disconnect_signal;

class PSMoveHandler
{
public:
    PSMoveHandler(const key_map &keymap1,
                  const key_map &keymap2,
                  const MoveCoeffs &coeffs,
                  int moveThreshold,
                  int gestureThreshold,
                  Log &log);
    virtual ~PSMoveHandler();

    void onGyroscope(int gx, int gy);
    void onGesture(int gx, int gy);
    void onButtons(int buttons, ControllerId controller);
    void reset();

    move_signal &getMoveSignal() { return move_signal_; }
    key_signal &getKeySignal() { return key_signal_; }
    disconnect_signal &getDisconnectSignal() { return disconnect_signal_; }

protected:
    move_signal move_signal_;
    key_signal key_signal_;
    disconnect_signal disconnect_signal_;
    key_map keymaps_[MAX_CONTROLLERS];
    MoveCoeffs coeffs_;
    int buttons_[MAX_CONTROLLERS];
    Log &log_;
    int moveThreshold_;
    int gestureThreshold_;
    timespec lastGyroTp_;
    timespec lastGestureTp_;
    boost::mutex mutex_;
    bool useMoveTrigger_;
    bool moveTrigger_;
    bool useGestureTrigger_;
    bool gestureTrigger_;
    bool releaseGestureKeys_;

    void reportKey(int button, bool pressed, ControllerId controller);
    bool handleSpecialKeys(int lincode, ControllerId controller, bool pressed);
    void checkTriggers();
};

} // namespace psmoveinput

#endif // PSMOVEINPUT_PSMOVE_HANDLER_HPP
