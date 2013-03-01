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



#include "psmove_handler.hpp"
#include "gtest/gtest.h"
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>

namespace psmovehandler_test
{

class TestListener
{
public:
    TestListener() :
        dx_(0),
        dy_(0),
        code_(0),
        pressed_(false),
        disconnect_(false)
    {
    }
    virtual ~TestListener() {}
    void onMove(int dx, int dy) { dx_ = dx; dy_ = dy; }
    void onKey(int code, bool pressed) { code_ = code; pressed_ = pressed; }
    void onDisconnect(psmoveinput::ControllerId id) { disconnect_ = true; id_ = id; }

    int dx_;
    int dy_;
    int code_;
    bool pressed_;
    bool disconnect_;
    psmoveinput::ControllerId id_;
};

class PSMoveHandlerTest : public testing::Test
{
public:
    virtual void SetUp()
    {
        dummyLog_ = new psmoveinput::Log(psmoveinput::LogParams("dummylog",
                                                                psmoveinput::LogLevel::INFO));
        psmoveinput::key_map keymap1{{Btn_CROSS, KEY_X},
                                     {Btn_START, KEY_ENTER},
                                     {Btn_TRIANGLE, KEY_PSMOVE_DISCONNECT}};
        psmoveinput::key_map keymap2{{Btn_CROSS, KEY_SPACE}, {Btn_START, KEY_DOWN}};
        psmoveinput::MoveCoeffs coeffs{0.5, 2.0};
        handler_ = new psmoveinput::PSMoveHandler(keymap1, keymap2, coeffs, 100, *dummyLog_);
        handler_->getMoveSignal().connect(boost::bind(&TestListener::onMove,
                                                      &listener_,
                                                      _1, _2));
        handler_->getKeySignal().connect(boost::bind(&TestListener::onKey,
                                                     &listener_,
                                                     _1, _2));
        handler_->getDisconnectSignal().connect(boost::bind(&TestListener::onDisconnect,
                                                            &listener_, _1));
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

TEST_F(PSMoveHandlerTest, Gyroscope)
{
    handler_->onGyroscope(10, 30);
    // first gyroscope values reported to handler should never produce move event
    // they are only used to take the timestamp
    ASSERT_EQ(0, listener_.dx_);
    ASSERT_EQ(0, listener_.dy_);

    boost::this_thread::sleep(boost::posix_time::millisec(10));
    handler_->onGyroscope(-40, 20);
    // dx should be calculated in the following way:
    // dx = -40 (gyroscope value) * 10 (time delta) * 0.5 (x coeff)
    // same for dy
    // the measurements here are not precise since the time difference
    // calculated by handler might be slightly bigger than 10 ms
    ASSERT_TRUE(listener_.dx_ >= -280);
    ASSERT_TRUE(listener_.dx_ <= -200);
    ASSERT_TRUE(listener_.dy_ <= 440);
    ASSERT_TRUE(listener_.dy_ >= 400);

    boost::this_thread::sleep(boost::posix_time::millisec(10));
    // these gyroscope values should produce only x axis move report
    // since y axis should be filtered by handler's move threshold value
    handler_->onGyroscope(30, 1);
    ASSERT_TRUE(listener_.dx_ <= 210);
    ASSERT_TRUE(listener_.dx_ >= 150);
    ASSERT_EQ(0, listener_.dy_);
}

TEST_F(PSMoveHandlerTest, CorrectButtons)
{
    // report correct keys, which are present in the handler's keymaps

    // first controller
    handler_->onButtons(Btn_CROSS, psmoveinput::ControllerId::FIRST);
    ASSERT_EQ(KEY_X, listener_.code_);
    ASSERT_EQ(true, listener_.pressed_);

    handler_->onButtons(0, psmoveinput::ControllerId::FIRST);
    ASSERT_EQ(KEY_X, listener_.code_);
    ASSERT_EQ(false, listener_.pressed_);

    handler_->onButtons(Btn_START, psmoveinput::ControllerId::FIRST);
    ASSERT_EQ(KEY_ENTER, listener_.code_);
    ASSERT_EQ(true, listener_.pressed_);

    handler_->onButtons(0, psmoveinput::ControllerId::FIRST);
    ASSERT_EQ(KEY_ENTER, listener_.code_);
    ASSERT_EQ(false, listener_.pressed_);

    // second controller
    handler_->onButtons(Btn_CROSS, psmoveinput::ControllerId::SECOND);
    ASSERT_EQ(KEY_SPACE, listener_.code_);
    ASSERT_EQ(true, listener_.pressed_);

    handler_->onButtons(0, psmoveinput::ControllerId::SECOND);
    ASSERT_EQ(KEY_SPACE, listener_.code_);
    ASSERT_EQ(false, listener_.pressed_);

    handler_->onButtons(Btn_START, psmoveinput::ControllerId::SECOND);
    ASSERT_EQ(KEY_DOWN, listener_.code_);
    ASSERT_EQ(true, listener_.pressed_);

    handler_->onButtons(0, psmoveinput::ControllerId::SECOND);
    ASSERT_EQ(KEY_DOWN, listener_.code_);
    ASSERT_EQ(false, listener_.pressed_);

    // disconnect button
    handler_->onButtons(Btn_TRIANGLE, psmoveinput::ControllerId::FIRST);
    ASSERT_EQ(true, listener_.disconnect_);
    ASSERT_EQ(psmoveinput::ControllerId::FIRST, listener_.id_);
}

TEST_F(PSMoveHandlerTest, IncorrectButtons)
{
    // report keys, which are not present in the handler's key map
    handler_->onButtons(Btn_SELECT, psmoveinput::ControllerId::FIRST);
    ASSERT_EQ(0, listener_.code_);
    ASSERT_EQ(false, listener_.pressed_);
}

class PSMoveHandlerTriggerTest : public PSMoveHandlerTest
{
public:
    virtual void SetUp()
    {
        dummyLog_ = new psmoveinput::Log(psmoveinput::LogParams("dummylog",
                                                                psmoveinput::LogLevel::INFO));
        psmoveinput::key_map keymap1{{Btn_MOVE, KEY_PSMOVE_MOVE_TRIGGER}};
        psmoveinput::key_map keymap2{{Btn_CROSS, KEY_SPACE}, {Btn_START, KEY_DOWN}};
        psmoveinput::MoveCoeffs coeffs{1.0, 1.0};
        handler_ = new psmoveinput::PSMoveHandler(keymap1, keymap2, coeffs, 0, *dummyLog_);
        handler_->getMoveSignal().connect(boost::bind(&TestListener::onMove,
                                                      &listener_,
                                                      _1, _2));
        handler_->getKeySignal().connect(boost::bind(&TestListener::onKey,
                                                     &listener_,
                                                     _1, _2));
    }
};

TEST_F(PSMoveHandlerTriggerTest, MoveTrigger)
{
    handler_->onGyroscope(10, 10);
    // first gyroscope values reported to handler should never produce move event
    // they are only used to take the timestamp
    ASSERT_EQ(0, listener_.dx_);
    ASSERT_EQ(0, listener_.dy_);

    boost::this_thread::sleep(boost::posix_time::millisec(10));
    // without move trigger button pressed gyroscope data should not
    // be handled
    handler_->onGyroscope(-40, 20);
    ASSERT_EQ(0, listener_.dx_);
    ASSERT_EQ(0, listener_.dy_);

    // press trigger button
    handler_->onButtons(Btn_MOVE, psmoveinput::ControllerId::FIRST);
    boost::this_thread::sleep(boost::posix_time::millisec(10));
    handler_->onGyroscope(20, 20);
    ASSERT_NE(0, listener_.dx_);
    ASSERT_NE(0, listener_.dy_);
    
    listener_.dx_ = 0;
    listener_.dy_ = 0;

    // release trigger button
    boost::this_thread::sleep(boost::posix_time::millisec(5));
    handler_->onButtons(0, psmoveinput::ControllerId::FIRST);
    handler_->onGyroscope(20, 20);
    ASSERT_EQ(0, listener_.dx_);
    ASSERT_EQ(0, listener_.dy_);
}

} // namespace psmovehandler_test
