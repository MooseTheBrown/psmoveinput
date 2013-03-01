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



#include "conf_keymap_parser.hpp"
#include "config_defs.hpp"
#include <psmoveapi/psmove.h>
#include <linux/input.h>

namespace psmoveinput
{

KeyMapParser::KeyMapParser() :
    psmap_ {
        std::make_pair(OPT_PSBTN_TRIANGLE, Btn_TRIANGLE),
        std::make_pair(OPT_PSBTN_CIRCLE, Btn_CIRCLE),
        std::make_pair(OPT_PSBTN_CROSS, Btn_CROSS),
        std::make_pair(OPT_PSBTN_SQUARE, Btn_SQUARE),
        std::make_pair(OPT_PSBTN_SELECT, Btn_SELECT),
        std::make_pair(OPT_PSBTN_START, Btn_START),
        std::make_pair(OPT_PSBTN_PS, Btn_PS),
        std::make_pair(OPT_PSBTN_MOVE, Btn_MOVE),
        std::make_pair(OPT_PSBTN_T, Btn_T),
        std::make_pair(OPT_PSBTN_1_TRIANGLE, Btn_TRIANGLE),
        std::make_pair(OPT_PSBTN_1_CIRCLE, Btn_CIRCLE),
        std::make_pair(OPT_PSBTN_1_CROSS, Btn_CROSS),
        std::make_pair(OPT_PSBTN_1_SQUARE, Btn_SQUARE),
        std::make_pair(OPT_PSBTN_1_SELECT, Btn_SELECT),
        std::make_pair(OPT_PSBTN_1_START, Btn_START),
        std::make_pair(OPT_PSBTN_1_PS, Btn_PS),
        std::make_pair(OPT_PSBTN_1_MOVE, Btn_MOVE),
        std::make_pair(OPT_PSBTN_1_T, Btn_T)
    },
    linmap_ {
        std::make_pair("KEY_ESC", KEY_ESC),
        std::make_pair("KEY_1", KEY_1),
        std::make_pair("KEY_2", KEY_2),
        std::make_pair("KEY_3", KEY_3),
        std::make_pair("KEY_4", KEY_4),
        std::make_pair("KEY_5", KEY_5),
        std::make_pair("KEY_6", KEY_6),
        std::make_pair("KEY_7", KEY_7),
        std::make_pair("KEY_8", KEY_8),
        std::make_pair("KEY_9", KEY_9),
        std::make_pair("KEY_0", KEY_0),
        std::make_pair("KEY_MINUS", KEY_MINUS),
        std::make_pair("KEY_EQUAL", KEY_EQUAL),
        std::make_pair("KEY_BACKSPACE", KEY_BACKSPACE),
        std::make_pair("KEY_TAB", KEY_TAB),
        std::make_pair("KEY_Q", KEY_Q),
        std::make_pair("KEY_W", KEY_W),
        std::make_pair("KEY_E", KEY_E),
        std::make_pair("KEY_R", KEY_R),
        std::make_pair("KEY_T", KEY_T),
        std::make_pair("KEY_Y", KEY_Y),
        std::make_pair("KEY_U", KEY_U),
        std::make_pair("KEY_I", KEY_I),
        std::make_pair("KEY_O", KEY_O),
        std::make_pair("KEY_P", KEY_P),
        std::make_pair("KEY_LEFTBRACE", KEY_LEFTBRACE),
        std::make_pair("KEY_RIGHTBRACE", KEY_RIGHTBRACE),
        std::make_pair("KEY_ENTER", KEY_ENTER),
        std::make_pair("KEY_LEFTCTRL", KEY_LEFTCTRL),
        std::make_pair("KEY_RIGHTCTRL", KEY_RIGHTCTRL),
        std::make_pair("KEY_A", KEY_A),
        std::make_pair("KEY_S", KEY_S),
        std::make_pair("KEY_D", KEY_D),
        std::make_pair("KEY_F", KEY_F),
        std::make_pair("KEY_G", KEY_G),
        std::make_pair("KEY_H", KEY_H),
        std::make_pair("KEY_J", KEY_J),
        std::make_pair("KEY_K", KEY_K),
        std::make_pair("KEY_L", KEY_L),
        std::make_pair("KEY_SEMICOLON", KEY_SEMICOLON),
        std::make_pair("KEY_APOSTROPHE", KEY_APOSTROPHE),
        std::make_pair("KEY_GRAVE", KEY_GRAVE),
        std::make_pair("KEY_LEFTSHIFT", KEY_LEFTSHIFT),
        std::make_pair("KEY_RIGHTSHIFT", KEY_RIGHTSHIFT),
        std::make_pair("KEY_BACKSLASH", KEY_BACKSLASH),
        std::make_pair("KEY_Z", KEY_Z),
        std::make_pair("KEY_X", KEY_X),
        std::make_pair("KEY_C", KEY_C),
        std::make_pair("KEY_V", KEY_V),
        std::make_pair("KEY_B", KEY_B),
        std::make_pair("KEY_N", KEY_N),
        std::make_pair("KEY_M", KEY_M),
        std::make_pair("KEY_COMMA", KEY_COMMA),
        std::make_pair("KEY_DOT", KEY_DOT),
        std::make_pair("KEY_SLASH", KEY_SLASH),
        std::make_pair("KEY_KPASTERISK", KEY_KPASTERISK),
        std::make_pair("KEY_LEFTALT", KEY_LEFTALT),
        std::make_pair("KEY_RIGHTALT", KEY_RIGHTALT),
        std::make_pair("KEY_SPACE", KEY_SPACE),
        std::make_pair("KEY_CAPSLOCK", KEY_CAPSLOCK),
        std::make_pair("KEY_F1", KEY_F1),
        std::make_pair("KEY_F2", KEY_F2),
        std::make_pair("KEY_F3", KEY_F3),
        std::make_pair("KEY_F4", KEY_F4),
        std::make_pair("KEY_F5", KEY_F5),
        std::make_pair("KEY_F6", KEY_F6),
        std::make_pair("KEY_F7", KEY_F7),
        std::make_pair("KEY_F8", KEY_F8),
        std::make_pair("KEY_F9", KEY_F9),
        std::make_pair("KEY_F10", KEY_F10),
        std::make_pair("KEY_NUMLOCK", KEY_NUMLOCK),
        std::make_pair("KEY_SCROLLLOCK", KEY_SCROLLLOCK),
        std::make_pair("KEY_F11", KEY_F11),
        std::make_pair("KEY_F12", KEY_F12),
        std::make_pair("KEY_SYSRQ", KEY_SYSRQ),
        std::make_pair("KEY_LEFTMETA", KEY_LEFTMETA),
        std::make_pair("KEY_RIGHTMETA", KEY_RIGHTMETA),
        std::make_pair("KEY_HOME", KEY_HOME),
        std::make_pair("KEY_UP", KEY_UP),
        std::make_pair("KEY_PAGEUP", KEY_PAGEUP),
        std::make_pair("KEY_LEFT", KEY_LEFT),
        std::make_pair("KEY_RIGHT", KEY_RIGHT),
        std::make_pair("KEY_END", KEY_END),
        std::make_pair("KEY_DOWN", KEY_DOWN),
        std::make_pair("KEY_PAGEDOWN", KEY_PAGEDOWN),
        std::make_pair("KEY_INSERT", KEY_INSERT),
        std::make_pair("KEY_DELETE", KEY_DELETE),
        std::make_pair("KEY_MUTE", KEY_MUTE),
        std::make_pair("KEY_VOLUMEDOWN", KEY_VOLUMEDOWN),
        std::make_pair("KEY_VOLUMEUP", KEY_VOLUMEUP),
        std::make_pair("KEY_PAUSE", KEY_PAUSE),
        std::make_pair("KEY_STOP", KEY_STOP),
        std::make_pair("KEY_BACK", KEY_BACK),
        std::make_pair("KEY_FORWARD", KEY_FORWARD),
        std::make_pair("KEY_PLAYPAUSE", KEY_PLAYPAUSE),
        std::make_pair("BTN_MOUSE", BTN_MOUSE),
        std::make_pair("BTN_LEFT", BTN_LEFT),
        std::make_pair("BTN_RIGHT", BTN_RIGHT),
        std::make_pair("BTN_MIDDLE", BTN_MIDDLE),
        std::make_pair("disconnect", KEY_PSMOVE_DISCONNECT),
        std::make_pair("move_trigger", KEY_PSMOVE_MOVE_TRIGGER)
    }
{
}

KeyMapParser::~KeyMapParser()
{
}

KeyMapEntry KeyMapParser::createEntry(const std::string &optname, const std::string &optval)
{
    KeyMapEntry newEntry = {0, KEY_RESERVED};

    auto psit = psmap_.find(optname);
    if (psit != psmap_.end())
    {
        newEntry.pscode = psit->second;
        auto linit = linmap_.find(optval.c_str());
        if (linit != linmap_.end())
        {
            newEntry.lincode = linit->second;
        }
    }

    return newEntry;
}

} // namespace psmoveinput
