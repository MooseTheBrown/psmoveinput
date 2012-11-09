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



#ifndef PSMOVEINPUT_CONFIG_DEFS_HPP
#define PSMOVEINPUT_CONFIG_DEFS_HPP

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
#define OPT_PSBTN_L3 "PSBTN_L3"
#define OPT_PSBTN_R3 "PSBTN_R3"
#define OPT_PSBTN_START "PSBTN_START"
#define OPT_PSBTN_UP "PSBTN_UP"
#define OPT_PSBTN_RIGHT "PSBTN_RIGHT"
#define OPT_PSBTN_DOWN "PSBTN_DOWN"
#define OPT_PSBTN_LEFT "PSBTN_LEFT"
#define OPT_PSBTN_PS "PSBTN_PS"
#define OPT_PSBTN_MOVE "PSBTN_MOVE"
#define OPT_PSBTN_T "PSBTN_T"

// defaults
#define DEF_PIDFILE "/var/run/psmoveinput.pid"
#define DEF_LOGLEVEL LogLevel::ERROR
#define DEF_CONFIGFILE "/etc/psmoveinput.conf"
#define DEF_MOVEC_X 1.0
#define DEF_MOVEC_Y 1.0

} // namespace psmoveinput

#endif // PSMOVEINPUT_CONFIG_DEFS_HPP
