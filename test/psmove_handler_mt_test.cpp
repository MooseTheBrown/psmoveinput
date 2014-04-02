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

#include "psmove_handler.hpp"
#include "gtest/gtest.h"
#include <boost/thread/thread.hpp>
#include <vector>

namespace psmovehandler_mt_test
{

struct KeyEvt
{
    int key;
    bool pressed;

    KeyEvt(int akey, bool apressed) : key(akey), pressed(apressed) {}
};

class TestListener
{
public:
    TestListener() {}
    virtual ~TestListener() {}

    void onKey(int key, bool pressed) { keyEvents_.push_back(KeyEvt(key, pressed)); }

    std::vector<KeyEvt> keyEvents_;
};

#define NUM_KEY_EVTS    40000

class TestThread
{
public:
    TestThread(psmoveinput::PSMoveHandler *handler,
               psmoveinput::ControllerId controllerId) :
        handler_(handler),
        controllerId_(controllerId)
    {
    }

    void operator ()()
    {
        // send several key events to the handler
        for (int i = 0; i < NUM_KEY_EVTS; i++)
        {
            int buttons = 0;
            if ((i % 2) == 0)
            {
                if (controllerId_ == psmoveinput::ControllerId::FIRST)
                {
                    buttons = Btn_T;
                }
                else
                {
                    buttons = Btn_CROSS;
                }
            }

            handler_->onButtons(buttons, controllerId_);
        }
    }

protected:
    psmoveinput::PSMoveHandler* handler_;
    psmoveinput::ControllerId controllerId_;
};

class PSMoveHandlerMtTest : public testing::Test
{
public:
    virtual void SetUp()
    {
        dummyLog_ = new psmoveinput::Log(psmoveinput::LogParams("dummylog",
                                                                psmoveinput::LogLevel::INFO));
        psmoveinput::key_map keymap1{{Btn_T, KEY_1}};
        psmoveinput::key_map keymap2{{Btn_CROSS, KEY_2}};
        psmoveinput::MoveCoeffs coeffs{1.0, 1.0};
        handler_ = new psmoveinput::PSMoveHandler(keymap1, keymap2, coeffs, 0, 0, *dummyLog_);
        handler_->getKeySignal().connect(boost::bind(&TestListener::onKey,
                                                     &listener_,
                                                     _1, _2));
    }

    virtual void TearDown()
    {
        delete handler_;
        delete dummyLog_;
    }

protected:
    psmoveinput::PSMoveHandler *handler_;
    TestListener listener_;
    psmoveinput::Log *dummyLog_;
};

#define NUM_THREADS 2

TEST_F(PSMoveHandlerMtTest, MultipleThreads)
{
    // launch several threads and verify that PSMoveHandler reports all events
    boost::thread *threads[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++)
    {
        psmoveinput::ControllerId id = ((i % 2) == 0) ? psmoveinput::ControllerId::FIRST : psmoveinput::ControllerId::SECOND;
        threads[i] = new boost::thread(TestThread(handler_, id));
    }

    // wait for all threads to finish
    for (int i = 0; i < NUM_THREADS; i++)
    {
        threads[i]->join();
        delete threads[i];
    }

    ASSERT_EQ(NUM_KEY_EVTS * NUM_THREADS, listener_.keyEvents_.size());
}

} // namespace psmovehandler_mt_test
