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
#include "test_config.h"
#include "gtest/gtest.h"
#include <psmoveapi/psmove.h>
#include <linux/input.h>
#include <sys/types.h>
#include <pwd.h>

namespace psmoveconfig_test
{

TEST(ConfigTest, CommandLine)
{
    const char *argv[12];
    psmoveinput::Config config;
    std::string config_name = TEST_CONFIG_PATH;
    config_name += "test_config.conf";
    
    // program name
    argv[0] = "test";
    // pid file
    argv[1] = "-p";
    argv[2] = "/var/run/testpidfile";
    // log level
    argv[3] = "-l";
    argv[4] = "2";
    // config file
    argv[5] = "-c";
    argv[6] = config_name.c_str();
    // operation mode
    argv[7] = "-m";
    argv[8] = "client";
    // foreground mode
    argv[9] = "-f";
    // log file location
    argv[10] = "-L";
    argv[11] = "/var/log/testlog";

    config.parse(12, const_cast<char**>(argv));
    ASSERT_EQ(true, config.isOK());
    ASSERT_STREQ("/var/run/testpidfile", config.getPidFileName());
    ASSERT_STREQ(config_name.c_str(), config.getConfigFileName());
    ASSERT_EQ(psmoveinput::LogLevel::INFO, config.getLogLevel());
    ASSERT_STREQ("/var/log/testlog", config.getLogFileName());
    ASSERT_EQ(psmoveinput::OpMode::CLIENT, config.getOpMode());
    ASSERT_EQ(true, config.getForeground());

    // mess up command line a bit
    psmoveinput::Config invalid_config;
    argv[4] = "blah";
    invalid_config.parse(12, const_cast<char**>(argv));
    ASSERT_EQ(false, invalid_config.isOK());

    // mess up command line totally
    psmoveinput::Config invalid_config_1;
    argv[1] = "-randomtext";
    invalid_config_1.parse(2, const_cast<char**>(argv));
    ASSERT_EQ(false, invalid_config_1.isOK());
}

TEST(ConfigTest, LongOptions)
{
    const char *argv[12];
    psmoveinput::Config config;
    std::string config_name = TEST_CONFIG_PATH;
    config_name += "test_config.conf";

    // program name
    argv[0] = "test";
    // pid file
    argv[1] = "--pidfile";
    argv[2] = "/var/run/testpidfile";
    // log level
    argv[3] = "--loglevel";
    argv[4] = "2";
    // config file
    argv[5] = "--config";
    argv[6] = config_name.c_str();
    // operation mode
    argv[7] = "--mode";
    argv[8] = "client";
    // foreground mode
    argv[9] = "--foreground";
    // log file location
    argv[10] = "--logfile";
    argv[11] = "/var/log/testlog";

    config.parse(12, const_cast<char**>(argv));
    ASSERT_EQ(true, config.isOK());
    ASSERT_STREQ("/var/run/testpidfile", config.getPidFileName());
    ASSERT_STREQ(config_name.c_str(), config.getConfigFileName());
    ASSERT_EQ(psmoveinput::LogLevel::INFO, config.getLogLevel());
    ASSERT_STREQ("/var/log/testlog", config.getLogFileName());
    ASSERT_EQ(psmoveinput::OpMode::CLIENT, config.getOpMode());
    ASSERT_EQ(true, config.getForeground());
}

TEST(ConfigTest, CorrectConfig)
{
    const char *argv[3];
    psmoveinput::Config config;
    std::string temp;

    argv[0] = "test";
    argv[1] = "-c";
    temp = TEST_CONFIG_PATH;
    temp += "test_config.conf";
    argv[2] = temp.c_str();

    config.parse(3, const_cast<char**>(argv));
    ASSERT_EQ(true, config.isOK());

    // check pid file
    ASSERT_STREQ("/var/run/testpidfile", config.getPidFileName());
    ASSERT_EQ(psmoveinput::LogLevel::INFO, config.getLogLevel());

    // check move coefficients
    psmoveinput::MoveCoeffs coeffs = config.getMoveCoeffs();
    ASSERT_EQ(1.0, coeffs.cx);
    ASSERT_EQ(1.5, coeffs.cy);

    // check operation mode
    ASSERT_EQ(psmoveinput::OpMode::CLIENT, config.getOpMode());

    // check timeouts
    ASSERT_EQ(100, config.getPollTimeout());
    ASSERT_EQ(500, config.getConnTimeout());
    ASSERT_EQ(20, config.getDisconnectTimeout());
    ASSERT_EQ(1000, config.getLedTimeout());
    ASSERT_EQ(300, config.getGestureTimeout());

    // check move threshold
    ASSERT_EQ(3, config.getMoveThreshold());

    // check gesture threshold
    ASSERT_EQ(20, config.getGestureThreshold());

    // check log file name
    ASSERT_STREQ("/var/run/testlog", config.getLogFileName());

    // check key maps
    psmoveinput::key_map keymap1 = config.getKeyMap(psmoveinput::ControllerId::FIRST);
    ASSERT_EQ(7, keymap1.size());
    for (psmoveinput::KeyMapEntry entry : keymap1)
    {
        if (entry.pscode == Btn_CROSS)
        {
            ASSERT_EQ(KEY_ENTER, entry.lincode);
        }
        else if (entry.pscode == Btn_T)
        {
            ASSERT_EQ(KEY_ESC, entry.lincode);
        }
        else if (entry.pscode == Btn_TRIANGLE)
        {
            ASSERT_EQ(KEY_1, entry.lincode);
        }
        else if (entry.pscode == Btn_PS)
        {
            ASSERT_EQ(KEY_PSMOVE_DISCONNECT, entry.lincode);
        }
        else if (entry.pscode == Btn_MOVE)
        {
            ASSERT_EQ(KEY_PSMOVE_MOVE_TRIGGER, entry.lincode);
        }
        else if (entry.pscode == Btn_SELECT)
        {
            ASSERT_EQ(KEY_PSMOVE_MWHEEL_UP, entry.lincode);
        }
        else if (entry.pscode == Btn_START)
        {
            ASSERT_EQ(KEY_PSMOVE_MWHEEL_DOWN, entry.lincode);
        }
        else
        {
            ASSERT_TRUE(false);
        }
    }
    psmoveinput::key_map keymap2 = config.getKeyMap(psmoveinput::ControllerId::SECOND);
    ASSERT_EQ(8, keymap2.size());
    for (psmoveinput::KeyMapEntry entry : keymap2)
    {
        if (entry.pscode == Btn_CROSS)
        {
            ASSERT_EQ(KEY_F1, entry.lincode);
        }
        else if (entry.pscode == Btn_T)
        {
            ASSERT_EQ(KEY_F2, entry.lincode);
        }
        else if (entry.pscode == Btn_TRIANGLE)
        {
            ASSERT_EQ(KEY_F3, entry.lincode);
        }
        else if (entry.pscode == BTN_GESTURE_UP)
        {
            ASSERT_EQ(KEY_UP, entry.lincode);
        }
        else if (entry.pscode == BTN_GESTURE_DOWN)
        {
            ASSERT_EQ(KEY_DOWN, entry.lincode);
        }
        else if (entry.pscode == BTN_GESTURE_LEFT)
        {
            ASSERT_EQ(KEY_LEFT, entry.lincode);
        }
        else if (entry.pscode == BTN_GESTURE_RIGHT)
        {
            ASSERT_EQ(KEY_RIGHT, entry.lincode);
        }
        else if (entry.pscode == Btn_MOVE)
        {
            ASSERT_EQ(KEY_PSMOVE_GESTURE_TRIGGER, entry.lincode);
        }
        else
        {
            ASSERT_TRUE(false);
        }
    }
}

TEST(ConfigTest, IncorrectConfig)
{
    const char *argv[3];
    psmoveinput::Config config;
    std::string temp;

    argv[0] = "test";
    argv[1] = "-c";
    temp = TEST_CONFIG_PATH;
    temp += "invalid_config.conf";
    argv[2] = temp.c_str();

    config.parse(3, const_cast<char**>(argv));
    ASSERT_EQ(false, config.isOK());
}

TEST(ConfigTest, DoubleSettings)
{
    const char *argv[3];
    psmoveinput::Config config;
    std::string temp;

    argv[0] = "test";
    argv[1] = "-c";
    temp = TEST_CONFIG_PATH;
    temp += "double_settings.conf";
    argv[2] = temp.c_str();

    config.parse(3, const_cast<char**>(argv));
    ASSERT_EQ(false, config.isOK());
}

TEST(ConfigTest, TildeExpansion)
{
    const char *argv[5];
    psmoveinput::Config config;
    struct passwd *pw;

    pw = getpwuid(geteuid());
    std::string expected_pidfile = pw->pw_dir;
    expected_pidfile += "/testpidfile";
    std::string expected_logfile = pw->pw_dir;
    expected_logfile += "/testlogfile";

    // program name
    argv[0] = "test";
    // pid file
    argv[1] = "-p";
    argv[2] = "~/testpidfile";
    // log file
    argv[3] = "-L";
    argv[4] = "~/testlogfile";

    config.parse(5, const_cast<char**>(argv));
    ASSERT_EQ(true, config.isOK());
    ASSERT_STREQ(expected_pidfile.c_str(), config.getPidFileName());
    ASSERT_STREQ(expected_logfile.c_str(), config.getLogFileName());
}

} // namespace psmoveconfig_test
