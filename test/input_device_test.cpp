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



#include "input_device.hpp"
#include "test_config.h"
#include "gtest/gtest.h"
#include <boost/thread/thread.hpp>
#include <fcntl.h>
#include <iostream>
#include <cstdio>

namespace psmoveinput_test
{

typedef std::vector<input_event*> event_vector;

// DeviceListener opens input device with given name for reading and saves
// each event coming from it thus making it possible to check that the device reports correct events
class DeviceListener
{
public:
    DeviceListener(const char *device);
    virtual ~DeviceListener();

    const input_event* getLastEvent();
    event_vector* getAllEvents();
    void start() { thread_ = new boost::thread(boost::ref(*this)); }
    void stop();

    void operator() ();
protected:
    int fd_;
    boost::mutex mutex_;
    bool stop_;
    event_vector events_;
    boost::thread *thread_;
    std::string device_;
    std::string devfile_;

    bool find_device();
};

DeviceListener::DeviceListener(const char *device) :
    stop_(false),
    thread_(nullptr),
    fd_(-1),
    device_(device),
    devfile_("/dev/")
{
}

DeviceListener::~DeviceListener()
{
    if (thread_ != nullptr)
    {
        // wait until listener thread exits
        stop();
        thread_->join();
        delete thread_;
    }
    for (auto event : events_)
    {
        if (event != nullptr)
        {
            delete event;
        }
    }
}

const input_event *DeviceListener::getLastEvent()
{
    input_event *ret = nullptr;

    mutex_.lock();
    if (events_.empty() == false)
    {
        ret = events_.back();
    }
    mutex_.unlock();

    return ret;
}

event_vector *DeviceListener::getAllEvents()
{
    mutex_.lock();
    event_vector *ret_vector = new event_vector(events_);
    mutex_.unlock();

    return ret_vector;
}

void DeviceListener::stop()
{
    mutex_.lock();
    stop_ = true;
    mutex_.unlock();
    thread_->join();
}

// listener thread execution function
void DeviceListener::operator() ()
{
    input_event events[64];
    int count = 0;

    if (find_device() != true)
    {
        return;
    }

    fd_ = open(devfile_.c_str(), O_RDONLY | O_NONBLOCK);
    if (fd_ < 0)
    {
        return;
    }

    while (true)
    {
        // read events from the device until we're told to stop
        mutex_.lock();
        if (stop_ == true)
        {
            mutex_.unlock();
            break;
        }
        mutex_.unlock();

        count = read(fd_, events, sizeof (input_event) * 64);
        if (count >= static_cast<int>(sizeof (input_event)))
        {
            for (int i = 0; i < count / sizeof (input_event); i++)
            {
                // ignore EV_SYN
                if (events[i].type != EV_SYN)
                {
                    // add new event to the event vector
                    input_event *event = new input_event;
                    std::memcpy(event, &events[i], sizeof (input_event));
                    mutex_.lock();
                    events_.push_back(event);
                    mutex_.unlock();
                }
            }
        }
    }

    // set stop to false to make sure that the next time we launch
    // the same listener again it won't stop on first iteration
    mutex_.lock();
    stop_ = false;
    mutex_.unlock();
    close(fd_);
}

bool DeviceListener::find_device()
{
    bool ret = false;
    char cmd[1024];
    char buf[1024];

    // shell script searches for device with given name in /sys/class/input
    // and prints device event file to stdout
    std::snprintf(cmd, 1024, "%s %s 2>/dev/null", FIND_DEVICE_SCRIPT, device_.c_str());
    std::FILE *f = popen(cmd, "r");
    if (f != NULL)
    {
        std::fgets(buf, 1024, f);
        int status = pclose(f);
        if (WEXITSTATUS(status) == 0)
        {
            // remove trailing newline character
            char *s = std::strchr(buf, '\n');
            if (s != NULL)
            {
                *s = 0;
            }
            devfile_ += buf;
            ret = true;
        }
    }

    return ret;
}




// test fixture for Input Device tests
class InputDeviceTest : public testing::Test
{
public:
    virtual void SetUp()
    {
        psmoveinput::LogParams params("dummylog", psmoveinput::LogLevel::INFO);
        dummyLog_ = new psmoveinput::Log(params);
        dev_listener_ = new DeviceListener("testinput");
        psmoveinput::key_array keys{KEY_A, KEY_B, KEY_1};
        input_device_ = new psmoveinput::InputDevice("testinput", keys, *dummyLog_);

        dev_listener_->start();
        boost::this_thread::sleep(boost::posix_time::millisec(300));
    }

