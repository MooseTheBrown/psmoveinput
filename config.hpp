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



#ifndef PSMOVEINPUT_CONFIG_HPP
#define PSMOVEINPUT_CONFIG_HPP

#include "common.hpp"
#include "config_defs.hpp"
#include "conf_keymap_parser.hpp"
#include <boost/program_options.hpp>
#include <string>

namespace psmoveinput
{

class Config
{
public:
    Config();
    virtual ~Config();

    // parse command line
    // automatically invokes config file parsing
    void parse(int argc, char **argv);
    // get location of the currently used config file
    const char *getConfigFileName() { return config_file_.c_str(); }
    // get location of the pid file
    const char *getPidFileName() { return pidfile_.c_str(); }
    // get move coeffs
    MoveCoeffs getMoveCoeffs() { return coeffs_; }
    // get keymap
    key_map getKeyMap(ControllerId controller);
    // get log level
    LogLevel getLogLevel() { return loglevel_; }
    // get operation mode
    OpMode getOpMode() { return opmode_; }
    // get fork mode
    bool getForeground() { return foreground_; }
    // timeout getters
    int getPollTimeout() { return pollTimeout_; }
    int getConnTimeout() { return connTimeout_; }
    int getDisconnectTimeout() { return disconnectTimeout_; }
    int getLedTimeout() { return ledTimeout_; }
    // get move threshold
    int getMoveThreshold() { return moveThreshold_; }

    // parsing status
    bool isOK() { return ok_; }
    // help message has to be displayed
    bool helpRequested() { return help_; }
    // version info has to be displayed
    bool versionRequested() { return version_; }
    // get error message; meaningful only in case isOK() returns false
    std::string error() { return error_; }

protected:
    boost::program_options::variables_map opts_;
    boost::program_options::variables_map conf_opts_;
    boost::program_options::options_description optdesc_;
    boost::program_options::options_description configdesc_;
    std::string error_;
    bool help_;
    bool version_;
    bool ok_;
    bool loglevel_set_;
    LogLevel loglevel_;
    bool pidfile_set_;
    std::string pidfile_;
    MoveCoeffs coeffs_;
    std::string config_file_;
    bool config_file_set_;
    key_map keymaps_[MAX_CONTROLLERS];
    KeyMapParser keymap_parser_;
    OpMode opmode_;
    bool opmode_set_;
    bool foreground_;
    int pollTimeout_;
    int connTimeout_;
    int disconnectTimeout_;
    int ledTimeout_;
    int moveThreshold_;
    
    void handleCmdLine();
    void getLogFromChar(char l);
    void parseConfig();
    bool configFileOK();
    void getModeFromString(const std::string &mode);
};

} // namespace psmoveinput

#endif // PSMOVEINPUT_CONFIG_HPP
