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



#ifndef PSMOVEINPUT_EXCEPT_HPP
#define PSMOVEINPUT_EXCEPT_HPP

namespace psmoveinput
{

enum class ex_type
{
    INVALID_CONFIG,
    HELP_RQ,
    VERSION_RQ,
    ALREADY_RUNNING,
    FORK_FAILED,
    PARENT_QUIT,
    PIDFILE_CREAT_FAILED
};

class Exception
{
public:
    Exception(ex_type what) : what_(what) {}
    ex_type what() { return what_; }

protected:
    ex_type what_;
};

} // namespace psmoveinput

#endif // PSMOVEINPUT_EXCEPT_HPP
