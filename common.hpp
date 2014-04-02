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



#ifndef PSMOVEINPUT_COMMON_HPP
#define PSMOVEINPUT_COMMON_HPP

#include <vector>

// common definitions used by several psmoveinput components

namespace psmoveinput
{

// log levels
enum class LogLevel : unsigned char
{
    FATAL = 0,  // fatal error, further operation is not possible
    ERROR,      // general error
    INFO        // information message, psmoveinput operation is not affected
};

// mapping of psmove keys to Linux keys
struct KeyMapEntry
{
    int pscode;         // button code as reported by psmoveapi
    int lincode;        // key code understood by Linux input
};

typedef std::vector<KeyMapEntry> key_map;

// gyroscope values reported by psmoveapi are multiplied by
// these coefficients to produce move values reported to Linux input
struct MoveCoeffs
{
    double cx;
    double cy;
};

// psmoveinput operation mode
enum class OpMode : unsigned char
{
    STANDALONE = 0,
    CLIENT
};

enum class ControllerId : unsigned char
{
    FIRST,
    SECOND
};

#define MAX_CONTROLLERS 2

} // namespace psmoveinput

#endif // PSMOVEINPUT_COMMON_HPP
