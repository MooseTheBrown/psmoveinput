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



#ifndef PSMOVEINPUT_CONFIG_DEFS_HPP
#define PSMOVEINPUT_CONFIG_DEFS_HPP

#include <linux/input.h>

namespace psmoveinput
{

// command line options
#define OPT_PID "pidfile,p"
#define OPT_PID_DESC "location of pid file"
#define OPT_PID_ONLYLONG "pidfile"
#define OPT_LOG "loglevel,l"
#define OPT_LOG_DESC "log level"
#define OPT_LOG_ONLYLONG "loglevel"
#define OPT_CONFIG "config,c"
#define OPT_CONFIG_DESC "config file location"
#define OPT_CONFIG_ONLYLONG "config"
#define OPT_HELP "help,h"
#define OPT_HELP_DESC "print help message"
#define OPT_HELP_ONLYLONG "help"
#define OPT_VERSION "version,v"
#define OPT_VERSION_DESC "print version information"
#define OPT_VERSION_ONLYLONG "version"
#define OPT_MODE "mode,m"
#define OPT_MODE_DESC "operation mode"
#define OPT_MODE_ONLYLONG "mode"
#define OPT_FOREGROUND "foreground,f"
#define OPT_FOREGROUND_DESC "run in foreground, do not fork"
#define OPT_FOREGROUND_ONLYLONG "foreground"
// config file options
#define OPT_CONF_PID "PID_FILE"
#define OPT_CONF_LOG "LOG_LEVEL"
#define OPT_MOVEC_X "MOVE_COEFF_X"
#define OPT_MOVEC_Y "MOVE_COEFF_Y"
#define OPT_PSBTN_L2 "PSBTN_L2"
#define OPT_PSBTN_R2 "PSBTN_R2"
#define OPT_PSBTN_L1 "PSBTN_L1"
#define OPT_PSBTN_R1 "PSBTN_R1"
#define OPT_PSBTN_TRIANGLE "PSBTN_TRIANGLE"
#define OPT_PSBTN_CIRCLE "PSBTN_CIRCLE"
#define OPT_PSBTN_CROSS "PSBTN_CROSS"
#define OPT_PSBTN_SQUARE "PSBTN_SQUARE"
#define OPT_PSBTN_SELECT "PSBTN_SELECT"
#define OPT_PSBTN_START "PSBTN_START"
#define OPT_PSBTN_PS "PSBTN_PS"
#define OPT_PSBTN_MOVE "PSBTN_MOVE"
#define OPT_PSBTN_T "PSBTN_T"
#define OPT_PSBTN_1_TRIANGLE "PSBTN_1_TRIANGLE"
#define OPT_PSBTN_1_CIRCLE "PSBTN_1_CIRCLE"
#define OPT_PSBTN_1_CROSS "PSBTN_1_CROSS"
#define OPT_PSBTN_1_SQUARE "PSBTN_1_SQUARE"
#define OPT_PSBTN_1_SELECT "PSBTN_1_SELECT"
#define OPT_PSBTN_1_START "PSBTN_1_START"
#define OPT_PSBTN_1_PS "PSBTN_1_PS"
#define OPT_PSBTN_1_MOVE "PSBTN_1_MOVE"
#define OPT_PSBTN_1_T "PSBTN_1_T"
#define OPT_CONF_MODE "MODE"
#define OPT_CONF_POLL_TIMEOUT "POLL_TIMEOUT"
#define OPT_CONF_CONN_TIMEOUT "CONN_TIMEOUT"
#define OPT_CONF_DISCONNECT_TIMEOUT "DISCONNECT_TIMEOUT"
#define OPT_CONF_LED_UPDATE_TIMEOUT "LED_UPDATE_TIMEOUT"
#define OPT_CONF_MOVE_THRESHOLD "MOVE_THRESHOLD"
#define OPT_GESTURE_UP "GESTURE_UP"
#define OPT_GESTURE_DOWN "GESTURE_DOWN"
#define OPT_GESTURE_LEFT "GESTURE_LEFT"
#define OPT_GESTURE_RIGHT "GESTURE_RIGHT"
#define OPT_CONF_GESTURE_THRESHOLD "GESTURE_THRESHOLD"

// operation modes
#define OPT_MODE_STANDALONE "standalone"
#define OPT_MODE_CLIENT     "client"

// special keys handled by psmoveinput itself
#define KEY_PSMOVE_DISCONNECT           KEY_MAX + 1
#define KEY_PSMOVE_MOVE_TRIGGER         KEY_MAX + 2
#define KEY_PSMOVE_GESTURE_TRIGGER      KEY_MAX + 3

// gesture button codes
// they should not overlap with psmoveapi button codes defined in psmove.h
#define BTN_GESTURE_UP      0x01000000
#define BTN_GESTURE_DOWN    0x02000000
#define BTN_GESTURE_LEFT    0x04000000
#define BTN_GESTURE_RIGHT   0x08000000

// defaults
#define DEF_PIDFILE "/var/run/psmoveinput.pid"
#define DEF_LOGLEVEL LogLevel::ERROR
#define DEF_CONFIGFILE "/etc/psmoveinput.conf"
#define DEF_MOVEC_X 1.0
#define DEF_MOVEC_Y 1.0
#define DEF_MODE    OPT_MODE_STANDALONE
#define DEF_POLL_TIMEOUT 20 // ms
#define DEF_CONN_TIMEOUT 3000 // ms
#define DEF_DISCONNECT_TIMEOUT 7 // s
#define DEF_LED_UPDATE_TIMEOUT 4000 // ms
#define DEF_MOVE_THRESHOLD 0 // pixels
#define DEF_GESTURE_THRESHOLD 50 // pixels

} // namespace psmoveinput

#endif // PSMOVEINPUT_CONFIG_DEFS_HPP
