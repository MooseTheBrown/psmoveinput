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



#include "config.hpp"
#include <exception>
#include <fstream>
#include <iostream>
#include <linux/input.h>

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
    pidfile_(DEF_PIDFILE),
    config_file_(DEF_CONFIGFILE),
    config_file_set_(false)
{
    // command line options description
    optdesc_.add_options()
        (OPT_HELP, OPT_HELP_DESC)
        (OPT_VERSION, OPT_VERSION_DESC)
        (OPT_PID, po::value<std::string>(), OPT_PID_DESC)
        (OPT_LOG, po::value<char>(), OPT_LOG_DESC)
        (OPT_CONFIG, po::value<std::string>(), OPT_CONFIG_DESC);

    // config file options description
    configdesc_.add_options()
        (OPT_CONF_PID, po::value<std::string>())
        (OPT_CONF_LOG, po::value<char>())
        (OPT_MOVEC_X, po::value<double>())
        (OPT_MOVEC_Y, po::value<double>())
        (OPT_PSBTN_L2, po::value<std::string>())
        (OPT_PSBTN_R2, po::value<std::string>())
        (OPT_PSBTN_L1, po::value<std::string>())
        (OPT_PSBTN_L2, po::value<std::string>())
        (OPT_PSBTN_TRIANGLE, po::value<std::string>())
        (OPT_PSBTN_CIRCLE, po::value<std::string>())
        (OPT_PSBTN_CROSS, po::value<std::string>())
        (OPT_PSBTN_SQUARE, po::value<std::string>())
        (OPT_PSBTN_SELECT, po::value<std::string>())
        (OPT_PSBTN_L3, po::value<std::string>())
        (OPT_PSBTN_R3, po::value<std::string>())
        (OPT_PSBTN_START, po::value<std::string>())
        (OPT_PSBTN_UP, po::value<std::string>())
        (OPT_PSBTN_RIGHT, po::value<std::string>())
        (OPT_PSBTN_DOWN, po::value<std::string>())
        (OPT_PSBTN_LEFT, po::value<std::string>())
        (OPT_PSBTN_PS, po::value<std::string>())
        (OPT_PSBTN_MOVE, po::value<std::string>())
        (OPT_PSBTN_T, po::value<std::string>());
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
                pidfile_ = opts_[OPT_PID_ONLYLONG].as<std::string>();
                pidfile_set_ = true;
            }
            if (opts_.count(OPT_LOG_ONLYLONG))
            {
                getLogFromChar(opts_[OPT_LOG_ONLYLONG].as<char>());
            }
            if (opts_.count(OPT_CONFIG_ONLYLONG))
            {
                config_file_ = opts_[OPT_CONFIG_ONLYLONG].as<std::string>();
                config_file_set_ = true;
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

        // store pidfile and log level, but don't override command line values
        if (conf_opts_.count(OPT_CONF_PID) && !pidfile_set_)
        {
            pidfile_ = conf_opts_[OPT_CONF_PID].as<std::string>();
            pidfile_set_ = true;
        }
        if (conf_opts_.count(OPT_CONF_LOG) && !loglevel_set_)
        {
            getLogFromChar(conf_opts_[OPT_CONF_LOG].as<char>());
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

        // add key map entries one by one
        const std::vector<boost::shared_ptr<po::option_description>> &opts = configdesc_.options();
        for (boost::shared_ptr<po::option_description> opt : opts)
        {
            const std::string &longname = opt->long_name();
            if ( (longname != OPT_CONF_PID) &&
                 (longname != OPT_CONF_LOG) &&
                 (longname != OPT_MOVEC_X) &&
                 (longname != OPT_MOVEC_Y) &&
                 conf_opts_.count(longname) )
            {
                KeyMapEntry entry;
                entry = keymap_parser_.createEntry(longname, conf_opts_[longname].as<std::string>());
                if ((entry.pscode != 0) && (entry.lincode != KEY_RESERVED))
                {
                    keymap_.push_back(entry);
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

} // namespace psmoveinput
