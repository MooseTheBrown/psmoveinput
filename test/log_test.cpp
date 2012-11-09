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



#include "log.hpp"
#include "gtest/gtest.h"

namespace psmoveinput_test
{

namespace pi = psmoveinput;

class TestLogBackend : public pi::LogBackend
{
public:
    TestLogBackend() {}
    virtual void init(const pi::LogParams &params) { params_ = params; }
    virtual void write(const char *msg) { lastmsg_ = msg; }

    std::string getLastMessage() { return lastmsg_; }
    pi::LogParams &getParams() { return params_; }
protected:
    std::string lastmsg_;
    pi::LogParams params_;
};

TEST(Log, Params)
{
    TestLogBackend *backend = new TestLogBackend();
    pi::LogParams initParams{std::string("testfile"), pi::LogLevel::INFO};
    pi::Log log(initParams);
    log.addBackend(backend);

    // verify that the backend received the same parameters we passed
    // previously to log frontend
    pi::LogParams &testParams = backend->getParams();
    ASSERT_EQ(pi::LogLevel::INFO, testParams.loglevel);
    ASSERT_STREQ("testfile", testParams.logfile.c_str());
}

TEST(Log, WriteError)
{
    TestLogBackend *backend = new TestLogBackend();
    pi::LogParams initParams{std::string("testfile"), pi::LogLevel::ERROR};
    pi::Log log(initParams);
    log.addBackend(backend);

    // error message gets written
    log.write("error message", pi::LogLevel::ERROR);
    ASSERT_STREQ("error message", backend->getLastMessage().c_str());

    // fatal message gets written as well
    log.write("fatal error message", pi::LogLevel::FATAL);
    ASSERT_STREQ("fatal error message", backend->getLastMessage().c_str());

    // info message is not written
    log.write("informational message", pi::LogLevel::INFO);
    ASSERT_STREQ("fatal error message", backend->getLastMessage().c_str());
}

TEST(Log, WriteFatal)
{
    TestLogBackend *backend = new TestLogBackend();
    pi::LogParams initParams{std::string("testfile"), pi::LogLevel::FATAL};
    pi::Log log(initParams);
    log.addBackend(backend);

    // fatal message gets written
    log.write("fatal error message", pi::LogLevel::FATAL);
    ASSERT_STREQ("fatal error message", backend->getLastMessage().c_str());

    // error message is not written
    log.write("error message", pi::LogLevel::ERROR);
    ASSERT_STREQ("fatal error message", backend->getLastMessage().c_str());

    // info message is not written
    log.write("informational message", pi::LogLevel::INFO);
    ASSERT_STREQ("fatal error message", backend->getLastMessage().c_str());
}

TEST(Log, WriteInfo)
{
    TestLogBackend *backend = new TestLogBackend();
    pi::LogParams initParams{std::string("testfile"), pi::LogLevel::INFO};
    pi::Log log(initParams);
    log.addBackend(backend);

    // everything gets written
    log.write("error message", pi::LogLevel::ERROR);
    ASSERT_STREQ("error message", backend->getLastMessage().c_str());

    log.write("fatal error message", pi::LogLevel::FATAL);
    ASSERT_STREQ("fatal error message", backend->getLastMessage().c_str());

    log.write("informational message", pi::LogLevel::INFO);
    ASSERT_STREQ("informational message", backend->getLastMessage().c_str());
}

} // namespace psmoveinput_test