    virtual void TearDown()
    {
        dev_listener_->stop();
        delete dev_listener_;
        delete input_device_;
        delete dummyLog_;
        boost::this_thread::sleep(boost::posix_time::millisec(300));
    }

protected:
    DeviceListener *dev_listener_;
    psmoveinput::InputDevice *input_device_;
    psmoveinput::Log *dummyLog_;
};

TEST_F(InputDeviceTest, Devname)
{
    const char *devname = input_device_->getDeviceName();
    ASSERT_STREQ("testinput", devname);
}

TEST_F(InputDeviceTest, MouseMovement)
{
    const input_event *event;

    // inject mouse movement events and check what comes from the device file

    // dx = 5
    input_device_->reportMove(5, 0);
    boost::this_thread::sleep(boost::posix_time::millisec(500));
    event = dev_listener_->getLastEvent();
    ASSERT_EQ(EV_REL, event->type);
    ASSERT_EQ(REL_X, event->code);
    ASSERT_EQ(5, event->value);

    // dy = -10
    input_device_->reportMove(0, -10);
    boost::this_thread::sleep(boost::posix_time::millisec(500));
    event = dev_listener_->getLastEvent();
    ASSERT_EQ(EV_REL, event->type);
    ASSERT_EQ(REL_Y, event->code);
    ASSERT_EQ(-10, event->value);

    event_vector *events;

    // now inject movement on two axes simultaneously and verify that there are two events generated
    input_device_->reportMove(7, 5);
    boost::this_thread::sleep(boost::posix_time::millisec(500));
    events = dev_listener_->getAllEvents();
    ASSERT_EQ(4, events->size());

    delete events;
}

TEST_F(InputDeviceTest, Keys)
{
    const input_event *event;

    // inject key events one by one and check what comes from the device file

    // "A" press
    input_device_->reportKey(KEY_A, true);
    boost::this_thread::sleep(boost::posix_time::millisec(500));
    event = dev_listener_->getLastEvent();
    ASSERT_EQ(EV_KEY, event->type);
    ASSERT_EQ(KEY_A, event->code);
    ASSERT_EQ(1, event->value);

    // "A" release
    input_device_->reportKey(KEY_A, false);
    boost::this_thread::sleep(boost::posix_time::millisec(500));
    event = dev_listener_->getLastEvent();
    ASSERT_EQ(EV_KEY, event->type);
    ASSERT_EQ(KEY_A, event->code);
    ASSERT_EQ(0, event->value);

    // "B" press
    input_device_->reportKey(KEY_B, true);
    boost::this_thread::sleep(boost::posix_time::millisec(500));
    event = dev_listener_->getLastEvent();
    ASSERT_EQ(EV_KEY, event->type);
    ASSERT_EQ(KEY_B, event->code);
    ASSERT_EQ(1, event->value);

    // "B" release
    input_device_->reportKey(KEY_B, false);
    boost::this_thread::sleep(boost::posix_time::millisec(500));
    event = dev_listener_->getLastEvent();
    ASSERT_EQ(EV_KEY, event->type);
    ASSERT_EQ(KEY_B, event->code);
    ASSERT_EQ(0, event->value);

    // "1" press
    input_device_->reportKey(KEY_1, true);
    boost::this_thread::sleep(boost::posix_time::millisec(500));
    event = dev_listener_->getLastEvent();
    ASSERT_EQ(EV_KEY, event->type);
    ASSERT_EQ(KEY_1, event->code);
    ASSERT_EQ(1, event->value);

    // "1" release
    input_device_->reportKey(KEY_1, false);
    boost::this_thread::sleep(boost::posix_time::millisec(500));
    event = dev_listener_->getLastEvent();
    ASSERT_EQ(EV_KEY, event->type);
    ASSERT_EQ(KEY_1, event->code);
    ASSERT_EQ(0, event->value);
}

TEST_F(InputDeviceTest, MWheel)
{
    const input_event *event;

    // mouse wheel up
    input_device_->reportMWheel(1);
    boost::this_thread::sleep(boost::posix_time::millisec(100));
    event = dev_listener_->getLastEvent();
    ASSERT_EQ(EV_REL, event->type);
    ASSERT_EQ(REL_WHEEL, event->code);
    ASSERT_EQ(1, event->value);

    boost::this_thread::sleep(boost::posix_time::millisec(100));

    // mouse wheel down
    input_device_->reportMWheel(-1);
    boost::this_thread::sleep(boost::posix_time::millisec(100));
    event = dev_listener_->getLastEvent();
    ASSERT_EQ(EV_REL, event->type);
    ASSERT_EQ(REL_WHEEL, event->code);
    ASSERT_EQ(-1, event->value);
}

} // namespace psmoveinput_test
