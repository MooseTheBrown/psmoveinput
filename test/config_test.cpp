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
#include "test_config.h"
#include "gtest/gtest.h"
#include <psmoveapi/psmove.h>
#include <linux/input.h>

namespace psmoveconfig_test
{

TEST(ConfigTest, CommandLine)
{
    const char *argv[7];
    psmoveinput::Config config;
    
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
    argv[6] = "/nonexistent";

    config.parse(7, const_cast<char**>(argv));
    ASSERT_EQ(true, config.isOK());
    ASSERT_STREQ("/var/run/testpidfile", config.getPidFileName());
    ASSERT_STREQ("/nonexistent", config.getConfigFileName());
    ASSERT_EQ(psmoveinput::LogLevel::INFO, config.getLogLevel());

    // mess up command line a bit
    psmoveinput::Config invalid_config;
    argv[4] = "blah";
    invalid_config.parse(7, const_cast<char**>(argv));
    ASSERT_EQ(false, invalid_config.isOK());

    // mess up command line totally
    psmoveinput::Config invalid_config_1;
    argv[1] = "-randomtext";
    invalid_config_1.parse(2, const_cast<char**>(argv));
    ASSERT_EQ(false, invalid_config_1.isOK());
}

TEST(ConfigTest, LongOptions)
{
    const char *argv[7];
    psmoveinput::Config config;

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
    argv[6] = "/nonexistent";

    config.parse(7, const_cast<char**>(argv));
    ASSERT_EQ(true, config.isOK());
    ASSERT_STREQ("/var/run/testpidfile", config.getPidFileName());
    ASSERT_STREQ("/nonexistent", config.getConfigFileName());
    ASSERT_EQ(psmoveinput::LogLevel::INFO, config.getLogLevel());
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

    // check key map
    psmoveinput::key_map keymap = config.getKeyMap();
    ASSERT_EQ(3, keymap.size());
    for (psmoveinput::KeyMapEntry entry : keymap)
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

} // namespace psmoveconfig_test
