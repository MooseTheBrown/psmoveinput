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



#include "config.hpp"
#include <exception>
#include <fstream>
#include <iostream>
#include <linux/input.h>
#include <sys/types.h>
#include <pwd.h>

namespace psmoveinput
{

namespace po = boost::program_options;

Config::Config() :
    optdesc_("Command line options"),
    help_(false),
    version_(false),
    ok_(false),
    loglevel_set_(false),
    pidfile_set_(false),
    loglevel_(DEF_LOGLEVEL),
    logfile_set_(false),
    config_file_(DEF_CONFIGFILE),
    config_file_set_(false),
    opmode_(OpMode::STANDALONE),
    opmode_set_(false),
    foreground_(false),
    pollTimeout_(DEF_POLL_TIMEOUT),
    connTimeout_(DEF_CONN_TIMEOUT),
    disconnectTimeout_(DEF_DISCONNECT_TIMEOUT),
    ledTimeout_(DEF_LED_UPDATE_TIMEOUT),
    moveThreshold_(DEF_MOVE_THRESHOLD),
    gestureThreshold_(DEF_GESTURE_THRESHOLD),
    gestureTimeout_(DEF_GESTURE_TIMEOUT)
{
    // default pid file location
    pidfile_ = expandTilde(DEF_PIDFILE);

    // default log file location
    logfile_ = expandTilde(DEF_LOGFILE);

    // command line options description
    optdesc_.add_options()
        (OPT_HELP, OPT_HELP_DESC)
        (OPT_VERSION, OPT_VERSION_DESC)
        (OPT_PID, po::value<std::string>(), OPT_PID_DESC)
        (OPT_LOG, po::value<char>(), OPT_LOG_DESC)
        (OPT_LOG_FILE, po::value<std::string>(), OPT_LOG_FILE_DESC)
        (OPT_CONFIG, po::value<std::string>(), OPT_CONFIG_DESC)
        (OPT_MODE, po::value<std::string>(), OPT_MODE_DESC)
        (OPT_FOREGROUND, OPT_FOREGROUND_DESC);

    // config file options description
    configdesc_.add_options()
        (OPT_CONF_PID, po::value<std::string>())
        (OPT_CONF_LOG, po::value<char>())
        (OPT_CONF_LOG_FILE, po::value<std::string>())
        (OPT_MOVEC_X, po::value<double>())
        (OPT_MOVEC_Y, po::value<double>())
        (OPT_PSBTN_TRIANGLE, po::value<std::string>())
        (OPT_PSBTN_CIRCLE, po::value<std::string>())
        (OPT_PSBTN_CROSS, po::value<std::string>())
        (OPT_PSBTN_SQUARE, po::value<std::string>())
        (OPT_PSBTN_SELECT, po::value<std::string>())
        (OPT_PSBTN_START, po::value<std::string>())
        (OPT_PSBTN_PS, po::value<std::string>())
        (OPT_PSBTN_MOVE, po::value<std::string>())
        (OPT_PSBTN_T, po::value<std::string>())
        (OPT_PSBTN_1_TRIANGLE, po::value<std::string>())
        (OPT_PSBTN_1_CIRCLE, po::value<std::string>())
        (OPT_PSBTN_1_CROSS, po::value<std::string>())
        (OPT_PSBTN_1_SQUARE, po::value<std::string>())
        (OPT_PSBTN_1_SELECT, po::value<std::string>())
        (OPT_PSBTN_1_START, po::value<std::string>())
        (OPT_PSBTN_1_PS, po::value<std::string>())
        (OPT_PSBTN_1_MOVE, po::value<std::string>())
        (OPT_PSBTN_1_T, po::value<std::string>())
        (OPT_CONF_MODE, po::value<std::string>())
        (OPT_CONF_POLL_TIMEOUT, po::value<int>())
        (OPT_CONF_CONN_TIMEOUT, po::value<int>())
        (OPT_CONF_DISCONNECT_TIMEOUT, po::value<int>())
        (OPT_CONF_LED_UPDATE_TIMEOUT, po::value<int>())
        (OPT_CONF_MOVE_THRESHOLD, po::value<int>())
        (OPT_GESTURE_UP, po::value<std::string>())
        (OPT_GESTURE_DOWN, po::value<std::string>())
        (OPT_GESTURE_LEFT, po::value<std::string>())
        (OPT_GESTURE_RIGHT, po::value<std::string>())
        (OPT_CONF_GESTURE_THRESHOLD, po::value<int>())
        (OPT_CONF_GESTURE_TIMEOUT, po::value<int>());
}

Config::~Config()
{
}

void Config::parse(int argc, char **argv)
{
    // parse command line
    try
    {
        po::store(po::parse_command_line(argc, argv, optdesc_), opts_);
        po::notify(opts_);
    }
    catch (std::exception &e)
    {
        error_ = e.what();
        return;
    }

    // fill in internal variables with values received from command line
    handleCmdLine();
    // parse configuration file only if command line is correct
    if (ok_)
    {
        parseConfig();
    }
}

key_map Config::getKeyMap(ControllerId controller)
{
    if (controller == ControllerId::FIRST)
    {
        return keymaps_[0];
    }
    else
    {
        return keymaps_[1];
    }
}

void Config::handleCmdLine()
{
    // at this point opts_ should be populated with command line options
    if (opts_.count(OPT_HELP_ONLYLONG))
    {
        help_ = true;
        ok_ = true;
        std::cout << optdesc_ << std::endl;
    }
    else if (opts_.count(OPT_VERSION_ONLYLONG))
    {
        version_ = true;
        ok_ = true;
    }
    else
    {
        // extract options values
        try
        {
            if (opts_.count(OPT_PID_ONLYLONG))
            {
                pidfile_ = expandTilde(opts_[OPT_PID_ONLYLONG].as<std::string>());
                pidfile_set_ = true;
            }
            if (opts_.count(OPT_LOG_ONLYLONG))
            {
                getLogFromChar(opts_[OPT_LOG_ONLYLONG].as<char>());
            }
            if (opts_.count(OPT_LOG_FILE_ONLYLONG))
            {
                logfile_ = expandTilde(opts_[OPT_LOG_FILE_ONLYLONG].as<std::string>());
                logfile_set_ = true;
            }
            if (opts_.count(OPT_CONFIG_ONLYLONG))
            {
                config_file_ = opts_[OPT_CONFIG_ONLYLONG].as<std::string>();
                config_file_set_ = true;
            }
            if (opts_.count(OPT_MODE_ONLYLONG))
            {
                getModeFromString(opts_[OPT_MODE_ONLYLONG].as<std::string>());
            }
            if (opts_.count(OPT_FOREGROUND_ONLYLONG))
            {
                foreground_ = true;
            }
            ok_ = true;
        }
        catch(std::exception &e)
        {
            error_ = e.what();
        }
    }
}

void Config::getLogFromChar(char l)
{
    switch (l)
    {
    case '0':
        loglevel_ = LogLevel::FATAL;
        loglevel_set_ = true;
        break;
    case '1':
        loglevel_ = LogLevel::ERROR;
        loglevel_set_ = true;
        break;
    case '2':
        loglevel_ = LogLevel::INFO;
        loglevel_set_ = true;
        break;
    default:
        break;
    }
}

void Config::parseConfig()
{
    if (configFileOK() == false)
    {
        return;
    }

    try
    {
        po::store(po::parse_config_file<char>(config_file_.c_str(), configdesc_), conf_opts_);
        po::notify(conf_opts_);

        // store pidfile, logfile and log level, but don't override command line values
        if (conf_opts_.count(OPT_CONF_PID) && !pidfile_set_)
        {
            pidfile_ = expandTilde(conf_opts_[OPT_CONF_PID].as<std::string>());
            pidfile_set_ = true;
        }
        if (conf_opts_.count(OPT_CONF_LOG) && !loglevel_set_)
        {
            getLogFromChar(conf_opts_[OPT_CONF_LOG].as<char>());
        }
        if (conf_opts_.count(OPT_CONF_LOG_FILE) && !logfile_set_)
        {
            logfile_ = expandTilde(conf_opts_[OPT_CONF_LOG_FILE].as<std::string>());
            logfile_set_ = true;
        }
        // store move coefficients
        if (conf_opts_.count(OPT_MOVEC_X))
        {
            coeffs_.cx = conf_opts_[OPT_MOVEC_X].as<double>();
        }
        if (conf_opts_.count(OPT_MOVEC_Y))
        {
            coeffs_.cy = conf_opts_[OPT_MOVEC_Y].as<double>();
        }
        if (conf_opts_.count(OPT_CONF_MODE) && !opmode_set_)
        {
            getModeFromString(conf_opts_[OPT_CONF_MODE].as<std::string>());
        }
        // store timeouts
        if (conf_opts_.count(OPT_CONF_POLL_TIMEOUT))
        {
            pollTimeout_= conf_opts_[OPT_CONF_POLL_TIMEOUT].as<int>();
        }
        if (conf_opts_.count(OPT_CONF_CONN_TIMEOUT))
        {
            connTimeout_= conf_opts_[OPT_CONF_CONN_TIMEOUT].as<int>();
        }
        if (conf_opts_.count(OPT_CONF_DISCONNECT_TIMEOUT))
        {
            disconnectTimeout_= conf_opts_[OPT_CONF_DISCONNECT_TIMEOUT].as<int>();
        }
        if (conf_opts_.count(OPT_CONF_LED_UPDATE_TIMEOUT))
        {
            ledTimeout_= conf_opts_[OPT_CONF_LED_UPDATE_TIMEOUT].as<int>();
        }
        if (conf_opts_.count(OPT_CONF_GESTURE_TIMEOUT))
        {
            gestureTimeout_ = conf_opts_[OPT_CONF_GESTURE_TIMEOUT].as<int>();
        }
        // store move threshold
        if (conf_opts_.count(OPT_CONF_MOVE_THRESHOLD))
        {
            moveThreshold_= conf_opts_[OPT_CONF_MOVE_THRESHOLD].as<int>();
        }
        // store gesture threshold
        if (conf_opts_.count(OPT_CONF_GESTURE_THRESHOLD))
        {
            gestureThreshold_= conf_opts_[OPT_CONF_GESTURE_THRESHOLD].as<int>();
        }

        // add key map entries one by one
        const std::vector<boost::shared_ptr<po::option_description>> &opts = configdesc_.options();
        for (boost::shared_ptr<po::option_description> opt : opts)
        {
            const std::string &longname = opt->long_name();
            // filter out non-key options
            if ( (longname != OPT_CONF_PID) &&
                 (longname != OPT_CONF_LOG) &&
                 (longname != OPT_CONF_LOG_FILE) &&
                 (longname != OPT_MOVEC_X) &&
                 (longname != OPT_MOVEC_Y) &&
                 (longname != OPT_CONF_MODE) &&
                 (longname != OPT_CONF_POLL_TIMEOUT) &&
                 (longname != OPT_CONF_CONN_TIMEOUT) &&
                 (longname != OPT_CONF_DISCONNECT_TIMEOUT) &&
                 (longname != OPT_CONF_LED_UPDATE_TIMEOUT) &&
                 (longname != OPT_CONF_MOVE_THRESHOLD) &&
                 (longname != OPT_CONF_GESTURE_THRESHOLD) &&
                 (longname != OPT_CONF_GESTURE_TIMEOUT) &&
                 conf_opts_.count(longname) )
            {
                KeyMapEntry entry;
                entry = keymap_parser_.createEntry(longname, conf_opts_[longname].as<std::string>());
                if ((entry.pscode != 0) && (entry.lincode != KEY_RESERVED))
                {
                    if ((longname == OPT_PSBTN_TRIANGLE) ||
                        (longname == OPT_PSBTN_CIRCLE) ||
                        (longname == OPT_PSBTN_CROSS) ||
                        (longname == OPT_PSBTN_SQUARE) ||
                        (longname == OPT_PSBTN_SELECT) ||
                        (longname == OPT_PSBTN_START) ||
                        (longname == OPT_PSBTN_PS) ||
                        (longname == OPT_PSBTN_MOVE) ||
                        (longname == OPT_PSBTN_T))
                    {
                        // first controller
                        keymaps_[0].push_back(entry);
                    }
                    // gesture options go to the second controller map
                    else
                    {
                        // second controller
                        keymaps_[1].push_back(entry);
                    }
                }
                else
                {
                    // invalid key map entry
                    ok_ = false;
                    break;
                }
            }
        }
    }
    catch(boost::bad_any_cast &e)
    {
        error_ = e.what();
        ok_ = false;
    }
    catch (std::exception &e)
    {
        error_ = e.what();
        ok_ = false;
    }
}

bool Config::configFileOK()
{
    bool ok = false;

    std::ifstream ifs(config_file_);
    if (ifs.good())
    {
        ok = true;
        ifs.close();
    }
    else if (config_file_set_)
    {
        // try default
        ifs.open(DEF_CONFIGFILE);
        if (ifs.good())
        {
            config_file_ = DEF_CONFIGFILE;
            ok = true;
            ifs.close();
        }
    }

    return ok;
}

void Config::getModeFromString(const std::string &mode)
{
    if (mode == OPT_MODE_STANDALONE)
    {
        opmode_ = OpMode::STANDALONE;
        opmode_set_ = true;
    }
    else if (mode == OPT_MODE_CLIENT)
    {
        opmode_ = OpMode::CLIENT;
        opmode_set_ = true;
    }
}

std::string Config::expandTilde(const std::string &str)
{
    if (str[0] == '~')
    {
        struct passwd *pwd = getpwuid(geteuid());
        if (pwd == nullptr)
        {
            return str;
        }
        std::string result = pwd->pw_dir;
        result += str.substr(1);
        return result;
    }
    else
    {
        return str;
    }
}

} // namespace psmoveinput
