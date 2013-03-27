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
        disconnect_(false),
        mwheel_value_(0)
    {
    }
    virtual ~TestListener() {}
    void onMove(int dx, int dy) { dx_ = dx; dy_ = dy; }
    void onKey(int code, bool pressed) { keys_.push_back(std::make_pair(code, pressed)); }
    void onDisconnect(psmoveinput::ControllerId id) { disconnect_ = true; id_ = id; }
    void onMWheel(int value) { mwheel_value_ = value; }

    int dx_;
    int dy_;
    std::vector<std::pair<int, bool>> keys_;
    bool disconnect_;
    psmoveinput::ControllerId id_;
    int mwheel_value_;
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
                                     {Btn_TRIANGLE, KEY_PSMOVE_DISCONNECT},
                                     {Btn_SQUARE, KEY_PSMOVE_MWHEEL_UP},
                                     {Btn_CIRCLE, KEY_PSMOVE_MWHEEL_DOWN}};
        psmoveinput::key_map keymap2{{Btn_CROSS, KEY_SPACE},
                                     {Btn_START, KEY_DOWN},
                                     {BTN_GESTURE_RIGHT, KEY_R},
                                     {BTN_GESTURE_DOWN, KEY_D}};
        psmoveinput::MoveCoeffs coeffs{0.5, 2.0};
        handler_ = new psmoveinput::PSMoveHandler(keymap1, keymap2, coeffs, 100, 50, *dummyLog_);
        handler_->getMoveSignal().connect(boost::bind(&TestListener::onMove,
                                                      &listener_,
                                                      _1, _2));
        handler_->getKeySignal().connect(boost::bind(&TestListener::onKey,
                                                     &listener_,
                                                     _1, _2));
        handler_->getDisconnectSignal().connect(boost::bind(&TestListener::onDisconnect,
                                                            &listener_, _1));
        handler_->getMWheelSignal().connect(boost::bind(&TestListener::onMWheel,
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

TEST_F(PSMoveHandlerTest, Gestures)
{
    handler_->onGesture(1, 1);
    // as with cursor movements, the first reading should not produce any event,
    // it is just for the timestamp taking
    ASSERT_EQ(0, listener_.keys_.size());

    boost::this_thread::sleep(boost::posix_time::millisec(10));
    handler_->onGesture(20, 1);
    // handler should calculate approximately 20 * 10 * 0.5 = 100 pixels on x axis,
    // which is above the gesture threshold of 50 pixels and report KEY_R
    ASSERT_EQ(KEY_R, listener_.keys_[0].first);
    ASSERT_EQ(true, listener_.keys_[0].second);
    // on y axis it should be 1 * 10 * 2 = 20, which is below 50 pixel threshold

    boost::this_thread::sleep(boost::posix_time::millisec(10));
    // emulate controller movement in the same direction
    handler_->onGesture(30, 0);
    // handler should not report anything, KEY_R should still be pressed
    ASSERT_EQ(KEY_R, listener_.keys_.back().first);
    ASSERT_EQ(true, listener_.keys_.back().second);

    boost::this_thread::sleep(boost::posix_time::millisec(10));
    handler_->onGesture(0, -50);
    // this time handler should get approximately 50 * 10 * 2 = 1000 pixels on y axis
    // and report KEY_D, while KEY_R should be released
    bool key_r_found = false;
    bool key_d_found = false;
    for (int i = 1; i < 3; i++)
    {
        
        std::pair<int, bool> key = listener_.keys_[i];
        if (key.first == KEY_R)
        {
            key_r_found = true;
            ASSERT_EQ(false, key.second);
        }
        else if (key.first == KEY_D)
        {
            key_d_found = true;
            ASSERT_EQ(true, key.second);
        }
    }
    ASSERT_TRUE(key_d_found);
    ASSERT_TRUE(key_r_found);

    handler_->onGesture(0, 0);
    boost::this_thread::sleep(boost::posix_time::millisec(10));

    // now emulate two gestures: the second short time after the first,
    // while the first is still active
    listener_.keys_.clear();
    handler_->onGesture(20, 0);
    ASSERT_EQ(KEY_R, listener_.keys_.back().first);
    ASSERT_EQ(true, listener_.keys_.back().second);
    boost::this_thread::sleep(boost::posix_time::millisec(10));
    handler_->onGesture(20, -50);
    ASSERT_EQ(2, listener_.keys_.size());
    ASSERT_EQ(KEY_D, listener_.keys_.back().first);
    ASSERT_EQ(true, listener_.keys_.back().second);
    boost::this_thread::sleep(boost::posix_time::millisec(10));
    handler_->onGesture(-20, -50);
    // gesture "up" is no longer active, so KEY_R should be released,
    // but KEY_D should remain pressed
    ASSERT_EQ(3, listener_.keys_.size());
    ASSERT_EQ(KEY_R, listener_.keys_.back().first);
    ASSERT_EQ(false, listener_.keys_.back().second);
}

TEST_F(PSMoveHandlerTest, CorrectButtons)
{
    // report correct keys, which are present in the handler's keymaps

    // first controller
    handler_->onButtons(Btn_CROSS, psmoveinput::ControllerId::FIRST);
    ASSERT_EQ(KEY_X, listener_.keys_.back().first);
    ASSERT_EQ(true, listener_.keys_.back().second);

    handler_->onButtons(0, psmoveinput::ControllerId::FIRST);
    ASSERT_EQ(KEY_X, listener_.keys_.back().first);
    ASSERT_EQ(false, listener_.keys_.back().second);

    handler_->onButtons(Btn_START, psmoveinput::ControllerId::FIRST);
    ASSERT_EQ(KEY_ENTER, listener_.keys_.back().first);
    ASSERT_EQ(true, listener_.keys_.back().second);

    handler_->onButtons(0, psmoveinput::ControllerId::FIRST);
    ASSERT_EQ(KEY_ENTER, listener_.keys_.back().first);
    ASSERT_EQ(false, listener_.keys_.back().second);

    // second controller
    handler_->onButtons(Btn_CROSS, psmoveinput::ControllerId::SECOND);
    ASSERT_EQ(KEY_SPACE, listener_.keys_.back().first);
    ASSERT_EQ(true, listener_.keys_.back().second);

    handler_->onButtons(0, psmoveinput::ControllerId::SECOND);
    ASSERT_EQ(KEY_SPACE, listener_.keys_.back().first);
    ASSERT_EQ(false, listener_.keys_.back().second);

    handler_->onButtons(Btn_START, psmoveinput::ControllerId::SECOND);
    ASSERT_EQ(KEY_DOWN, listener_.keys_.back().first);
    ASSERT_EQ(true, listener_.keys_.back().second);

    handler_->onButtons(0, psmoveinput::ControllerId::SECOND);
    ASSERT_EQ(KEY_DOWN, listener_.keys_.back().first);
    ASSERT_EQ(false, listener_.keys_.back().second);

    // disconnect button
    handler_->onButtons(Btn_TRIANGLE, psmoveinput::ControllerId::FIRST);
    ASSERT_EQ(true, listener_.disconnect_);
    ASSERT_EQ(psmoveinput::ControllerId::FIRST, listener_.id_);
}

TEST_F(PSMoveHandlerTest, IncorrectButtons)
{
    // report keys, which are not present in the handler's key map
    handler_->onButtons(Btn_SELECT, psmoveinput::ControllerId::FIRST);
    ASSERT_EQ(0, listener_.keys_.size());
}

TEST_F(PSMoveHandlerTest, HandlerReset)
{
    // report single key press
    handler_->onButtons(Btn_CROSS, psmoveinput::ControllerId::FIRST);
    ASSERT_EQ(KEY_X, listener_.keys_.back().first);
    ASSERT_EQ(true, listener_.keys_.back().second);

    // now reset handler's internal state
    handler_->reset();

    // report the same key again and verify that the handler in turn reports the same key
    handler_->onButtons(Btn_CROSS, psmoveinput::ControllerId::FIRST);
    ASSERT_EQ(KEY_X, listener_.keys_.back().first);
    ASSERT_EQ(true, listener_.keys_.back().second);

    // reset again
    handler_->reset();

    // report key release, which should not be reported by the handler
    listener_.keys_.clear();
    handler_->onButtons(0, psmoveinput::ControllerId::FIRST);
    ASSERT_EQ(0, listener_.keys_.size());

    // same procedure for the second controller
    handler_->onButtons(Btn_CROSS, psmoveinput::ControllerId::SECOND);
    ASSERT_EQ(KEY_SPACE, listener_.keys_.back().first);
    ASSERT_EQ(true, listener_.keys_.back().second);

    handler_->reset();

    handler_->onButtons(Btn_CROSS, psmoveinput::ControllerId::SECOND);
    ASSERT_EQ(KEY_SPACE, listener_.keys_.back().first);
    ASSERT_EQ(true, listener_.keys_.back().second);

    // reset again
    handler_->reset();

    // report key release, which should not be reported by the handler
    listener_.keys_.clear();
    handler_->onButtons(0, psmoveinput::ControllerId::SECOND);
    ASSERT_EQ(0, listener_.keys_.size());
}

TEST_F(PSMoveHandlerTest, MWheel)
{
    handler_->onButtons(Btn_SQUARE, psmoveinput::ControllerId::FIRST);
    ASSERT_EQ(1, listener_.mwheel_value_);
    boost::this_thread::sleep(boost::posix_time::millisec(10));
    handler_->onButtons(0, psmoveinput::ControllerId::FIRST);
    handler_->onButtons(Btn_CIRCLE, psmoveinput::ControllerId::FIRST);
    ASSERT_EQ(-1, listener_.mwheel_value_);
}

class PSMoveHandlerTriggerTest : public PSMoveHandlerTest
{
public:
    virtual void SetUp()
    {
        dummyLog_ = new psmoveinput::Log(psmoveinput::LogParams("dummylog",
                                                                psmoveinput::LogLevel::INFO));
        psmoveinput::key_map keymap1{{Btn_MOVE, KEY_PSMOVE_MOVE_TRIGGER}};
        psmoveinput::key_map keymap2{{Btn_CROSS, KEY_SPACE},
                                     {Btn_START, KEY_DOWN},
                                     {BTN_GESTURE_LEFT, KEY_L},
                                     {Btn_T, KEY_PSMOVE_GESTURE_TRIGGER}};
        psmoveinput::MoveCoeffs coeffs{1.0, 1.0};
        handler_ = new psmoveinput::PSMoveHandler(keymap1, keymap2, coeffs, 0, 40, *dummyLog_);
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

TEST_F(PSMoveHandlerTriggerTest, GestureTrigger)
{
    handler_->onGesture(1, 1);
    // no key reports, only timestamp taken by the handler

    boost::this_thread::sleep(boost::posix_time::millisec(10));
    handler_->onGesture(-10, 3);
    // although x axis movement (-10 * 10 * 1 = -100) is above the threshold value (40),
    // no key should be reported without gesture trigger pressed
    ASSERT_EQ(0, listener_.keys_.size());

    boost::this_thread::sleep(boost::posix_time::millisec(10));
    // press trigger button
    handler_->onButtons(Btn_T, psmoveinput::ControllerId::SECOND);
    handler_->onGesture(-7, 2);
    // this time x axis movement of -70 should produce KEY_L report
    ASSERT_EQ(KEY_L, listener_.keys_.back().first);
    ASSERT_EQ(true, listener_.keys_.back().second);

    boost::this_thread::sleep(boost::posix_time::millisec(10));

    // release the trigger; this should immediately release all pressed
    // gesture keys
    handler_->onButtons(0, psmoveinput::ControllerId::SECOND);
    ASSERT_EQ(KEY_L, listener_.keys_.back().first);
    ASSERT_EQ(false, listener_.keys_.back().second);

    listener_.keys_.clear();

    handler_->onGesture(-20, 0);
    ASSERT_EQ(0, listener_.keys_.size());
}

} // namespace psmovehandler_test
