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



#ifndef PSMOVEINPUT_CONF_KEYMAP_PARSER_HPP
#define PSMOVEINPUT_CONF_KEYMAP_PARSER_HPP

#include "common.hpp"
#include <string>
#include <map>

namespace psmoveinput
{

class KeyMapParser
{
public:
    KeyMapParser();
    virtual ~KeyMapParser();

    // given the option string and option value, produce key map entry
    KeyMapEntry createEntry(const std::string &optname, const std::string &optval);

protected:
    std::map<std::string, int> psmap_;
    std::map<std::string, int> linmap_;
};

} // namespace psmoveinput

#endif // PSMOVEINPUT_CONF_KEYMAP_PARSER_HPP
